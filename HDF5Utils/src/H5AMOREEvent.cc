#include <cstring>

#include "HDF5Utils/H5AMOREEvent.hh"

ClassImp(H5AMOREEvent)

H5AMOREEvent::H5AMOREEvent()
  : AbsH5Event()
{
}

H5AMOREEvent::~H5AMOREEvent()
{
  // free per-event read buffer
  if (fData) {
    delete[] fData;
    fData = nullptr;
  }
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
    Error("Open", "invalid file id (fFile = %d). SetFileId must be called before Open().", static_cast<int>(fFile));
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

    fDsetIndex = H5Dcreate2(fFile, "/events/index", H5T_NATIVE_ULLONG, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

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

    fDsetPhonon = H5Dcreate2(fFile, "/events/phonon", H5T_NATIVE_USHORT, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

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

    fDsetPhoton = H5Dcreate2(fFile, "/events/photon", H5T_NATIVE_USHORT, space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

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

  if (dims[1] > 0 && dims[1] <= static_cast<hsize_t>(kH5AMORENDPMAX)) { fNDP = static_cast<int>(dims[1]); }
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

    status = H5Dwrite(fDsetIndex, H5T_NATIVE_ULLONG, mem_space, file_space, H5P_DEFAULT, indexBuf.data());

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

    status = H5Dwrite(fDsetPhonon, H5T_NATIVE_USHORT, mem_space_wave, file_space_ph, H5P_DEFAULT, fPhononBuf.data());
    if (status >= 0) {
      status = H5Dwrite(fDsetPhoton, H5T_NATIVE_USHORT, mem_space_wave, file_space_pt, H5P_DEFAULT, fPhotonBuf.data());
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
  if (fWriteTag) {
    // flush remaining buffered events before closing datasets
    FlushBuffer();

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
  }

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
  addBytes += static_cast<std::size_t>(nhit) *
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
  if ((fBufEventCap > 0 && fBufEventCount >= fBufEventCap) || (fBufMaxBytes > 0 && fBufBytesUsed >= fBufMaxBytes)) {
    return FlushBuffer();
  }

  return 0;
}

herr_t H5AMOREEvent::ReadEvent(int n)
{
  // resolve (file, local event index) from chain if needed
  int evtno;
  hid_t fid = fChain && fChain->GetNFile() > 0 ? fChain->GetFileId(n, evtno) : fFile;

  // open all datasets needed for this event
  hid_t dset_info = H5Dopen2(fid, "/events/info", H5P_DEFAULT);
  hid_t dset_index = H5Dopen2(fid, "/events/index", H5P_DEFAULT);
  hid_t dset_chs = H5Dopen2(fid, "/events/chs", H5P_DEFAULT);
  hid_t dset_phonon = H5Dopen2(fid, "/events/phonon", H5P_DEFAULT);
  hid_t dset_photon = H5Dopen2(fid, "/events/photon", H5P_DEFAULT);

  if (dset_info < 0 || dset_index < 0 || dset_chs < 0 || dset_phonon < 0 || dset_photon < 0) {
    if (dset_info >= 0) H5Dclose(dset_info);
    if (dset_index >= 0) H5Dclose(dset_index);
    if (dset_chs >= 0) H5Dclose(dset_chs);
    if (dset_phonon >= 0) H5Dclose(dset_phonon);
    if (dset_photon >= 0) H5Dclose(dset_photon);
    return -1;
  }

  herr_t status = 0;

  // read one event header from /events/info
  hsize_t offset_evt[1] = {static_cast<hsize_t>(evtno)};
  hsize_t count_evt[1] = {1};

  hid_t file_space_info = H5Dget_space(dset_info);
  H5Sselect_hyperslab(file_space_info, H5S_SELECT_SET, offset_evt, nullptr, count_evt, nullptr);

  hid_t mem_space_info = H5Screate_simple(1, count_evt, nullptr);

  status = H5Dread(dset_info, fEvtType, mem_space_info, file_space_info, H5P_DEFAULT, &fEvtInfo);

  H5Sclose(mem_space_info);
  H5Sclose(file_space_info);

  if (status < 0) {
    H5Dclose(dset_info);
    H5Dclose(dset_index);
    H5Dclose(dset_chs);
    H5Dclose(dset_phonon);
    H5Dclose(dset_photon);
    return status;
  }

  // read crystal start index for this event from /events/index
  std::uint64_t offset_value = 0;

  hid_t file_space_idx = H5Dget_space(dset_index);
  H5Sselect_hyperslab(file_space_idx, H5S_SELECT_SET, offset_evt, nullptr, count_evt, nullptr);

  hid_t mem_space_idx = H5Screate_simple(1, count_evt, nullptr);

  status = H5Dread(dset_index, H5T_NATIVE_ULLONG, mem_space_idx, file_space_idx, H5P_DEFAULT, &offset_value);

  H5Sclose(mem_space_idx);
  H5Sclose(file_space_idx);

  if (status < 0) {
    H5Dclose(dset_info);
    H5Dclose(dset_index);
    H5Dclose(dset_chs);
    H5Dclose(dset_phonon);
    H5Dclose(dset_photon);
    return status;
  }

  const std::uint16_t nhit = fEvtInfo.nhit;

  // (re)allocate per-event crystal buffer
  if (fData) {
    delete[] fData;
    fData = nullptr;
  }
  fData = (nhit > 0) ? new Crystal_t[nhit] : nullptr;

  if (nhit > 0) {
    // make sure NDP is known in read mode
    GetNDP();

    if (fNDP <= 0 || fNDP > kH5AMORENDPMAX) {
      Error("ReadEvent", "Invalid NDP: %d (max %d)", fNDP, kH5AMORENDPMAX);
      H5Dclose(dset_info);
      H5Dclose(dset_index);
      H5Dclose(dset_chs);
      H5Dclose(dset_phonon);
      H5Dclose(dset_photon);
      return -1;
    }

    // read crystal headers
    std::vector<CrystalHeader_t> headers(nhit);

    hsize_t offset_ch[1] = {static_cast<hsize_t>(offset_value)};
    hsize_t count_ch[1] = {static_cast<hsize_t>(nhit)};

    hid_t file_space_chs = H5Dget_space(dset_chs);
    H5Sselect_hyperslab(file_space_chs, H5S_SELECT_SET, offset_ch, nullptr, count_ch, nullptr);

    hid_t mem_space_chs = H5Screate_simple(1, count_ch, nullptr);

    status = H5Dread(dset_chs, fChType, mem_space_chs, file_space_chs, H5P_DEFAULT, headers.data());

    H5Sclose(mem_space_chs);
    H5Sclose(file_space_chs);

    if (status < 0) {
      H5Dclose(dset_info);
      H5Dclose(dset_index);
      H5Dclose(dset_chs);
      H5Dclose(dset_phonon);
      H5Dclose(dset_photon);
      return status;
    }

    // read phonon and photon waveforms
    std::vector<std::uint16_t> bufPn(static_cast<std::size_t>(nhit) * static_cast<std::size_t>(fNDP));
    std::vector<std::uint16_t> bufPt(static_cast<std::size_t>(nhit) * static_cast<std::size_t>(fNDP));

    hsize_t offset_wave[2] = {static_cast<hsize_t>(offset_value), 0};
    hsize_t count_wave[2] = {static_cast<hsize_t>(nhit), static_cast<hsize_t>(fNDP)};

    hid_t file_space_ph = H5Dget_space(dset_phonon);
    hid_t file_space_pt = H5Dget_space(dset_photon);

    H5Sselect_hyperslab(file_space_ph, H5S_SELECT_SET, offset_wave, nullptr, count_wave, nullptr);
    H5Sselect_hyperslab(file_space_pt, H5S_SELECT_SET, offset_wave, nullptr, count_wave, nullptr);

    hid_t mem_space_wave = H5Screate_simple(2, count_wave, nullptr);

    status = H5Dread(dset_phonon, H5T_NATIVE_USHORT, mem_space_wave, file_space_ph, H5P_DEFAULT, bufPn.data());
    if (status >= 0) {
      status = H5Dread(dset_photon, H5T_NATIVE_USHORT, mem_space_wave, file_space_pt, H5P_DEFAULT, bufPt.data());
    }

    H5Sclose(mem_space_wave);
    H5Sclose(file_space_ph);
    H5Sclose(file_space_pt);

    if (status < 0) {
      H5Dclose(dset_info);
      H5Dclose(dset_index);
      H5Dclose(dset_chs);
      H5Dclose(dset_phonon);
      H5Dclose(dset_photon);
      return status;
    }

    // fill user-facing Crystal_t array
    for (std::size_t i = 0; i < nhit; ++i) {
      fData[i].id = headers[i].id;
      fData[i].ttime = headers[i].ttime;

      std::uint16_t * dstPn = fData[i].phonon;
      std::uint16_t * dstPt = fData[i].photon;
      const std::uint16_t * srcPn = &bufPn[i * static_cast<std::size_t>(fNDP)];
      const std::uint16_t * srcPt = &bufPt[i * static_cast<std::size_t>(fNDP)];

      std::memcpy(dstPn, srcPn, static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t));
      std::memcpy(dstPt, srcPt, static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t));
    }
  }

  // close datasets used for this read
  H5Dclose(dset_info);
  H5Dclose(dset_index);
  H5Dclose(dset_chs);
  H5Dclose(dset_phonon);
  H5Dclose(dset_photon);

  return status;
}
