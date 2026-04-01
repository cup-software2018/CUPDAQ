#include <algorithm>

#include "HDF5Utils/H5SADCHit.hh"

ClassImp(H5SADCHit)

H5SADCHit::H5SADCHit()
  : AbsH5Hit()
{
}

H5SADCHit::~H5SADCHit() {}

void H5SADCHit::Open()
{
  fChType = AChannel_t::BuildType();

  // Initialize SubRun management in base class
  InitSubRun();

  if (!fWriteTag) { return; }

  if (fFile < 0) {
    Error("Open", "invalid file id (fFile = %d). SetFileId must be called before Open().",
          static_cast<int>(fFile));
    return;
  }

  // Create /hits group for Self Trigger stream
  {
    htri_t gexists = H5Lexists(fFile, "/hits", H5P_DEFAULT);
    if (gexists < 0) {
      Error("Open", "H5Lexists(/hits) failed");
      return;
    }
    if (gexists == 0) {
      hid_t grp_hits = H5Gcreate2(fFile, "/hits", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      if (grp_hits < 0) {
        Error("Open", "Failed to create group /hits");
        return;
      }
      H5Gclose(grp_hits);
    }
  }

  // Create extendable dataset: /hits/chs (flattened SADC hits)
  {
    hsize_t dims[1] = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hid_t space = H5Screate_simple(1, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk = 4096; // Chunk size for SADC hits
    H5Pset_chunk(dcpl, 1, &chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);

    fDsetChs = H5Dcreate2(fFile, "/hits/chs", fChType, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

    H5Pclose(dcpl);
    H5Sclose(space);

    if (fDsetChs < 0) {
      Error("Open", "Failed to create dataset /hits/chs");
      return;
    }
  }

  fMemSize = 0;
  fTotalHits = 0; // Member of AbsH5Hit

  fChBuf.clear();
  fBufCount = 0;
  fBufBytesUsed = 0;
}

herr_t H5SADCHit::FlushBuffer()
{
  const std::size_t nHitBuf = fChBuf.size();
  if (nHitBuf == 0) {
    fBufCount = 0;
    fBufBytesUsed = 0;
    return 0;
  }

  herr_t status = 0;

  // Append SADC hits to /hits/chs
  {
    hsize_t old_dim[1] = {fTotalHits};
    hsize_t new_dim[1] = {fTotalHits + nHitBuf};
    H5Dset_extent(fDsetChs, new_dim);

    hid_t file_space = H5Dget_space(fDsetChs);
    hsize_t offset[1] = {old_dim[0]};
    hsize_t count[1] = {nHitBuf};
    H5Sselect_hyperslab(file_space, H5S_SELECT_SET, offset, nullptr, count, nullptr);

    hid_t mem_space = H5Screate_simple(1, count, nullptr);
    status = H5Dwrite(fDsetChs, fChType, mem_space, file_space, H5P_DEFAULT, fChBuf.data());

    H5Sclose(mem_space);
    H5Sclose(file_space);
    if (status < 0) return status;
  }

  // Update base counter and clear buffers
  fTotalHits += static_cast<std::uint64_t>(nHitBuf);

  fChBuf.clear();
  fBufCount = 0;
  fBufBytesUsed = 0;

  return status;
}

void H5SADCHit::Close()
{
  if (fWriteTag) { FlushBuffer(); }

  // Close cached DataSpaces
  if (fFileSpaceChs >= 0) {
    H5Sclose(fFileSpaceChs);
    fFileSpaceChs = H5I_INVALID_HID;
  }

  // Close Datasets
  if (fDsetChs >= 0) {
    H5Dclose(fDsetChs);
    fDsetChs = H5I_INVALID_HID;
  }

  fCurrentReadFid = H5I_INVALID_HID;

  // Reset prefetch buffers
  fPrefetchChs.clear();
  fReadBufStart = 0;
  fReadBufSize = 0;

  // Close type
  if (fChType >= 0) {
    H5Tclose(fChType);
    fChType = H5I_INVALID_HID;
  }

  // Finalize SubRun management in base
  CloseSubRun();
}

herr_t H5SADCHit::AppendHit(const AChannel_t & hit)
{
  if (!fWriteTag) return -1;

  // 1. Buffer data
  fChBuf.push_back(hit);

  // 2. Accounting
  std::size_t addBytes = sizeof(AChannel_t);
  fMemSize += static_cast<hsize_t>(addBytes);
  fBufCount += 1;
  fBufBytesUsed += addBytes;

  // 3. Update SubRun (Treating hit time as the sequential tracker)
  UpdateSubRun(hit.time);

  // 4. Flush if needed
  if ((fBufCap > 0 && fBufCount >= fBufCap) ||
      (fBufMaxBytes > 0 && fBufBytesUsed >= fBufMaxBytes)) {
    return FlushBuffer();
  }

  return 0;
}

herr_t H5SADCHit::ReadHit(std::uint64_t n)
{
  std::uint64_t local_hit_no = n;
  hid_t fid = H5I_INVALID_HID;

  // Resolve File ID:
  // H5DataReader (Read mode) uses fChain.
  // H5DataWriter (Write mode) uses fFile.
  if (fChain && fChain->GetNFile() > 0) {
    int dummy = 0;
    fid = fChain->GetFileId(0, dummy);
  }
  else {
    fid = fFile;
  }

  if (fid < 0) return -1;

  // ---------------------------------------------------------------------
  // 1. File/Dataset Initialization (Lazy loading)
  // ---------------------------------------------------------------------
  if (fCurrentReadFid != fid) {
    // Clean up previous cached dataspaces if any
    if (fFileSpaceChs >= 0) H5Sclose(fFileSpaceChs);
    if (fDsetChs >= 0) H5Dclose(fDsetChs);

    fDsetChs = H5Dopen2(fid, "/hits/chs", H5P_DEFAULT);
    if (fDsetChs < 0) return -1;

    fFileSpaceChs = H5Dget_space(fDsetChs);
    fCurrentReadFid = fid;

    fReadBufStart = 0;
    fReadBufSize = 0;
  }

  // ---------------------------------------------------------------------
  // 2. Linear Hit Prefetching (Sliding Window)
  // ---------------------------------------------------------------------
  if (local_hit_no < fReadBufStart || local_hit_no >= fReadBufStart + fReadBufSize) {
    hsize_t dims[1];
    H5Sget_simple_extent_dims(fFileSpaceChs, dims, nullptr);
    std::uint64_t total_hits_in_file = dims[0];

    if (local_hit_no >= total_hits_in_file) return -1; // Out of bounds

    // Calculate fetch size based on memory budget
    std::size_t bytes_per_hit = sizeof(AChannel_t);
    std::uint64_t max_safe_hits = fBufMaxBytes / bytes_per_hit;
    if (max_safe_hits == 0) max_safe_hits = 10000; // Fallback for safety

    std::uint64_t remaining = total_hits_in_file - local_hit_no;
    std::uint64_t fetch_size = std::min(max_safe_hits, remaining);

    if (fetch_size > 0) {
      fPrefetchChs.resize(fetch_size);

      // Fetch Hit Data (AChannel_t structures)
      hsize_t offset[1] = {local_hit_no};
      hsize_t count[1] = {fetch_size};
      hid_t mem_space = H5Screate_simple(1, count, nullptr);

      H5Sselect_hyperslab(fFileSpaceChs, H5S_SELECT_SET, offset, nullptr, count, nullptr);
      H5Dread(fDsetChs, fChType, mem_space, fFileSpaceChs, H5P_DEFAULT, fPrefetchChs.data());

      H5Sclose(mem_space);

      fReadBufStart = local_hit_no;
      fReadBufSize = fetch_size;
    }
  }

  // ---------------------------------------------------------------------
  // 3. Serve from RAM
  // ---------------------------------------------------------------------
  std::uint64_t local_idx = local_hit_no - fReadBufStart;

  // Copy data directly to fCurrentHit
  fCurrentHit = fPrefetchChs[local_idx];

  return 0;
}