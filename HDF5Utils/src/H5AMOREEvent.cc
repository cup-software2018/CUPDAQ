#include <cstring>

#include "HDF5Utils/H5AMOREEvent.hh"

ClassImp(H5AMOREEvent)

H5AMOREEvent::H5AMOREEvent()
  : AbsH5Event()
{
}

H5AMOREEvent::~H5AMOREEvent()
{
  // fDataBuf is managed by std::vector, no manual deletion needed.
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
    Error("Open", "invalid file id (fFile = %d). SetFileId must be called before Open().",
          static_cast<int>(fFile));
    return;
  }

  // check NDP (number of data points per waveform)
  if (fNDP <= 0 || fNDP > kH5AMORENDPMAX) {
    Error("Open", "Invalid NDP: %d (max %d)", fNDP, kH5AMORENDPMAX);
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

  // (optional) commit types in this file for inspection / reuse
  H5Tcommit2(fFile, "evttype", fEvtType, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Tcommit2(fFile, "crystalheadertype", fChType, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  // create extendable dataset: /events/info (per–event metadata)
  {
    hsize_t dims[1] = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hid_t space = H5Screate_simple(1, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk = (fBufEventCap > 0) ? static_cast<hsize_t>(fBufEventCap) : 32;
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

  // create extendable dataset: /events/index (event → first crystal index)
  {
    hsize_t dims[1] = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hid_t space = H5Screate_simple(1, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk = (fBufEventCap > 0) ? static_cast<hsize_t>(fBufEventCap) : 32;
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

  // create extendable dataset: /events/chs (crystal headers)
  {
    hsize_t dims[1] = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hid_t space = H5Screate_simple(1, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk = 256;
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

  // create extendable dataset: /events/phonon (phonon waveforms)
  {
    hsize_t dims[2] = {0, static_cast<hsize_t>(fNDP)};
    hsize_t maxdims[2] = {H5S_UNLIMITED, static_cast<hsize_t>(fNDP)};
    hid_t space = H5Screate_simple(2, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk[2] = {256, static_cast<hsize_t>(fNDP)};
    H5Pset_chunk(dcpl, 2, chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);

    fDsetPhonon = H5Dcreate2(fFile, "/events/phonon", H5T_NATIVE_USHORT, space, H5P_DEFAULT, dcpl,
                             H5P_DEFAULT);

    H5Pclose(dcpl);
    H5Sclose(space);

    if (fDsetPhonon < 0) {
      Error("Open", "Failed to create dataset /events/phonon");
      return;
    }
  }

  // create extendable dataset: /events/photon (photon waveforms)
  {
    hsize_t dims[2] = {0, static_cast<hsize_t>(fNDP)};
    hsize_t maxdims[2] = {H5S_UNLIMITED, static_cast<hsize_t>(fNDP)};
    hid_t space = H5Screate_simple(2, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk[2] = {256, static_cast<hsize_t>(fNDP)};
    H5Pset_chunk(dcpl, 2, chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);

    fDsetPhoton = H5Dcreate2(fFile, "/events/photon", H5T_NATIVE_USHORT, space, H5P_DEFAULT, dcpl,
                             H5P_DEFAULT);

    H5Pclose(dcpl);
    H5Sclose(space);

    if (fDsetPhoton < 0) {
      Error("Open", "Failed to create dataset /events/photon");
      return;
    }
  }

  // initialize subrun and internal counters
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
  // in write mode, use configured NDP
  if (fWriteTag) { return fNDP; }

  // cached value already known
  if (fNDP > 0) { return fNDP; }

  hid_t fid = H5I_INVALID_HID;
  int dummy_evt = 0;

  // resolve file id from chain if available
  if (fChain && fChain->GetNFile() > 0) { fid = fChain->GetFileId(0, dummy_evt); }
  else if (fFile >= 0) {
    fid = fFile;
  }

  if (fid < 0) { return 0; }

  // infer NDP from /events/phonon dataset second dimension
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
  // nothing to flush
  const std::size_t nEvtBuf = fEvtBuf.size();
  if (nEvtBuf == 0) {
    fBufEventCount = 0;
    fBufBytesUsed = 0;
    return 0;
  }

  const std::size_t nChBuf = fChBuf.size();
  // phonon / photon buffers must be consistent with NDP and nch
  if (nChBuf * static_cast<std::size_t>(fNDP) != fPhononBuf.size() ||
      nChBuf * static_cast<std::size_t>(fNDP) != fPhotonBuf.size()) {
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

  // build and append event → first-crystal index map
  {
    std::vector<std::uint64_t> indexBuf(nEvtBuf);
    std::uint64_t base = fTotalCrystals;
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

  // append crystal headers and waveforms
  if (nChBuf > 0) {
    // /events/chs
    hsize_t old_dim[1] = {fTotalCrystals};
    hsize_t new_dim[1] = {fTotalCrystals + nChBuf};
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

    // /events/phonon and /events/photon
    hsize_t old_dims[2] = {fTotalCrystals, static_cast<hsize_t>(fNDP)};
    hsize_t new_dims[2] = {fTotalCrystals + nChBuf, static_cast<hsize_t>(fNDP)};
    H5Dset_extent(fDsetPhonon, new_dims);
    H5Dset_extent(fDsetPhoton, new_dims);

    hid_t file_space_ph = H5Dget_space(fDsetPhonon);
    hid_t file_space_pt = H5Dget_space(fDsetPhoton);

    hsize_t offset_wave[2] = {old_dims[0], 0};
    hsize_t count_wave[2] = {nChBuf, static_cast<hsize_t>(fNDP)};

    H5Sselect_hyperslab(file_space_ph, H5S_SELECT_SET, offset_wave, nullptr, count_wave, nullptr);
    H5Sselect_hyperslab(file_space_pt, H5S_SELECT_SET, offset_wave, nullptr, count_wave, nullptr);

    hid_t mem_space_wave = H5Screate_simple(2, count_wave, nullptr);

    status = H5Dwrite(fDsetPhonon, H5T_NATIVE_USHORT, mem_space_wave, file_space_ph, H5P_DEFAULT,
                      fPhononBuf.data());
    if (status >= 0) {
      status = H5Dwrite(fDsetPhoton, H5T_NATIVE_USHORT, mem_space_wave, file_space_pt, H5P_DEFAULT,
                        fPhotonBuf.data());
    }

    H5Sclose(mem_space_wave);
    H5Sclose(file_space_ph);
    H5Sclose(file_space_pt);

    if (status < 0) { return status; }
  }

  // update global counters and reset buffers
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
  if (fMemSpaceEvt >= 0) {
    H5Sclose(fMemSpaceEvt);
    fMemSpaceEvt = H5I_INVALID_HID;
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

  fCurrentReadFid = H5I_INVALID_HID; // Reset tracker

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

herr_t H5AMOREEvent::AppendEvent(const EventInfo_t & info, const std::vector<Crystal_t> & data)
{
  if (!fWriteTag) { return -1; }
  if (fNDP <= 0 || fNDP > kH5AMORENDPMAX) {
    Error("AppendEvent", "Invalid NDP: %d (max %d)", fNDP, kH5AMORENDPMAX);
    return -1;
  }

  // number of hit crystals in this event
  const std::uint16_t nhit = static_cast<std::uint16_t>(data.size());

  // store event header in buffer (with nhit filled)
  EventInfo_t info_local = info;
  info_local.nhit = nhit;
  fEvtBuf.push_back(info_local);

  // approximate memory usage for this event (metadata + all crystals)
  std::size_t addBytes = sizeof(EventInfo_t);
  addBytes +=
      static_cast<std::size_t>(nhit) *
      (sizeof(CrystalHeader_t) + 2u * static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t));

  // copy crystal headers and waveforms into flat buffers
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

  // update subrun information
  if (fSubRun.nevent == 0) { fSubRun.first = info.tnum; }
  fSubRun.last = info.tnum;
  fSubRun.nevent += 1;

  // accumulate estimated in–memory size
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

herr_t H5AMOREEvent::ReadEvent(int n)
{
  int evtno = n;
  bool file_changed = false;
  hid_t fid = H5I_INVALID_HID;

  // resolve (file, local event index) from chain if needed
  if (fChain && fChain->GetNFile() > 0) { fid = fChain->GetFileId(n, evtno, &file_changed); }
  else {
    fid = fFile;
    if (fCurrentReadFid != fid) { file_changed = true; }
  }

  if (fid < 0) return -1;

  // Optimization 1 & 2: Open datasets and cache DataSpaces only when file switches
  if (file_changed) {
    // Close existing DataSpaces
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
    if (fMemSpaceEvt >= 0) {
      H5Sclose(fMemSpaceEvt);
      fMemSpaceEvt = H5I_INVALID_HID;
    }

    // Close existing Datasets
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

    // Tuning: Use DAPL to increase chunk cache for massive waveforms (32MB)
    hid_t dapl = H5Pcreate(H5P_DATASET_ACCESS);
    H5Pset_chunk_cache(dapl, 10007, 32 * 1024 * 1024, 1.0);

    fDsetInfo = H5Dopen2(fid, "/events/info", H5P_DEFAULT);
    fDsetIndex = H5Dopen2(fid, "/events/index", H5P_DEFAULT);
    fDsetChs = H5Dopen2(fid, "/events/chs", H5P_DEFAULT);
    fDsetPhonon = H5Dopen2(fid, "/events/phonon", dapl);
    fDsetPhoton = H5Dopen2(fid, "/events/photon", dapl);

    H5Pclose(dapl);

    if (fDsetInfo < 0 || fDsetIndex < 0 || fDsetChs < 0 || fDsetPhonon < 0 || fDsetPhoton < 0) {
      // Cleanup on failure handled by Close()
      return -1;
    }

    // Cache DataSpaces
    fFileSpaceInfo = H5Dget_space(fDsetInfo);
    fFileSpaceIndex = H5Dget_space(fDsetIndex);
    fFileSpaceChs = H5Dget_space(fDsetChs);
    fFileSpacePhonon = H5Dget_space(fDsetPhonon);
    fFileSpacePhoton = H5Dget_space(fDsetPhoton);

    // Create a simple memory space for 1 event
    hsize_t count_evt[1] = {1};
    fMemSpaceEvt = H5Screate_simple(1, count_evt, nullptr);

    fCurrentReadFid = fid;
    fNDP = 0; // Reset cached NDP for new file
    GetNDP();
  }

  herr_t status = 0;

  // Read one event header from /events/info
  hsize_t offset_evt[1] = {static_cast<hsize_t>(evtno)};
  hsize_t count_evt[1] = {1};

  H5Sselect_hyperslab(fFileSpaceInfo, H5S_SELECT_SET, offset_evt, nullptr, count_evt, nullptr);
  status = H5Dread(fDsetInfo, fEvtType, fMemSpaceEvt, fFileSpaceInfo, H5P_DEFAULT, &fEvtInfo);

  if (status < 0) { return status; }

  // Read crystal start index for this event from /events/index
  std::uint64_t offset_value = 0;

  H5Sselect_hyperslab(fFileSpaceIndex, H5S_SELECT_SET, offset_evt, nullptr, count_evt, nullptr);
  status = H5Dread(fDsetIndex, H5T_NATIVE_ULLONG, fMemSpaceEvt, fFileSpaceIndex, H5P_DEFAULT,
                   &offset_value);

  if (status < 0) { return status; }

  const std::uint16_t nhit = fEvtInfo.nhit;

  // Optimization 3: Resize vector buffer to avoid repeated memory allocation
  fDataBuf.resize(nhit);

  if (nhit > 0) {
    if (fNDP <= 0 || fNDP > kH5AMORENDPMAX) {
      Error("ReadEvent", "Invalid NDP: %d (max %d)", fNDP, kH5AMORENDPMAX);
      return -1;
    }

    // Read crystal headers
    std::vector<CrystalHeader_t> headers(nhit);

    hsize_t offset_ch[1] = {static_cast<hsize_t>(offset_value)};
    hsize_t count_ch[1] = {static_cast<hsize_t>(nhit)};

    H5Sselect_hyperslab(fFileSpaceChs, H5S_SELECT_SET, offset_ch, nullptr, count_ch, nullptr);
    hid_t mem_space_chs = H5Screate_simple(1, count_ch, nullptr);
    status = H5Dread(fDsetChs, fChType, mem_space_chs, fFileSpaceChs, H5P_DEFAULT, headers.data());
    H5Sclose(mem_space_chs);

    if (status < 0) { return status; }

    // Read phonon and photon waveforms
    std::vector<std::uint16_t> bufPn(static_cast<std::size_t>(nhit) *
                                     static_cast<std::size_t>(fNDP));
    std::vector<std::uint16_t> bufPt(static_cast<std::size_t>(nhit) *
                                     static_cast<std::size_t>(fNDP));

    hsize_t offset_wave[2] = {static_cast<hsize_t>(offset_value), 0};
    hsize_t count_wave[2] = {static_cast<hsize_t>(nhit), static_cast<hsize_t>(fNDP)};

    H5Sselect_hyperslab(fFileSpacePhonon, H5S_SELECT_SET, offset_wave, nullptr, count_wave,
                        nullptr);
    H5Sselect_hyperslab(fFileSpacePhoton, H5S_SELECT_SET, offset_wave, nullptr, count_wave,
                        nullptr);

    hid_t mem_space_wave = H5Screate_simple(2, count_wave, nullptr);

    status = H5Dread(fDsetPhonon, H5T_NATIVE_USHORT, mem_space_wave, fFileSpacePhonon, H5P_DEFAULT,
                     bufPn.data());
    if (status >= 0) {
      status = H5Dread(fDsetPhoton, H5T_NATIVE_USHORT, mem_space_wave, fFileSpacePhoton,
                       H5P_DEFAULT, bufPt.data());
    }
    H5Sclose(mem_space_wave);

    if (status < 0) { return status; }

    // Fill user-facing Crystal_t array
    for (std::size_t i = 0; i < nhit; ++i) {
      fDataBuf[i].id = headers[i].id;
      fDataBuf[i].ttime = headers[i].ttime;

      std::uint16_t * dstPn = fDataBuf[i].phonon;
      std::uint16_t * dstPt = fDataBuf[i].photon;
      const std::uint16_t * srcPn = &bufPn[i * static_cast<std::size_t>(fNDP)];
      const std::uint16_t * srcPt = &bufPt[i * static_cast<std::size_t>(fNDP)];

      std::memcpy(dstPn, srcPn, static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t));
      std::memcpy(dstPt, srcPt, static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t));
    }
  }

  return status;
}