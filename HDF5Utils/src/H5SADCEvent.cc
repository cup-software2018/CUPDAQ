#include "HDF5Utils/H5SADCEvent.hh"

ClassImp(H5SADCEvent)

H5SADCEvent::H5SADCEvent()
  : AbsH5Event()
{
}

H5SADCEvent::~H5SADCEvent() {}

void H5SADCEvent::Open()
{
  fEvtType = EventInfo_t::BuildType();
  fChType = AChannel_t::BuildType();

  // Initialize SubRun using the protected helper method from AbsH5Base
  InitSubRun();

  if (!fWriteTag) { return; }

  if (fFile < 0) {
    Error("Open", "invalid file id (fFile = %d). SetFileId must be called before Open().",
          static_cast<int>(fFile));
    return;
  }

  // create /events group
  {
    htri_t gexists = H5Lexists(fFile, "/events", H5P_DEFAULT);
    if (gexists < 0) {
      Error("Open", "H5Lexists(/events) failed");
      return;
    }
    if (gexists == 0) {
      hid_t grp_events = H5Gcreate2(fFile, "/events", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      if (grp_events < 0) {
        Error("Open", "Failed to create group /events");
        return;
      }
      H5Gclose(grp_events);
    }
  }

  // create extendable 1D datasets
  hsize_t dims[1] = {0};
  hsize_t maxdims[1] = {H5S_UNLIMITED};
  hid_t space1d = H5Screate_simple(1, dims, maxdims);

  hid_t dcpl1d = H5Pcreate(H5P_DATASET_CREATE);
  hsize_t chunk1d = (fBufCap > 0) ? static_cast<hsize_t>(fBufCap) : 1024;
  H5Pset_chunk(dcpl1d, 1, &chunk1d);
  H5Pset_deflate(dcpl1d, fCompressionLevel);

  fDsetInfo =
      H5Dcreate2(fFile, "/events/info", fEvtType, space1d, H5P_DEFAULT, dcpl1d, H5P_DEFAULT);
  fDsetIndex = H5Dcreate2(fFile, "/events/index", H5T_NATIVE_ULLONG, space1d, H5P_DEFAULT, dcpl1d,
                          H5P_DEFAULT);

  // SADC specific: use a potentially larger chunk for channels as they are smaller without
  // waveforms
  hsize_t chunkChs = 4096;
  hid_t dcplChs = H5Pcreate(H5P_DATASET_CREATE);
  H5Pset_chunk(dcplChs, 1, &chunkChs);
  H5Pset_deflate(dcplChs, fCompressionLevel);
  fDsetChs = H5Dcreate2(fFile, "/events/chs", fChType, space1d, H5P_DEFAULT, dcplChs, H5P_DEFAULT);

  H5Pclose(dcpl1d);
  H5Pclose(dcplChs);
  H5Sclose(space1d);

  fMemSize = 0;
  fTotalEvents = 0;
  fTotalChannels = 0;

  fEvtBuf.clear();
  fChBuf.clear();
  fBufCount = 0;
  fBufBytesUsed = 0;
}

herr_t H5SADCEvent::FlushBuffer()
{
  const std::size_t nEvtBuf = fEvtBuf.size();
  if (nEvtBuf == 0) {
    fBufCount = 0;
    fBufBytesUsed = 0;
    return 0;
  }

  const std::size_t nChBuf = fChBuf.size();
  herr_t status = 0;

  // Append events info
  {
    hsize_t old_dim[1] = {fTotalEvents};
    hsize_t new_dim[1] = {fTotalEvents + nEvtBuf};
    H5Dset_extent(fDsetInfo, new_dim);

    hid_t file_space = H5Dget_space(fDsetInfo);
    hsize_t offset[1] = {old_dim[0]};
    hsize_t count[1] = {nEvtBuf};

    H5Sselect_hyperslab(file_space, H5S_SELECT_SET, offset, nullptr, count, nullptr);
    hid_t mem_space = H5Screate_simple(1, count, nullptr);
    status = H5Dwrite(fDsetInfo, fEvtType, mem_space, file_space, H5P_DEFAULT, fEvtBuf.data());

    H5Sclose(mem_space);
    H5Sclose(file_space);
    if (status < 0) return status;
  }

  // Append event index
  {
    std::vector<std::uint64_t> indexBuf(nEvtBuf);
    std::uint64_t base = fTotalChannels;
    std::uint64_t running = 0;
    for (std::size_t i = 0; i < nEvtBuf; ++i) {
      indexBuf[i] = base + running;
      running += static_cast<std::uint64_t>(fEvtBuf[i].nhit);
    }

    hsize_t old_dim[1] = {fTotalEvents};
    hsize_t new_dim[1] = {fTotalEvents + nEvtBuf};
    H5Dset_extent(fDsetIndex, new_dim);

    hid_t file_space = H5Dget_space(fDsetIndex);
    hsize_t offset[1] = {old_dim[0]};
    hsize_t count[1] = {nEvtBuf};

    H5Sselect_hyperslab(file_space, H5S_SELECT_SET, offset, nullptr, count, nullptr);
    hid_t mem_space = H5Screate_simple(1, count, nullptr);
    status = H5Dwrite(fDsetIndex, H5T_NATIVE_ULLONG, mem_space, file_space, H5P_DEFAULT,
                      indexBuf.data());

    H5Sclose(mem_space);
    H5Sclose(file_space);
    if (status < 0) return status;
  }

  // Append SADC channels
  if (nChBuf > 0) {
    hsize_t old_dim[1] = {fTotalChannels};
    hsize_t new_dim[1] = {fTotalChannels + nChBuf};
    H5Dset_extent(fDsetChs, new_dim);

    hid_t file_space = H5Dget_space(fDsetChs);
    hsize_t offset[1] = {old_dim[0]};
    hsize_t count[1] = {nChBuf};

    H5Sselect_hyperslab(file_space, H5S_SELECT_SET, offset, nullptr, count, nullptr);
    hid_t mem_space = H5Screate_simple(1, count, nullptr);
    status = H5Dwrite(fDsetChs, fChType, mem_space, file_space, H5P_DEFAULT, fChBuf.data());

    H5Sclose(mem_space);
    H5Sclose(file_space);
    if (status < 0) return status;
  }

  fTotalEvents += static_cast<std::uint64_t>(nEvtBuf);
  fTotalChannels += static_cast<std::uint64_t>(nChBuf);

  fEvtBuf.clear();
  fChBuf.clear();
  fBufCount = 0;
  fBufBytesUsed = 0;

  return status;
}

void H5SADCEvent::Close()
{
  if (fWriteTag) { FlushBuffer(); }

  if (fFileSpaceInfo >= 0) {
    H5Sclose(fFileSpaceInfo);
    fFileSpaceInfo = H5I_INVALID_HID;
  }
  if (fFileSpaceIndex >= 0) {
    H5Sclose(fFileSpaceIndex);
    fFileSpaceIndex = H5I_INVALID_HID;
  }
  if (fFileSpaceChs >= 0) {
    H5Sclose(fFileSpaceChs);
    fFileSpaceChs = H5I_INVALID_HID;
  }

  if (fDsetInfo >= 0) {
    H5Dclose(fDsetInfo);
    fDsetInfo = H5I_INVALID_HID;
  }
  if (fDsetIndex >= 0) {
    H5Dclose(fDsetIndex);
    fDsetIndex = H5I_INVALID_HID;
  }
  if (fDsetChs >= 0) {
    H5Dclose(fDsetChs);
    fDsetChs = H5I_INVALID_HID;
  }

  fCurrentReadFid = H5I_INVALID_HID;

  fReadBufStart = -1;
  fReadBufSize = 0;
  fReadBufInfo.clear();
  fReadBufIndex.clear();
  fPrefetchChs.clear();

  if (fEvtType >= 0) {
    H5Tclose(fEvtType);
    fEvtType = H5I_INVALID_HID;
  }
  if (fChType >= 0) {
    H5Tclose(fChType);
    fChType = H5I_INVALID_HID;
  }

  // Use the protected helper method from AbsH5Base
  CloseSubRun();
}

herr_t H5SADCEvent::AppendEvent(const EventInfo_t & info, const std::vector<AChannel_t> & data)
{
  if (!fWriteTag) return -1;

  const std::uint16_t nhit = static_cast<std::uint16_t>(data.size());

  EventInfo_t info_local = info;
  info_local.nhit = nhit;
  fEvtBuf.push_back(info_local);

  std::size_t addBytes = sizeof(EventInfo_t) + static_cast<std::size_t>(nhit) * sizeof(AChannel_t);

  for (std::size_t i = 0; i < nhit; ++i) {
    fChBuf.push_back(data[i]);
  }

  // Update SubRun using the protected helper method from AbsH5Base
  UpdateSubRun(info.tnum);

  fMemSize += static_cast<hsize_t>(addBytes);
  fBufCount += 1;
  fBufBytesUsed += addBytes;

  if ((fBufCap > 0 && fBufCount >= fBufCap) ||
      (fBufMaxBytes > 0 && fBufBytesUsed >= fBufMaxBytes)) {
    return FlushBuffer();
  }

  return 0;
}

herr_t H5SADCEvent::ReadEvent(int n)
{
  int evtno = n;
  bool file_changed = false;
  hid_t fid = H5I_INVALID_HID;

  if (fChain && fChain->GetNFile() > 0) { fid = fChain->GetFileId(n, evtno, &file_changed); }
  else {
    fid = fFile;
    if (fCurrentReadFid != fid) { file_changed = true; }
  }

  if (fid < 0) return -1;

  if (file_changed) {
    if (fFileSpaceInfo >= 0) { H5Sclose(fFileSpaceInfo); }
    if (fFileSpaceIndex >= 0) { H5Sclose(fFileSpaceIndex); }
    if (fFileSpaceChs >= 0) { H5Sclose(fFileSpaceChs); }

    if (fDsetInfo >= 0) { H5Dclose(fDsetInfo); }
    if (fDsetIndex >= 0) { H5Dclose(fDsetIndex); }
    if (fDsetChs >= 0) { H5Dclose(fDsetChs); }

    hid_t dapl = H5Pcreate(H5P_DATASET_ACCESS);
    H5Pset_chunk_cache(dapl, 10007, 128 * 1024 * 1024, 1.0);

    fDsetInfo = H5Dopen2(fid, "/events/info", H5P_DEFAULT);
    fDsetIndex = H5Dopen2(fid, "/events/index", H5P_DEFAULT);
    fDsetChs = H5Dopen2(fid, "/events/chs", dapl);

    H5Pclose(dapl);

    if (fDsetInfo < 0 || fDsetIndex < 0 || fDsetChs < 0) return -1;

    fFileSpaceInfo = H5Dget_space(fDsetInfo);
    fFileSpaceIndex = H5Dget_space(fDsetIndex);
    fFileSpaceChs = H5Dget_space(fDsetChs);

    fCurrentReadFid = fid;
    fReadBufStart = -1;
    fReadBufSize = 0;
  }

  if (evtno < fReadBufStart || evtno >= fReadBufStart + fReadBufSize) {
    hsize_t dims[1];
    H5Sget_simple_extent_dims(fFileSpaceInfo, dims, nullptr);
    int total_events = static_cast<int>(dims[0]);

    std::size_t avg_channels = 10;
    std::size_t bytes_per_event =
        sizeof(EventInfo_t) + sizeof(std::uint64_t) + avg_channels * sizeof(AChannel_t);

    int max_safe_events = static_cast<int>(fBufMaxBytes / bytes_per_event);
    if (max_safe_events <= 0) max_safe_events = 10000;

    int fetch_size = std::min(max_safe_events, total_events - evtno);

    if (fetch_size > 0) {
      fReadBufInfo.resize(fetch_size);
      fReadBufIndex.resize(fetch_size);

      hsize_t offset_blk[1] = {static_cast<hsize_t>(evtno)};
      hsize_t count_blk[1] = {static_cast<hsize_t>(fetch_size)};
      hid_t mem_space_blk = H5Screate_simple(1, count_blk, nullptr);

      H5Sselect_hyperslab(fFileSpaceInfo, H5S_SELECT_SET, offset_blk, nullptr, count_blk, nullptr);
      H5Dread(fDsetInfo, fEvtType, mem_space_blk, fFileSpaceInfo, H5P_DEFAULT, fReadBufInfo.data());

      H5Sselect_hyperslab(fFileSpaceIndex, H5S_SELECT_SET, offset_blk, nullptr, count_blk, nullptr);
      H5Dread(fDsetIndex, H5T_NATIVE_ULLONG, mem_space_blk, fFileSpaceIndex, H5P_DEFAULT,
              fReadBufIndex.data());
      H5Sclose(mem_space_blk);

      std::uint64_t first_ch = fReadBufIndex[0];
      std::uint64_t total_chs =
          fReadBufIndex[fetch_size - 1] + fReadBufInfo[fetch_size - 1].nhit - first_ch;

      if (total_chs > 0) {
        fPrefetchChs.resize(total_chs);

        hsize_t offset_ch[1] = {first_ch};
        hsize_t count_ch[1] = {total_chs};
        H5Sselect_hyperslab(fFileSpaceChs, H5S_SELECT_SET, offset_ch, nullptr, count_ch, nullptr);
        hid_t mem_space_chs = H5Screate_simple(1, count_ch, nullptr);
        H5Dread(fDsetChs, fChType, mem_space_chs, fFileSpaceChs, H5P_DEFAULT, fPrefetchChs.data());
        H5Sclose(mem_space_chs);
      }

      fReadBufStart = evtno;
      fReadBufSize = fetch_size;
      fPrefetchChStart = first_ch;
    }
    else {
      return -1;
    }
  }

  int local_idx = evtno - fReadBufStart;
  fEvtInfo = fReadBufInfo[local_idx];
  std::uint64_t ch_offset = fReadBufIndex[local_idx];

  const std::uint16_t nhit = fEvtInfo.nhit;
  fDataBuf.resize(nhit);

  if (nhit > 0) {
    std::uint64_t local_ch_idx = ch_offset - fPrefetchChStart;
    std::copy(fPrefetchChs.begin() + local_ch_idx, fPrefetchChs.begin() + local_ch_idx + nhit,
              fDataBuf.begin());
  }

  return 0;
}