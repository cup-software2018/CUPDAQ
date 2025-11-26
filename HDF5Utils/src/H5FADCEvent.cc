#include "HDF5Utils/H5FADCEvent.hh"

#include <cstring>

ClassImp(H5FADCEvent)

H5FADCEvent::H5FADCEvent()
  : AbsH5Event()
{
}

H5FADCEvent::~H5FADCEvent()
{
  if (fData) {
    delete[] fData;
    fData = nullptr;
  }
}

void H5FADCEvent::Open()
{
  fEvtType = EventInfo_t::BuildType();
  fChType  = FChannelHeader_t::BuildType();

  if (!fWriteTag) {
    return;
  }

  if (fNDP <= 0 || fNDP > kH5FADCNDPMAX) {
    Error("Open", "Invalid NDP: %d (max %d)", fNDP, kH5FADCNDPMAX);
    return;
  }

  hid_t grp_events = H5Gcreate2(fFile, "/events",
                                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  if (grp_events >= 0) {
    H5Gclose(grp_events);
  }

  H5Tcommit2(fFile, "evttype", fEvtType,
             H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Tcommit2(fFile, "fchheadertype", fChType,
             H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  {
    hsize_t dims[1]    = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hid_t space = H5Screate_simple(1, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk = (fBufEventCap > 0) ? static_cast<hsize_t>(fBufEventCap) : 1024;
    H5Pset_chunk(dcpl, 1, &chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);

    fDsetInfo = H5Dcreate2(fFile, "/events/info", fEvtType,
                           space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

    H5Pclose(dcpl);
    H5Sclose(space);
  }

  {
    hsize_t dims[1]    = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hid_t space = H5Screate_simple(1, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk = (fBufEventCap > 0) ? static_cast<hsize_t>(fBufEventCap) : 1024;
    H5Pset_chunk(dcpl, 1, &chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);

    fDsetIndex = H5Dcreate2(fFile, "/events/index", H5T_NATIVE_ULLONG,
                            space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

    H5Pclose(dcpl);
    H5Sclose(space);
  }

  {
    hsize_t dims[1]    = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hid_t space = H5Screate_simple(1, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk = 1024;
    H5Pset_chunk(dcpl, 1, &chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);

    fDsetChs = H5Dcreate2(fFile, "/events/chs", fChType,
                          space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

    H5Pclose(dcpl);
    H5Sclose(space);
  }

  {
    hsize_t dims[2]    = {0, static_cast<hsize_t>(fNDP)};
    hsize_t maxdims[2] = {H5S_UNLIMITED, static_cast<hsize_t>(fNDP)};
    hid_t space = H5Screate_simple(2, dims, maxdims);

    hid_t dcpl = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk[2] = {1024, static_cast<hsize_t>(fNDP)};
    H5Pset_chunk(dcpl, 2, chunk);
    H5Pset_deflate(dcpl, fCompressionLevel);

    fDsetWave = H5Dcreate2(fFile, "/events/wave", H5T_NATIVE_USHORT,
                           space, H5P_DEFAULT, dcpl, H5P_DEFAULT);

    H5Pclose(dcpl);
    H5Sclose(space);
  }

  fSubRun.nevent = 0;
  fSubRun.first  = 0;
  fSubRun.last   = 0;

  fMemSize       = 0;
  fTotalEvents   = 0;
  fTotalChannels = 0;

  fEvtBuf.clear();
  fChBuf.clear();
  fWaveBuf.clear();
  fBufEventCount = 0;
  fBufBytesUsed  = 0;
}

int H5FADCEvent::GetNDP()
{
  if (fWriteTag) {
    return fNDP;
  }

  if (fNDP > 0) {
    return fNDP;
  }

  hid_t fid = H5I_INVALID_HID;
  int dummy_evt = 0;

  if (fChain && fChain->GetNFile() > 0) {
    fid = fChain->GetFileId(0, dummy_evt);
  } else if (fFile >= 0) {
    fid = fFile;
  }

  if (fid < 0) {
    return 0;
  }

  hid_t dset = H5Dopen2(fid, "/events/wave", H5P_DEFAULT);
  if (dset < 0) {
    return 0;
  }

  hid_t space = H5Dget_space(dset);
  hsize_t dims[2] = {0, 0};
  H5Sget_simple_extent_dims(space, dims, nullptr);
  H5Sclose(space);
  H5Dclose(dset);

  if (dims[1] > 0 && dims[1] <= static_cast<hsize_t>(kH5FADCNDPMAX)) {
    fNDP = static_cast<int>(dims[1]);
  } else {
    fNDP = 0;
  }

  return fNDP;
}


herr_t H5FADCEvent::FlushBuffer()
{
  const std::size_t nEvtBuf = fEvtBuf.size();
  if (nEvtBuf == 0) {
    fBufEventCount = 0;
    fBufBytesUsed  = 0;
    return 0;
  }

  const std::size_t nChBuf = fChBuf.size();
  if (nChBuf * static_cast<std::size_t>(fNDP) != fWaveBuf.size()) {
    Error("FlushBuffer", "Internal buffer size mismatch");
    return -1;
  }

  herr_t status = 0;

  {
    hsize_t old_dim[1] = { fTotalEvents };
    hsize_t new_dim[1] = { fTotalEvents + nEvtBuf };
    H5Dset_extent(fDsetInfo, new_dim);

    hid_t file_space = H5Dget_space(fDsetInfo);

    hsize_t offset[1] = { old_dim[0] };
    hsize_t count[1]  = { nEvtBuf };

    H5Sselect_hyperslab(file_space, H5S_SELECT_SET,
                        offset, nullptr, count, nullptr);

    hid_t mem_space = H5Screate_simple(1, count, nullptr);

    status = H5Dwrite(fDsetInfo, fEvtType,
                      mem_space, file_space,
                      H5P_DEFAULT, fEvtBuf.data());

    H5Sclose(mem_space);
    H5Sclose(file_space);

    if (status < 0) {
      return status;
    }
  }

  {
    std::vector<std::uint64_t> indexBuf(nEvtBuf);
    std::uint64_t base = fTotalChannels;
    std::uint64_t running = 0;
    for (std::size_t i = 0; i < nEvtBuf; ++i) {
      indexBuf[i] = base + running;
      running += static_cast<std::uint64_t>(fEvtBuf[i].nhit);
    }

    hsize_t old_dim[1] = { fTotalEvents };
    hsize_t new_dim[1] = { fTotalEvents + nEvtBuf };
    H5Dset_extent(fDsetIndex, new_dim);

    hid_t file_space = H5Dget_space(fDsetIndex);

    hsize_t offset[1] = { old_dim[0] };
    hsize_t count[1]  = { nEvtBuf };

    H5Sselect_hyperslab(file_space, H5S_SELECT_SET,
                        offset, nullptr, count, nullptr);

    hid_t mem_space = H5Screate_simple(1, count, nullptr);

    status = H5Dwrite(fDsetIndex, H5T_NATIVE_ULLONG,
                      mem_space, file_space,
                      H5P_DEFAULT, indexBuf.data());

    H5Sclose(mem_space);
    H5Sclose(file_space);

    if (status < 0) {
      return status;
    }
  }

  if (nChBuf > 0) {
    hsize_t old_dim[1] = { fTotalChannels };
    hsize_t new_dim[1] = { fTotalChannels + nChBuf };
    H5Dset_extent(fDsetChs, new_dim);

    hid_t file_space = H5Dget_space(fDsetChs);

    hsize_t offset[1] = { old_dim[0] };
    hsize_t count[1]  = { nChBuf };

    H5Sselect_hyperslab(file_space, H5S_SELECT_SET,
                        offset, nullptr, count, nullptr);

    hid_t mem_space = H5Screate_simple(1, count, nullptr);

    status = H5Dwrite(fDsetChs, fChType,
                      mem_space, file_space,
                      H5P_DEFAULT, fChBuf.data());

    H5Sclose(mem_space);
    H5Sclose(file_space);

    if (status < 0) {
      return status;
    }

    hsize_t old_dims[2] = { fTotalChannels, static_cast<hsize_t>(fNDP) };
    hsize_t new_dims[2] = { fTotalChannels + nChBuf, static_cast<hsize_t>(fNDP) };
    H5Dset_extent(fDsetWave, new_dims);

    hid_t file_space_wave = H5Dget_space(fDsetWave);

    hsize_t offset_wave[2] = { old_dims[0], 0 };
    hsize_t count_wave[2]  = { nChBuf, static_cast<hsize_t>(fNDP) };

    H5Sselect_hyperslab(file_space_wave, H5S_SELECT_SET,
                        offset_wave, nullptr, count_wave, nullptr);

    hid_t mem_space_wave = H5Screate_simple(2, count_wave, nullptr);

    status = H5Dwrite(fDsetWave, H5T_NATIVE_USHORT,
                      mem_space_wave, file_space_wave,
                      H5P_DEFAULT, fWaveBuf.data());

    H5Sclose(mem_space_wave);
    H5Sclose(file_space_wave);

    if (status < 0) {
      return status;
    }
  }

  fTotalEvents   += static_cast<std::uint64_t>(nEvtBuf);
  fTotalChannels += static_cast<std::uint64_t>(nChBuf);

  fEvtBuf.clear();
  fChBuf.clear();
  fWaveBuf.clear();
  fBufEventCount = 0;
  fBufBytesUsed  = 0;

  return status;
}

void H5FADCEvent::Close()
{
  if (fWriteTag) {
    FlushBuffer();

    if (fDsetInfo  >= 0) {
      H5Dclose(fDsetInfo);
      fDsetInfo = H5I_INVALID_HID;
    }
    if (fDsetIndex >= 0) {
      H5Dclose(fDsetIndex);
      fDsetIndex = H5I_INVALID_HID;
    }
    if (fDsetChs   >= 0) {
      H5Dclose(fDsetChs);
      fDsetChs = H5I_INVALID_HID;
    }
    if (fDsetWave  >= 0) {
      H5Dclose(fDsetWave);
      fDsetWave = H5I_INVALID_HID;
    }
  }

  if (fEvtType >= 0) {
    H5Tclose(fEvtType);
    fEvtType = H5I_INVALID_HID;
  }
  if (fChType  >= 0) {
    H5Tclose(fChType);
    fChType = H5I_INVALID_HID;
  }
}

herr_t H5FADCEvent::AppendEvent(const EventInfo_t & info,
                                const std::vector<FChannel_t> & data)
{
  if (!fWriteTag) {
    return -1;
  }
  if (fNDP <= 0 || fNDP > kH5FADCNDPMAX) {
    Error("AppendEvent", "Invalid NDP: %d (max %d)", fNDP, kH5FADCNDPMAX);
    return -1;
  }

  const std::uint16_t nhit = static_cast<std::uint16_t>(data.size());

  EventInfo_t info_local = info;
  info_local.nhit = nhit;
  fEvtBuf.push_back(info_local);

  std::size_t addBytes = sizeof(EventInfo_t);
  addBytes += static_cast<std::size_t>(nhit) *
              (sizeof(FChannelHeader_t) +
               static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t));

  for (std::size_t i = 0; i < nhit; ++i) {
    FChannelHeader_t h{};
    h.id   = data[i].id;
    h.tbit = data[i].tbit;
    h.ped  = data[i].ped;
    fChBuf.push_back(h);

    const std::uint16_t * src = data[i].waveform;
    fWaveBuf.insert(fWaveBuf.end(),
                    src, src + static_cast<std::size_t>(fNDP));
  }

  if (fSubRun.nevent == 0) {
    fSubRun.first = info.tnum;
  }
  fSubRun.last   = info.tnum;
  fSubRun.nevent += 1;

  fMemSize += static_cast<hsize_t>(addBytes);

  fBufEventCount += 1;
  fBufBytesUsed  += addBytes;

  if ((fBufEventCap > 0 && fBufEventCount >= fBufEventCap) ||
      (fBufMaxBytes > 0 && fBufBytesUsed >= fBufMaxBytes)) {
    return FlushBuffer();
  }

  return 0;
}

herr_t H5FADCEvent::ReadEvent(int n)
{
  int evtno;
  hid_t fid = fChain && fChain->GetNFile() > 0
              ? fChain->GetFileId(n, evtno)
              : fFile;

  hid_t dset_info  = H5Dopen2(fid, "/events/info",  H5P_DEFAULT);
  hid_t dset_index = H5Dopen2(fid, "/events/index", H5P_DEFAULT);
  hid_t dset_chs   = H5Dopen2(fid, "/events/chs",   H5P_DEFAULT);
  hid_t dset_wave  = H5Dopen2(fid, "/events/wave",  H5P_DEFAULT);

  if (dset_info < 0 || dset_index < 0 || dset_chs < 0 || dset_wave < 0) {
    if (dset_info  >= 0) H5Dclose(dset_info);
    if (dset_index >= 0) H5Dclose(dset_index);
    if (dset_chs   >= 0) H5Dclose(dset_chs);
    if (dset_wave  >= 0) H5Dclose(dset_wave);
    return -1;
  }

  herr_t status = 0;

  hsize_t offset_evt[1] = { static_cast<hsize_t>(evtno) };
  hsize_t count_evt[1]  = { 1 };

  hid_t file_space_info = H5Dget_space(dset_info);
  H5Sselect_hyperslab(file_space_info, H5S_SELECT_SET,
                      offset_evt, nullptr, count_evt, nullptr);

  hid_t mem_space_info = H5Screate_simple(1, count_evt, nullptr);

  status = H5Dread(dset_info, fEvtType,
                   mem_space_info, file_space_info,
                   H5P_DEFAULT, &fEvtInfo);

  H5Sclose(mem_space_info);
  H5Sclose(file_space_info);

  if (status < 0) {
    H5Dclose(dset_info);
    H5Dclose(dset_index);
    H5Dclose(dset_chs);
    H5Dclose(dset_wave);
    return status;
  }

  std::uint64_t offset_value = 0;

  hid_t file_space_idx = H5Dget_space(dset_index);
  H5Sselect_hyperslab(file_space_idx, H5S_SELECT_SET,
                      offset_evt, nullptr, count_evt, nullptr);

  hid_t mem_space_idx = H5Screate_simple(1, count_evt, nullptr);

  status = H5Dread(dset_index, H5T_NATIVE_ULLONG,
                   mem_space_idx, file_space_idx,
                   H5P_DEFAULT, &offset_value);

  H5Sclose(mem_space_idx);
  H5Sclose(file_space_idx);

  if (status < 0) {
    H5Dclose(dset_info);
    H5Dclose(dset_index);
    H5Dclose(dset_chs);
    H5Dclose(dset_wave);
    return status;
  }

  const std::uint16_t nhit = fEvtInfo.nhit;

  if (fData) {
    delete[] fData;
    fData = nullptr;
  }
  fData = (nhit > 0) ? new FChannel_t[nhit] : nullptr;

  if (nhit > 0) {
    GetNDP();
    
    if (fNDP <= 0 || fNDP > kH5FADCNDPMAX) {
      Error("ReadEvent", "Invalid NDP: %d (max %d)", fNDP, kH5FADCNDPMAX);
      H5Dclose(dset_info);
      H5Dclose(dset_index);
      H5Dclose(dset_chs);
      H5Dclose(dset_wave);
      return -1;
    }

    std::vector<FChannelHeader_t> headers(nhit);

    hsize_t offset_ch[1] = { static_cast<hsize_t>(offset_value) };
    hsize_t count_ch[1]  = { static_cast<hsize_t>(nhit) };

    hid_t file_space_chs = H5Dget_space(dset_chs);
    H5Sselect_hyperslab(file_space_chs, H5S_SELECT_SET,
                        offset_ch, nullptr, count_ch, nullptr);

    hid_t mem_space_chs = H5Screate_simple(1, count_ch, nullptr);

    status = H5Dread(dset_chs, fChType,
                     mem_space_chs, file_space_chs,
                     H5P_DEFAULT, headers.data());

    H5Sclose(mem_space_chs);
    H5Sclose(file_space_chs);

    if (status < 0) {
      H5Dclose(dset_info);
      H5Dclose(dset_index);
      H5Dclose(dset_chs);
      H5Dclose(dset_wave);
      return status;
    }

    std::vector<std::uint16_t> wbuf(static_cast<std::size_t>(nhit) *
                                    static_cast<std::size_t>(fNDP));

    hsize_t offset_wave[2] = { static_cast<hsize_t>(offset_value), 0 };
    hsize_t count_wave[2]  = { static_cast<hsize_t>(nhit), static_cast<hsize_t>(fNDP) };

    hid_t file_space_wave = H5Dget_space(dset_wave);
    H5Sselect_hyperslab(file_space_wave, H5S_SELECT_SET,
                        offset_wave, nullptr, count_wave, nullptr);

    hid_t mem_space_wave = H5Screate_simple(2, count_wave, nullptr);

    status = H5Dread(dset_wave, H5T_NATIVE_USHORT,
                     mem_space_wave, file_space_wave,
                     H5P_DEFAULT, wbuf.data());

    H5Sclose(mem_space_wave);
    H5Sclose(file_space_wave);

    if (status < 0) {
      H5Dclose(dset_info);
      H5Dclose(dset_index);
      H5Dclose(dset_chs);
      H5Dclose(dset_wave);
      return status;
    }

    for (std::size_t ich = 0; ich < nhit; ++ich) {
      fData[ich].id   = headers[ich].id;
      fData[ich].tbit = headers[ich].tbit;
      fData[ich].ped  = headers[ich].ped;

      std::uint16_t * dst = fData[ich].waveform;
      const std::uint16_t * src =
        &wbuf[ich * static_cast<std::size_t>(fNDP)];
      std::memcpy(dst, src,
                  static_cast<std::size_t>(fNDP) * sizeof(std::uint16_t));
    }
  }

  H5Dclose(dset_info);
  H5Dclose(dset_index);
  H5Dclose(dset_chs);
  H5Dclose(dset_wave);

  return status;
}
