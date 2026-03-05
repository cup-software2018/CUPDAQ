#include <cstring>

#include "HDF5Utils/H5FADCEvent.hh"

ClassImp(H5FADCEvent)

H5FADCEvent::H5FADCEvent()
  : AbsH5Event()
{
}

H5FADCEvent::~H5FADCEvent()
{
  // fDataBuf and fReadBufs are managed by std::vector, no manual deletion needed.
}

void H5FADCEvent::Open()
{
  // build HDF5 types for event header and channel header
  fEvtType = EventInfo_t::BuildType();
  fChType = FChannelHeader_t::BuildType();

  // read mode: only build types, no dataset creation
  if (!fWriteTag) { return; }

  // safety: file id must be valid in write mode
  if (fFile < 0) {
    Error("Open", "invalid file id (fFile = %d). SetFileId must be called before Open().",
          static_cast<int>(fFile));
    return;
  }

  // NDP must be fixed and sane in write mode
  if (fNDP <= 0 || fNDP > kH5FADCNDPMAX) {
    Error("Open", "Invalid NDP: %d (max %d)", fNDP, kH5FADCNDPMAX);
    return;
  }

  // create /events group only if it does not exist
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

  // create extendable dataset: /events/info (per-event metadata)
  {
    hsize_t dims[1] = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hid_t space = H5Screate_simple(1, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk = (fBufEventCap > 0) ? static_cast<hsize_t>(fBufEventCap) : 1024;
    H5Pset_chunk(dcpl, 1, &chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);

    fDsetInfo = H5Dcreate2(fFile, "/events/info", fEvtType, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

    H5Pclose(dcpl);
    H5Sclose(space);

    if (fDsetInfo < 0) {
      Error("Open", "Failed to create dataset /events/info");
      return;
    }
  }

  // create extendable dataset: /events/index (event -> first channel index)
  {
    hsize_t dims[1] = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hid_t space = H5Screate_simple(1, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk = (fBufEventCap > 0) ? static_cast<hsize_t>(fBufEventCap) : 1024;
    H5Pset_chunk(dcpl, 1, &chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);

    fDsetIndex = H5Dcreate2(fFile, "/events/index", H5T_NATIVE_ULLONG, space, H5P_DEFAULT, dcpl,
                            H5P_DEFAULT);

    H5Pclose(dcpl);
    H5Sclose(space);

    if (fDsetIndex < 0) {
      Error("Open", "Failed to create dataset /events/index");
      return;
    }
  }

  // create extendable dataset: /events/chs (flattened channel headers)
  {
    hsize_t dims[1] = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hid_t space = H5Screate_simple(1, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk = 1024;
    H5Pset_chunk(dcpl, 1, &chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);

    fDsetChs = H5Dcreate2(fFile, "/events/chs", fChType, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

    H5Pclose(dcpl);
    H5Sclose(space);

    if (fDsetChs < 0) {
      Error("Open", "Failed to create dataset /events/chs");
      return;
    }
  }

  // create extendable dataset: /events/wave (channels x NDP samples)
  {
    hsize_t dims[2] = {0, static_cast<hsize_t>(fNDP)};
    hsize_t maxdims[2] = {H5S_UNLIMITED, static_cast<hsize_t>(fNDP)};
    hid_t space = H5Screate_simple(2, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk[2] = {1024, static_cast<hsize_t>(fNDP)};
    H5Pset_chunk(dcpl, 2, chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);

    fDsetWave =
        H5Dcreate2(fFile, "/events/wave", H5T_NATIVE_USHORT, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

    H5Pclose(dcpl);
    H5Sclose(space);

    if (fDsetWave < 0) {
      Error("Open", "Failed to create dataset /events/wave");
      return;
    }
  }

  // initialize subrun and internal counters
  fSubRun.nevent = 0;
  fSubRun.first = 0;
  fSubRun.last = 0;

  fMemSize = 0;
  fTotalEvents = 0;
  fTotalChannels = 0;

  fEvtBuf.clear();
  fChBuf.clear();
  fWaveBuf.clear();
  fBufEventCount = 0;
  fBufBytesUsed = 0;
}

int H5FADCEvent::GetNDP()
{
  if (fWriteTag) { return fNDP; }
  if (fNDP > 0) { return fNDP; }

  // 💡 FIX: If the dataspace is already open (from ReadEvent), use it instantly!
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

  if (fid < 0) { return 0; }

  hid_t dset = H5Dopen2(fid, "/events/wave", H5P_DEFAULT);
  if (dset < 0) { return 0; }

  hid_t space = H5Dget_space(dset);
  hsize_t dims[2] = {0, 0};
  H5Sget_simple_extent_dims(space, dims, nullptr);
  H5Sclose(space);
  H5Dclose(dset);

  if (dims[1] > 0 && dims[1] <= static_cast<hsize_t>(kH5FADCNDPMAX)) {
    fNDP = static_cast<int>(dims[1]);
  }
  else {
    fNDP = 0;
  }

  return fNDP;
}

herr_t H5FADCEvent::FlushBuffer()
{
  // nothing to flush
  const std::size_t nEvtBuf = fEvtBuf.size();
  if (nEvtBuf == 0) {
    fBufEventCount = 0;
    fBufBytesUsed = 0;
    return 0;
  }

  const std::size_t nChBuf = fChBuf.size();
  // internal consistency check: channels x NDP must match waveform buffer length
  if (nChBuf * static_cast<std::size_t>(fNDP) != fWaveBuf.size()) {
    Error("FlushBuffer", "Internal buffer size mismatch");
    return -1;
  }

  herr_t status = 0;

  // append event headers to /events/info
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

    if (status < 0) { return status; }
  }

  // build and append event -> first-channel index map
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

    if (status < 0) { return status; }
  }

  // append channel headers and waveforms
  if (nChBuf > 0) {
    // /events/chs: 1D list of channel headers
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

    if (status < 0) { return status; }

    // /events/wave: 2D (channel index, sample)
    hsize_t old_dims[2] = {fTotalChannels, static_cast<hsize_t>(fNDP)};
    hsize_t new_dims[2] = {fTotalChannels + nChBuf, static_cast<hsize_t>(fNDP)};
    H5Dset_extent(fDsetWave, new_dims);

    hid_t file_space_wave = H5Dget_space(fDsetWave);

    hsize_t offset_wave[2] = {old_dims[0], 0};
    hsize_t count_wave[2] = {nChBuf, static_cast<hsize_t>(fNDP)};

    H5Sselect_hyperslab(file_space_wave, H5S_SELECT_SET, offset_wave, nullptr, count_wave, nullptr);

    hid_t mem_space_wave = H5Screate_simple(2, count_wave, nullptr);

    status = H5Dwrite(fDsetWave, H5T_NATIVE_USHORT, mem_space_wave, file_space_wave, H5P_DEFAULT,
                      fWaveBuf.data());

    H5Sclose(mem_space_wave);
    H5Sclose(file_space_wave);

    if (status < 0) { return status; }
  }

  // update global counters and reset buffers
  fTotalEvents += static_cast<std::uint64_t>(nEvtBuf);
  fTotalChannels += static_cast<std::uint64_t>(nChBuf);

  fEvtBuf.clear();
  fChBuf.clear();
  fWaveBuf.clear();
  fBufEventCount = 0;
  fBufBytesUsed = 0;

  return status;
}

void H5FADCEvent::Close()
{
  if (fWriteTag) { FlushBuffer(); }

  // Close cached DataSpaces
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
  if (fFileSpaceWave >= 0) {
    H5Sclose(fFileSpaceWave);
    fFileSpaceWave = H5I_INVALID_HID;
  }

  // Close Datasets
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
  if (fDsetWave >= 0) {
    H5Dclose(fDsetWave);
    fDsetWave = H5I_INVALID_HID;
  }

  fCurrentReadFid = H5I_INVALID_HID;

  // Reset bulk load buffers
  fReadBufInfo.clear();
  fReadBufIndex.clear();

  // close committed types
  if (fEvtType >= 0) {
    H5Tclose(fEvtType);
    fEvtType = H5I_INVALID_HID;
  }
  if (fChType >= 0) {
    H5Tclose(fChType);
    fChType = H5I_INVALID_HID;
  }
}

herr_t H5FADCEvent::AppendEvent(const EventInfo_t & info, const std::vector<FChannel_t> & data)
{
  if (!fWriteTag) { return -1; }
  if (fNDP <= 0 || fNDP > kH5FADCNDPMAX) {
    Error("AppendEvent", "Invalid NDP: %d (max %d)", fNDP, kH5FADCNDPMAX);
    return -1;
  }

  // number of hit channels in this event
  const std::uint16_t nhit = static_cast<std::uint16_t>(data.size());

  // store event header in buffer (with nhit filled)
  EventInfo_t info_local = info;
  info_local.nhit = nhit;
  fEvtBuf.push_back(info_local);

  // approximate memory usage for this event (metadata + all channels)
  std::size_t addBytes = sizeof(EventInfo_t);
  addBytes += static_cast<std::size_t>(nhit) *
              (sizeof(FChannelHeader_t) + static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t));

  // copy channel headers and waveforms into flat buffers
  for (std::size_t i = 0; i < nhit; ++i) {
    FChannelHeader_t h{};
    h.id = data[i].id;
    h.tbit = data[i].tbit;
    h.ped = data[i].ped;
    fChBuf.push_back(h);

    const std::uint16_t * src = data[i].waveform;
    fWaveBuf.insert(fWaveBuf.end(), src, src + static_cast<std::size_t>(fNDP));
  }

  // update subrun information
  if (fSubRun.nevent == 0) { fSubRun.first = info.tnum; }
  fSubRun.last = info.tnum;
  fSubRun.nevent += 1;

  // accumulate estimated in-memory size
  fMemSize += static_cast<hsize_t>(addBytes);

  // buffer accounting for flush thresholds
  fBufEventCount += 1;
  fBufBytesUsed += addBytes;

  // flush if event-count or byte-size thresholds are exceeded
  if ((fBufEventCap > 0 && fBufEventCount >= fBufEventCap) ||
      (fBufMaxBytes > 0 && fBufBytesUsed >= fBufMaxBytes)) {
    return FlushBuffer();
  }

  return 0;
}

herr_t H5FADCEvent::ReadEvent(int n)
{
  int evtno = n; // Local event index inside the file
  bool file_changed = false;
  hid_t fid = H5I_INVALID_HID;

  // Resolve file ID from chain
  if (fChain && fChain->GetNFile() > 0) { fid = fChain->GetFileId(n, evtno, &file_changed); }
  else {
    fid = fFile;
    if (fCurrentReadFid != fid) { file_changed = true; }
  }

  if (fid < 0) return -1;

  // ---------------------------------------------------------------------
  // 1. File Change Handler (Open datasets & apply cache)
  // ---------------------------------------------------------------------
  if (file_changed) {
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
    if (fFileSpaceWave >= 0) {
      H5Sclose(fFileSpaceWave);
      fFileSpaceWave = H5I_INVALID_HID;
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
    if (fDsetWave >= 0) {
      H5Dclose(fDsetWave);
      fDsetWave = H5I_INVALID_HID;
    }

    // Use a large chunk cache to prevent thrashing
    hid_t dapl = H5Pcreate(H5P_DATASET_ACCESS);
    H5Pset_chunk_cache(dapl, 10007, 128 * 1024 * 1024, 1.0);

    fDsetInfo = H5Dopen2(fid, "/events/info", H5P_DEFAULT);
    fDsetIndex = H5Dopen2(fid, "/events/index", H5P_DEFAULT);
    fDsetChs = H5Dopen2(fid, "/events/chs", H5P_DEFAULT);
    fDsetWave = H5Dopen2(fid, "/events/wave", dapl);

    H5Pclose(dapl);

    if (fDsetInfo < 0 || fDsetIndex < 0 || fDsetChs < 0 || fDsetWave < 0) return -1;

    fFileSpaceInfo = H5Dget_space(fDsetInfo);
    fFileSpaceIndex = H5Dget_space(fDsetIndex);
    fFileSpaceChs = H5Dget_space(fDsetChs);
    fFileSpaceWave = H5Dget_space(fDsetWave);

    fCurrentReadFid = fid;

    // Get NDP safely without ping-pong
    hsize_t wdims[2];
    H5Sget_simple_extent_dims(fFileSpaceWave, wdims, nullptr);
    fNDP = static_cast<int>(wdims[1]);

    // Reset sliding window
    fReadBufStart = -1;
    fReadBufSize = 0;
  }

  // ---------------------------------------------------------------------
  // 2. Dynamic Sliding Window Prefetching (User's Strategy Implementation)
  // ---------------------------------------------------------------------
  if (evtno < fReadBufStart || evtno >= fReadBufStart + fReadBufSize) {

    hsize_t dims[1];
    H5Sget_simple_extent_dims(fFileSpaceInfo, dims, nullptr);
    int total_events_in_file = static_cast<int>(dims[0]);

    // Calculate dynamic safe chunk size based on memory budget (fBufMaxBytes)
    // Assume an average of 4 active channels per event for safety margin
    std::size_t avg_channels = 4;
    std::size_t bytes_per_event =
        sizeof(EventInfo_t) + sizeof(std::uint64_t) +
        avg_channels * (sizeof(FChannelHeader_t) + fNDP * sizeof(std::uint16_t));

    int max_safe_events = static_cast<int>(fBufMaxBytes / bytes_per_event);
    if (max_safe_events <= 0) max_safe_events = 1000; // Fallback

    // Fetch up to the safe limit, or until the end of the current file
    int fetch_size = std::min(max_safe_events, total_events_in_file - evtno);

    if (fetch_size > 0) {
      // Step A: Fetch Metadata
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

      // Step B: Fetch EXACT amount of Waveform Data based on metadata
      std::uint64_t first_ch_global = fReadBufIndex[0];
      std::uint64_t last_ch_global =
          fReadBufIndex[fetch_size - 1] + fReadBufInfo[fetch_size - 1].nhit;
      std::uint64_t total_chs_to_fetch = last_ch_global - first_ch_global;

      if (total_chs_to_fetch > 0) {
        fPrefetchChs.resize(total_chs_to_fetch);
        fPrefetchWave.resize(total_chs_to_fetch * fNDP);

        hsize_t offset_ch[1] = {first_ch_global};
        hsize_t count_ch[1] = {total_chs_to_fetch};
        H5Sselect_hyperslab(fFileSpaceChs, H5S_SELECT_SET, offset_ch, nullptr, count_ch, nullptr);
        hid_t mem_space_chs = H5Screate_simple(1, count_ch, nullptr);
        H5Dread(fDsetChs, fChType, mem_space_chs, fFileSpaceChs, H5P_DEFAULT, fPrefetchChs.data());
        H5Sclose(mem_space_chs);

        hsize_t offset_wave[2] = {first_ch_global, 0};
        hsize_t count_wave[2] = {total_chs_to_fetch, static_cast<hsize_t>(fNDP)};
        H5Sselect_hyperslab(fFileSpaceWave, H5S_SELECT_SET, offset_wave, nullptr, count_wave,
                            nullptr);
        hid_t mem_space_wave = H5Screate_simple(2, count_wave, nullptr);
        H5Dread(fDsetWave, H5T_NATIVE_USHORT, mem_space_wave, fFileSpaceWave, H5P_DEFAULT,
                fPrefetchWave.data());
        H5Sclose(mem_space_wave);
      }

      fReadBufStart = evtno;
      fReadBufSize = fetch_size;
      fPrefetchChStart = first_ch_global;
    }
    else {
      return -1; // Out of bounds
    }
  }

  // ---------------------------------------------------------------------
  // 3. Serve from RAM (Zero HDF5 API overhead in this section)
  // ---------------------------------------------------------------------
  int local_idx = evtno - fReadBufStart;
  fEvtInfo = fReadBufInfo[local_idx];
  std::uint64_t ch_offset_global = fReadBufIndex[local_idx];

  const std::uint16_t nhit = fEvtInfo.nhit;
  fDataBuf.resize(nhit);

  if (nhit > 0) {
    // Calculate where this event's channels start within our prefetch buffer
    std::uint64_t local_ch_idx = ch_offset_global - fPrefetchChStart;

    for (std::size_t ich = 0; ich < nhit; ++ich) {
      fDataBuf[ich].id = fPrefetchChs[local_ch_idx + ich].id;
      fDataBuf[ich].tbit = fPrefetchChs[local_ch_idx + ich].tbit;
      fDataBuf[ich].ped = fPrefetchChs[local_ch_idx + ich].ped;

      std::uint16_t * dst = fDataBuf[ich].waveform;
      const std::uint16_t * src = &fPrefetchWave[(local_ch_idx + ich) * fNDP];

      // Pure memory copy for maximum performance
      std::memcpy(dst, src, static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t));
    }
  }

  return 0;
}