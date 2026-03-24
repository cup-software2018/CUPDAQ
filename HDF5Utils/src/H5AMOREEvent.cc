#include <cstring>

#include "HDF5Utils/H5AMOREEvent.hh"

ClassImp(H5AMOREEvent)

H5AMOREEvent::H5AMOREEvent()
  : AbsH5Event()
{
}

H5AMOREEvent::~H5AMOREEvent()
{
  // Buffers are managed by std::vector
}

void H5AMOREEvent::Open()
{
  // build HDF5 types for event header and crystal header
  fEvtType = EventInfo_t::BuildType();
  fChType = CrystalHeader_t::BuildType();

  // read mode: only build types, no dataset creation
  if (!fWriteTag) { return; }

  // safety: file id must be valid in write mode
  if (fFile < 0) {
    Error("Open", "invalid file id");
    return;
  }

  // check NDP
  if (fNDP <= 0 || fNDP > kH5AMORENDPMAX) {
    Error("Open", "Invalid NDP: %d", fNDP);
    return;
  }

  // create /events group
  {
    htri_t gexists = H5Lexists(fFile, "/events", H5P_DEFAULT);
    if (gexists == 0) {
      hid_t grp_events = H5Gcreate2(fFile, "/events", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      H5Gclose(grp_events);
    }
  }

  // create datasets
  auto create_1d_dataset = [&](const char * name, hid_t dtype, hid_t & dset, hsize_t chunk) {
    hsize_t dims[1] = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hid_t space = H5Screate_simple(1, dims, maxdims);
    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_chunk(dcpl, 1, &chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);
    dset = H5Dcreate2(fFile, name, dtype, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);
    H5Pclose(dcpl);
    H5Sclose(space);
  };

  create_1d_dataset("/events/info", fEvtType, fDsetInfo,
                    (fBufEventCap > 0) ? static_cast<hsize_t>(fBufEventCap) : 32);
  create_1d_dataset("/events/index", H5T_NATIVE_ULLONG, fDsetIndex,
                    (fBufEventCap > 0) ? static_cast<hsize_t>(fBufEventCap) : 32);
  create_1d_dataset("/events/chs", fChType, fDsetChs, 256);

  // create 2d waveform datasets
  auto create_2d_dataset = [&](const char * name, hid_t dtype, hid_t & dset) {
    hsize_t dims[2] = {0, static_cast<hsize_t>(fNDP)};
    hsize_t maxdims[2] = {H5S_UNLIMITED, static_cast<hsize_t>(fNDP)};
    hid_t space = H5Screate_simple(2, dims, maxdims);
    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk[2] = {256, static_cast<hsize_t>(fNDP)};
    H5Pset_chunk(dcpl, 2, chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);
    dset = H5Dcreate2(fFile, name, dtype, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);
    H5Pclose(dcpl);
    H5Sclose(space);
  };

  create_2d_dataset("/events/phonon", H5T_NATIVE_USHORT, fDsetPhonon);
  create_2d_dataset("/events/photon", H5T_NATIVE_USHORT, fDsetPhoton);

  fSubRun.nevent = 0;
  fSubRun.first = 0;
  fSubRun.last = 0;
  fMemSize = 0;
  fTotalEvents = 0;
  fTotalCrystals = 0;

  fEvtBuf.clear();
  fChBuf.clear();
  fPhononBuf.clear();
  fPhotonBuf.clear();
  fBufEventCount = 0;
  fBufBytesUsed = 0;
}

int H5AMOREEvent::GetNDP()
{
  if (fWriteTag) { return fNDP; }
  if (fNDP > 0) { return fNDP; }

  // 💡 FIX: Avoid File Ping-Pong by checking cached dataspace
  if (fFileSpacePhonon >= 0) {
    hsize_t dims[2] = {0, 0};
    H5Sget_simple_extent_dims(fFileSpacePhonon, dims, nullptr);
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

  hid_t dset = H5Dopen2(fid, "/events/phonon", H5P_DEFAULT);
  if (dset < 0) { return 0; }

  hid_t space = H5Dget_space(dset);
  hsize_t dims[2] = {0, 0};
  H5Sget_simple_extent_dims(space, dims, nullptr);
  H5Sclose(space);
  H5Dclose(dset);

  if (dims[1] > 0 && dims[1] <= static_cast<hsize_t>(kH5AMORENDPMAX)) {
    fNDP = static_cast<int>(dims[1]);
  }
  else {
    fNDP = 0;
  }

  return fNDP;
}

herr_t H5AMOREEvent::FlushBuffer()
{
  const std::size_t nEvtBuf = fEvtBuf.size();
  if (nEvtBuf == 0) return 0;

  const std::size_t nChBuf = fChBuf.size();
  if (nChBuf * static_cast<std::size_t>(fNDP) != fPhononBuf.size() ||
      nChBuf * static_cast<std::size_t>(fNDP) != fPhotonBuf.size()) {
    Error("FlushBuffer", "Internal buffer size mismatch");
    return -1;
  }

  herr_t status = 0;

  auto append_1d = [&](hid_t dset, hid_t dtype, hsize_t old_dim, hsize_t count, const void * data) {
    hsize_t new_dim[1] = {old_dim + count};
    H5Dset_extent(dset, new_dim);
    hid_t file_space = H5Dget_space(dset);
    hsize_t offset[1] = {old_dim};
    hsize_t hcount[1] = {count};
    H5Sselect_hyperslab(file_space, H5S_SELECT_SET, offset, nullptr, hcount, nullptr);
    hid_t mem_space = H5Screate_simple(1, hcount, nullptr);
    status = H5Dwrite(dset, dtype, mem_space, file_space, H5P_DEFAULT, data);
    H5Sclose(mem_space);
    H5Sclose(file_space);
  };

  append_1d(fDsetInfo, fEvtType, fTotalEvents, nEvtBuf, fEvtBuf.data());
  if (status < 0) return status;

  std::vector<std::uint64_t> indexBuf(nEvtBuf);
  std::uint64_t base = fTotalCrystals;
  std::uint64_t running = 0;
  for (std::size_t i = 0; i < nEvtBuf; ++i) {
    indexBuf[i] = base + running;
    running += static_cast<std::uint64_t>(fEvtBuf[i].nhit);
  }

  append_1d(fDsetIndex, H5T_NATIVE_ULLONG, fTotalEvents, nEvtBuf, indexBuf.data());
  if (status < 0) return status;

  if (nChBuf > 0) {
    append_1d(fDsetChs, fChType, fTotalCrystals, nChBuf, fChBuf.data());
    if (status < 0) return status;

    auto append_2d = [&](hid_t dset, hid_t dtype, const void * data) {
      hsize_t old_dims[2] = {fTotalCrystals, static_cast<hsize_t>(fNDP)};
      hsize_t new_dims[2] = {fTotalCrystals + nChBuf, static_cast<hsize_t>(fNDP)};
      H5Dset_extent(dset, new_dims);
      hid_t file_space = H5Dget_space(dset);
      hsize_t offset[2] = {old_dims[0], 0};
      hsize_t hcount[2] = {nChBuf, static_cast<hsize_t>(fNDP)};
      H5Sselect_hyperslab(file_space, H5S_SELECT_SET, offset, nullptr, hcount, nullptr);
      hid_t mem_space = H5Screate_simple(2, hcount, nullptr);
      status = H5Dwrite(dset, dtype, mem_space, file_space, H5P_DEFAULT, data);
      H5Sclose(mem_space);
      H5Sclose(file_space);
    };

    append_2d(fDsetPhonon, H5T_NATIVE_USHORT, fPhononBuf.data());
    if (status < 0) return status;

    append_2d(fDsetPhoton, H5T_NATIVE_USHORT, fPhotonBuf.data());
    if (status < 0) return status;
  }

  fTotalEvents += static_cast<std::uint64_t>(nEvtBuf);
  fTotalCrystals += static_cast<std::uint64_t>(nChBuf);

  fEvtBuf.clear();
  fChBuf.clear();
  fPhononBuf.clear();
  fPhotonBuf.clear();
  fBufEventCount = 0;
  fBufBytesUsed = 0;

  return status;
}

void H5AMOREEvent::Close()
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
  if (fFileSpacePhonon >= 0) {
    H5Sclose(fFileSpacePhonon);
    fFileSpacePhonon = H5I_INVALID_HID;
  }
  if (fFileSpacePhoton >= 0) {
    H5Sclose(fFileSpacePhoton);
    fFileSpacePhoton = H5I_INVALID_HID;
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
  if (fDsetPhonon >= 0) {
    H5Dclose(fDsetPhonon);
    fDsetPhonon = H5I_INVALID_HID;
  }
  if (fDsetPhoton >= 0) {
    H5Dclose(fDsetPhoton);
    fDsetPhoton = H5I_INVALID_HID;
  }

  fCurrentReadFid = H5I_INVALID_HID;

  // Reset hybrid bulk load buffers
  fReadBufStart = -1;
  fReadBufSize = 0;
  fReadBufInfo.clear();
  fReadBufIndex.clear();
  fPrefetchChs.clear();
  fPrefetchPhonon.clear();
  fPrefetchPhoton.clear();

  if (fEvtType >= 0) {
    H5Tclose(fEvtType);
    fEvtType = H5I_INVALID_HID;
  }
  if (fChType >= 0) {
    H5Tclose(fChType);
    fChType = H5I_INVALID_HID;
  }
}

herr_t H5AMOREEvent::AppendEvent(const EventInfo_t & info, const std::vector<Crystal_t> & data)
{
  if (!fWriteTag) return -1;
  if (fNDP <= 0 || fNDP > kH5AMORENDPMAX) return -1;

  const std::uint16_t nhit = static_cast<std::uint16_t>(data.size());

  EventInfo_t info_local = info;
  info_local.nhit = nhit;
  fEvtBuf.push_back(info_local);

  std::size_t addBytes = sizeof(EventInfo_t);
  addBytes +=
      static_cast<std::size_t>(nhit) *
      (sizeof(CrystalHeader_t) + 2u * static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t));

  for (std::size_t i = 0; i < nhit; ++i) {
    CrystalHeader_t h{};
    h.id = data[i].id;
    h.ttime = data[i].ttime;
    fChBuf.push_back(h);

    const std::uint16_t * srcPn = data[i].phonon;
    const std::uint16_t * srcPt = data[i].photon;
    fPhononBuf.insert(fPhononBuf.end(), srcPn, srcPn + static_cast<std::size_t>(fNDP));
    fPhotonBuf.insert(fPhotonBuf.end(), srcPt, srcPt + static_cast<std::size_t>(fNDP));
  }

  if (fSubRun.nevent == 0) { fSubRun.first = info.tnum; }
  fSubRun.last = info.tnum;
  fSubRun.nevent += 1;

  fMemSize += static_cast<hsize_t>(addBytes);
  fBufEventCount += 1;
  fBufBytesUsed += addBytes;

  if ((fBufEventCap > 0 && fBufEventCount >= fBufEventCap) ||
      (fBufMaxBytes > 0 && fBufBytesUsed >= fBufMaxBytes)) {
    return FlushBuffer();
  }

  return 0;
}

herr_t H5AMOREEvent::ReadEvent(int n)
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

  // ---------------------------------------------------------------------
  // 1. File Change Handler
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
    if (fFileSpacePhonon >= 0) {
      H5Sclose(fFileSpacePhonon);
      fFileSpacePhonon = H5I_INVALID_HID;
    }
    if (fFileSpacePhoton >= 0) {
      H5Sclose(fFileSpacePhoton);
      fFileSpacePhoton = H5I_INVALID_HID;
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
    if (fDsetPhonon >= 0) {
      H5Dclose(fDsetPhonon);
      fDsetPhonon = H5I_INVALID_HID;
    }
    if (fDsetPhoton >= 0) {
      H5Dclose(fDsetPhoton);
      fDsetPhoton = H5I_INVALID_HID;
    }

    hid_t dapl = H5Pcreate(H5P_DATASET_ACCESS);
    H5Pset_chunk_cache(dapl, 10007, 128 * 1024 * 1024, 1.0);

    fDsetInfo = H5Dopen2(fid, "/events/info", H5P_DEFAULT);
    fDsetIndex = H5Dopen2(fid, "/events/index", H5P_DEFAULT);
    fDsetChs = H5Dopen2(fid, "/events/chs", H5P_DEFAULT);
    fDsetPhonon = H5Dopen2(fid, "/events/phonon", dapl);
    fDsetPhoton = H5Dopen2(fid, "/events/photon", dapl);

    H5Pclose(dapl);

    if (fDsetInfo < 0 || fDsetIndex < 0 || fDsetChs < 0 || fDsetPhonon < 0 || fDsetPhoton < 0)
      return -1;

    fFileSpaceInfo = H5Dget_space(fDsetInfo);
    fFileSpaceIndex = H5Dget_space(fDsetIndex);
    fFileSpaceChs = H5Dget_space(fDsetChs);
    fFileSpacePhonon = H5Dget_space(fDsetPhonon);
    fFileSpacePhoton = H5Dget_space(fDsetPhoton);

    fCurrentReadFid = fid;

    // Get NDP safely
    hsize_t wdims[2];
    H5Sget_simple_extent_dims(fFileSpacePhonon, wdims, nullptr);
    fNDP = static_cast<int>(wdims[1]);

    fReadBufStart = -1;
    fReadBufSize = 0;
  }

  // ---------------------------------------------------------------------
  // 2. Dynamic Sliding Window Prefetching (TWO waveforms included)
  // ---------------------------------------------------------------------
  if (evtno < fReadBufStart || evtno >= fReadBufStart + fReadBufSize) {

    hsize_t dims[1];
    H5Sget_simple_extent_dims(fFileSpaceInfo, dims, nullptr);
    int total_events_in_file = static_cast<int>(dims[0]);

    // Safety margin assumption: average 2 active crystals per event
    std::size_t avg_channels = 2;
    std::size_t bytes_per_event =
        sizeof(EventInfo_t) + sizeof(std::uint64_t) +
        avg_channels * (sizeof(CrystalHeader_t) + 2 * fNDP * sizeof(std::uint16_t));

    int max_safe_events = static_cast<int>(fBufMaxBytes / bytes_per_event);
    if (max_safe_events <= 0) max_safe_events = 100; // Fallback for extremely large NDP

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

      // Step B: Fetch EXACT amount of Two Waveforms Data
      std::uint64_t first_ch_global = fReadBufIndex[0];
      std::uint64_t last_ch_global =
          fReadBufIndex[fetch_size - 1] + fReadBufInfo[fetch_size - 1].nhit;
      std::uint64_t total_chs_to_fetch = last_ch_global - first_ch_global;

      if (total_chs_to_fetch > 0) {
        fPrefetchChs.resize(total_chs_to_fetch);
        fPrefetchPhonon.resize(total_chs_to_fetch * fNDP);
        fPrefetchPhoton.resize(total_chs_to_fetch * fNDP);

        // Fetch Headers
        hsize_t offset_ch[1] = {first_ch_global};
        hsize_t count_ch[1] = {total_chs_to_fetch};
        H5Sselect_hyperslab(fFileSpaceChs, H5S_SELECT_SET, offset_ch, nullptr, count_ch, nullptr);
        hid_t mem_space_chs = H5Screate_simple(1, count_ch, nullptr);
        H5Dread(fDsetChs, fChType, mem_space_chs, fFileSpaceChs, H5P_DEFAULT, fPrefetchChs.data());
        H5Sclose(mem_space_chs);

        // Fetch Phonon and Photon
        hsize_t offset_wave[2] = {first_ch_global, 0};
        hsize_t count_wave[2] = {total_chs_to_fetch, static_cast<hsize_t>(fNDP)};

        H5Sselect_hyperslab(fFileSpacePhonon, H5S_SELECT_SET, offset_wave, nullptr, count_wave,
                            nullptr);
        H5Sselect_hyperslab(fFileSpacePhoton, H5S_SELECT_SET, offset_wave, nullptr, count_wave,
                            nullptr);
        hid_t mem_space_wave = H5Screate_simple(2, count_wave, nullptr);

        H5Dread(fDsetPhonon, H5T_NATIVE_USHORT, mem_space_wave, fFileSpacePhonon, H5P_DEFAULT,
                fPrefetchPhonon.data());
        H5Dread(fDsetPhoton, H5T_NATIVE_USHORT, mem_space_wave, fFileSpacePhoton, H5P_DEFAULT,
                fPrefetchPhoton.data());

        H5Sclose(mem_space_wave);
      }

      fReadBufStart = evtno;
      fReadBufSize = fetch_size;
      fPrefetchChStart = first_ch_global;
    }
    else {
      return -1;
    }
  }

  // ---------------------------------------------------------------------
  // 3. Serve from RAM (Zero HDF5 API overhead)
  // ---------------------------------------------------------------------
  int local_idx = evtno - fReadBufStart;
  fEvtInfo = fReadBufInfo[local_idx];
  std::uint64_t ch_offset_global = fReadBufIndex[local_idx];

  const std::uint16_t nhit = fEvtInfo.nhit;
  fDataBuf.resize(nhit);

  if (nhit > 0) {
    std::uint64_t local_ch_idx = ch_offset_global - fPrefetchChStart;

    for (std::size_t i = 0; i < nhit; ++i) {
      fDataBuf[i].id = fPrefetchChs[local_ch_idx + i].id;
      fDataBuf[i].ttime = fPrefetchChs[local_ch_idx + i].ttime;

      std::uint16_t * dstPn = fDataBuf[i].phonon;
      std::uint16_t * dstPt = fDataBuf[i].photon;

      const std::uint16_t * srcPn = &fPrefetchPhonon[(local_ch_idx + i) * fNDP];
      const std::uint16_t * srcPt = &fPrefetchPhoton[(local_ch_idx + i) * fNDP];

      std::memcpy(dstPn, srcPn, static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t));
      std::memcpy(dstPt, srcPt, static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t));
    }
  }

  return 0;
}