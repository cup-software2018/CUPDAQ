#include <algorithm>
#include <cstring>

#include "HDF5Utils/H5FADCHit.hh"

ClassImp(H5FADCHit)

H5FADCHit::H5FADCHit()
  : AbsH5Hit()
{
}

H5FADCHit::~H5FADCHit() {}

void H5FADCHit::Open()
{
  fChType = FChannelHeader_t::BuildType();

  // Initialize SubRun management in base class
  InitSubRun();

  if (!fWriteTag) { return; }

  if (fFile < 0) {
    Error("Open", "invalid file id (fFile = %d). SetFileId must be called before Open().",
          static_cast<int>(fFile));
    return;
  }

  if (fNDP <= 0 || fNDP > kH5FADCNDPMAX) {
    Error("Open", "Invalid NDP: %d (max %d)", fNDP, kH5FADCNDPMAX);
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

  // Create extendable dataset: /hits/chs (flattened hit headers)
  {
    hsize_t dims[1] = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hid_t space = H5Screate_simple(1, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk = 2048; // Chunk size for headers
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

  // Create extendable dataset: /hits/wave (hits x NDP samples)
  {
    hsize_t dims[2] = {0, static_cast<hsize_t>(fNDP)};
    hsize_t maxdims[2] = {H5S_UNLIMITED, static_cast<hsize_t>(fNDP)};
    hid_t space = H5Screate_simple(2, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk[2] = {1024, static_cast<hsize_t>(fNDP)}; // Chunk along hit dimension
    H5Pset_chunk(dcpl, 2, chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);

    fDsetWave =
        H5Dcreate2(fFile, "/hits/wave", H5T_NATIVE_USHORT, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

    H5Pclose(dcpl);
    H5Sclose(space);

    if (fDsetWave < 0) {
      Error("Open", "Failed to create dataset /hits/wave");
      return;
    }
  }

  fMemSize = 0;
  fTotalHits = 0; // Member of AbsH5Hit

  fChBuf.clear();
  fWaveBuf.clear();
  fBufCount = 0;
  fBufBytesUsed = 0;
}

int H5FADCHit::GetNDP()
{
  if (fWriteTag || fNDP > 0) { return fNDP; }

  // Read mode logic
  if (fFileSpaceWave >= 0) {
    hsize_t dims[2] = {0, 0};
    H5Sget_simple_extent_dims(fFileSpaceWave, dims, nullptr);
    fNDP = static_cast<int>(dims[1]);
    return fNDP;
  }

  hid_t fid = H5I_INVALID_HID;
  int dummy_evt = 0;
  if (fChain && fChain->GetNFile() > 0) { fid = fChain->GetFileId(0, dummy_evt); }
  else if (fFile >= 0) {
    fid = fFile;
  }

  if (fid < 0) return 0;

  hid_t dset = H5Dopen2(fid, "/hits/wave", H5P_DEFAULT);
  if (dset < 0) return 0;

  hid_t space = H5Dget_space(dset);
  hsize_t dims[2] = {0, 0};
  H5Sget_simple_extent_dims(space, dims, nullptr);
  H5Sclose(space);
  H5Dclose(dset);

  fNDP = (dims[1] > 0 && dims[1] <= static_cast<hsize_t>(kH5FADCNDPMAX)) ? static_cast<int>(dims[1])
                                                                         : 0;
  return fNDP;
}

herr_t H5FADCHit::FlushBuffer()
{
  const std::size_t nHitBuf = fChBuf.size();
  if (nHitBuf == 0) {
    fBufCount = 0;
    fBufBytesUsed = 0;
    return 0;
  }

  // Consistency check
  if (nHitBuf * static_cast<std::size_t>(fNDP) != fWaveBuf.size()) {
    Error("FlushBuffer", "Internal hit buffer size mismatch");
    return -1;
  }

  herr_t status = 0;

  // Append hit headers to /hits/chs
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

  // Append waveforms to /hits/wave
  {
    hsize_t old_dims[2] = {fTotalHits, static_cast<hsize_t>(fNDP)};
    hsize_t new_dims[2] = {fTotalHits + nHitBuf, static_cast<hsize_t>(fNDP)};
    H5Dset_extent(fDsetWave, new_dims);

    hid_t file_space = H5Dget_space(fDsetWave);
    hsize_t offset[2] = {old_dims[0], 0};
    hsize_t count[2] = {nHitBuf, static_cast<hsize_t>(fNDP)};
    H5Sselect_hyperslab(file_space, H5S_SELECT_SET, offset, nullptr, count, nullptr);

    hid_t mem_space = H5Screate_simple(2, count, nullptr);
    status =
        H5Dwrite(fDsetWave, H5T_NATIVE_USHORT, mem_space, file_space, H5P_DEFAULT, fWaveBuf.data());

    H5Sclose(mem_space);
    H5Sclose(file_space);
    if (status < 0) return status;
  }

  // Update base counter and clear buffers
  fTotalHits += static_cast<std::uint64_t>(nHitBuf);

  fChBuf.clear();
  fWaveBuf.clear();
  fBufCount = 0;
  fBufBytesUsed = 0;

  return status;
}

void H5FADCHit::Close()
{
  if (fWriteTag) { FlushBuffer(); }

  // Close cached DataSpaces
  if (fFileSpaceChs >= 0) {
    H5Sclose(fFileSpaceChs);
    fFileSpaceChs = H5I_INVALID_HID;
  }
  if (fFileSpaceWave >= 0) {
    H5Sclose(fFileSpaceWave);
    fFileSpaceWave = H5I_INVALID_HID;
  }

  // Close Datasets
  if (fDsetChs >= 0) {
    H5Dclose(fDsetChs);
    fDsetChs = H5I_INVALID_HID;
  }
  if (fDsetWave >= 0) {
    H5Dclose(fDsetWave);
    fDsetWave = H5I_INVALID_HID;
  }

  fCurrentReadFid = H5I_INVALID_HID;

  // Reset prefetch buffers
  fPrefetchChs.clear();
  fPrefetchWave.clear();

  // Close type
  if (fChType >= 0) {
    H5Tclose(fChType);
    fChType = H5I_INVALID_HID;
  }

  // Finalize SubRun management in base
  CloseSubRun();
}

herr_t H5FADCHit::AppendHit(const FChannel_t & hit)
{
  if (!fWriteTag) return -1;
  if (fNDP <= 0 || fNDP > kH5FADCNDPMAX) return -1;

  // 1. Map to header struct
  FChannelHeader_t header;
  header.id = hit.id;
  header.tbit = hit.tbit;
  header.ped = hit.ped;
  header.time = hit.time;

  // 2. Buffer data
  fChBuf.push_back(header);
  fWaveBuf.insert(fWaveBuf.end(), hit.waveform, hit.waveform + static_cast<std::size_t>(fNDP));

  // 3. Accounting
  std::size_t addBytes =
      sizeof(FChannelHeader_t) + static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t);
  fMemSize += static_cast<hsize_t>(addBytes);
  fBufCount += 1;
  fBufBytesUsed += addBytes;

  // 4. Update SubRun (Treating hit time as the sequential tracker)
  UpdateSubRun(header.time);

  // 5. Flush if needed
  if ((fBufCap > 0 && fBufCount >= fBufCap) ||
      (fBufMaxBytes > 0 && fBufBytesUsed >= fBufMaxBytes)) {
    return FlushBuffer();
  }

  return 0;
}

herr_t H5FADCHit::ReadHit(std::uint64_t n)
{
  std::uint64_t local_hit_no = n;
  hid_t fid = H5I_INVALID_HID;

  // Resolve File ID:
  // H5DataReader (Read mode) uses fChain.
  // H5DataWriter (Write mode) uses fFile.
  if (fChain && fChain->GetNFile() > 0) {
    int dummy = 0;
    fid = fChain->GetFileId(0, dummy); // Simplified for single file reading
  }
  else {
    fid = fFile;
  }

  if (fid < 0) return -1;

  // ---------------------------------------------------------------------
  // 1. File/Dataset Initialization (Lazy loading)
  // ---------------------------------------------------------------------
  if (fCurrentReadFid != fid) {
    if (fFileSpaceChs >= 0) H5Sclose(fFileSpaceChs);
    if (fFileSpaceWave >= 0) H5Sclose(fFileSpaceWave);
    if (fDsetChs >= 0) H5Dclose(fDsetChs);
    if (fDsetWave >= 0) H5Dclose(fDsetWave);

    // Setup chunk cache for wave data
    hid_t dapl = H5Pcreate(H5P_DATASET_ACCESS);
    H5Pset_chunk_cache(dapl, 10007, 128 * 1024 * 1024, 1.0);

    fDsetChs = H5Dopen2(fid, "/hits/chs", H5P_DEFAULT);
    fDsetWave = H5Dopen2(fid, "/hits/wave", dapl);
    H5Pclose(dapl);

    if (fDsetChs < 0 || fDsetWave < 0) return -1;

    fFileSpaceChs = H5Dget_space(fDsetChs);
    fFileSpaceWave = H5Dget_space(fDsetWave);

    fCurrentReadFid = fid;
    fNDP = GetNDP();
    fReadBufStart = 0; // Reset prefetch window
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
    std::size_t bytes_per_hit =
        sizeof(FChannelHeader_t) + static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t);

    // Safety check
    if (bytes_per_hit == 0) return -1;

    std::uint64_t max_safe_hits = fBufMaxBytes / bytes_per_hit;
    if (max_safe_hits == 0) max_safe_hits = 1000; // Fallback

    std::uint64_t remaining = total_hits_in_file - local_hit_no;
    std::uint64_t fetch_size = std::min(max_safe_hits, remaining);

    if (fetch_size > 0) {
      fPrefetchChs.resize(fetch_size);
      fPrefetchWave.resize(fetch_size * fNDP);

      // Fetch Headers
      hsize_t offset_ch[1] = {local_hit_no};
      hsize_t count_ch[1] = {fetch_size};
      hid_t mem_space_chs = H5Screate_simple(1, count_ch, nullptr);
      H5Sselect_hyperslab(fFileSpaceChs, H5S_SELECT_SET, offset_ch, nullptr, count_ch, nullptr);
      H5Dread(fDsetChs, fChType, mem_space_chs, fFileSpaceChs, H5P_DEFAULT, fPrefetchChs.data());
      H5Sclose(mem_space_chs);

      // Fetch Waveforms (2D)
      hsize_t offset_wave[2] = {local_hit_no, 0};
      hsize_t count_wave[2] = {fetch_size, static_cast<hsize_t>(fNDP)};
      hid_t mem_space_wave = H5Screate_simple(2, count_wave, nullptr);
      H5Sselect_hyperslab(fFileSpaceWave, H5S_SELECT_SET, offset_wave, nullptr, count_wave,
                          nullptr);
      H5Dread(fDsetWave, H5T_NATIVE_USHORT, mem_space_wave, fFileSpaceWave, H5P_DEFAULT,
              fPrefetchWave.data());
      H5Sclose(mem_space_wave);

      fReadBufStart = local_hit_no;
      fReadBufSize = fetch_size;
    }
  }

  // ---------------------------------------------------------------------
  // 3. Serve from RAM
  // ---------------------------------------------------------------------
  std::uint64_t local_idx = local_hit_no - fReadBufStart;

  // Copy header properties directly to fCurrentHit
  fCurrentHit.id = fPrefetchChs[local_idx].id;
  fCurrentHit.tbit = fPrefetchChs[local_idx].tbit;
  fCurrentHit.ped = fPrefetchChs[local_idx].ped;
  fCurrentHit.time = fPrefetchChs[local_idx].time;

  // Copy waveform into the struct's array
  std::memcpy(fCurrentHit.waveform, &fPrefetchWave[local_idx * fNDP],
              static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t));

  return 0;
}