#pragma once

#include <string>
#include <vector>

#include "HDF5Utils/AbsH5Event.hh"
#include "HDF5Utils/EDM.hh"

#include "hdf5.h"

template <typename T>
class H5Event : public AbsH5Event {
public:
  H5Event();
  ~H5Event() override;

  void Open() override;
  void Close() override;

  herr_t AppendEvent(const EventInfo_t & info, const std::vector<T> & data);
  herr_t ReadEvent(int n);

  EventInfo_t GetEventInfo() const;
  T * GetData() const;

private:
  T * fData;

  hid_t fDsetInfo;
  hid_t fDsetChs;
  hid_t fDsetIndex;

  std::uint64_t fNEvents;
  std::uint64_t fNChannels;

  ClassDef(H5Event, 0)
};

ClassImp(H5Event<FChannel_t>)
ClassImp(H5Event<AChannel_t>)
ClassImp(H5Event<Crystal_t>)

template <typename T>
H5Event<T>::H5Event()
  : AbsH5Event(),
    fData(nullptr),
    fDsetInfo(H5I_INVALID_HID),
    fDsetChs(H5I_INVALID_HID),
    fDsetIndex(H5I_INVALID_HID),
    fNEvents(0),
    fNChannels(0)
{
}

template <typename T>
H5Event<T>::~H5Event()
{
  if (fData) {
    delete[] fData;
    fData = nullptr;
  }
}

template <typename T>
void H5Event<T>::Open()
{
  fEvtType = EventInfo_t::BuildType();
  fChType  = T::BuildType();

  if (fWriteTag) {
    hid_t grp_events = H5Gcreate2(fFile, "/events",
                                  H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (grp_events >= 0) {
      H5Gclose(grp_events);
    }

    hsize_t one[1] = {1};
    fSpace = H5Screate_simple(1, one, one);

    hsize_t zero[1]    = {0};
    hsize_t maxdims[1] = {H5S_UNLIMITED};
    hid_t space0 = H5Screate_simple(1, zero, maxdims);

    fProp = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk_evt[1] = {1024};
    H5Pset_chunk(fProp, 1, chunk_evt);
    H5Pset_deflate(fProp, fCompressionLevel);

    fDsetInfo = H5Dcreate2(fFile, "/events/info", fEvtType, space0,
                           H5P_DEFAULT, fProp, H5P_DEFAULT);

    fDProp = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk_chs[1] = {1024};
    H5Pset_chunk(fDProp, 1, chunk_chs);
    H5Pset_deflate(fDProp, fCompressionLevel);

    fDsetChs = H5Dcreate2(fFile, "/events/chs", fChType, space0,
                          H5P_DEFAULT, fDProp, H5P_DEFAULT);

    hid_t dcpl_idx = H5Pcreate(H5P_DATASET_CREATE);
    hsize_t chunk_idx[1] = {1024};
    H5Pset_chunk(dcpl_idx, 1, chunk_idx);
    H5Pset_deflate(dcpl_idx, fCompressionLevel);

    fDsetIndex = H5Dcreate2(fFile, "/events/index",
                            H5T_NATIVE_ULLONG, space0,
                            H5P_DEFAULT, dcpl_idx, H5P_DEFAULT);

    H5Pclose(dcpl_idx);
    H5Sclose(space0);

    H5Tcommit2(fFile, "evttype", fEvtType,
               H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Tcommit2(fFile, "chtype",  fChType,
               H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    fSubRun.nevent = 0;
    fSubRun.first  = 0;
    fSubRun.last   = 0;

    fMemSize    = 0;
    fNEvents    = 0;
    fNChannels  = 0;
  }
}

template <typename T>
void H5Event<T>::Close()
{
  if (fWriteTag) {
    if (fDsetInfo  >= 0) { H5Dclose(fDsetInfo);  fDsetInfo  = H5I_INVALID_HID; }
    if (fDsetChs   >= 0) { H5Dclose(fDsetChs);   fDsetChs   = H5I_INVALID_HID; }
    if (fDsetIndex >= 0) { H5Dclose(fDsetIndex); fDsetIndex = H5I_INVALID_HID; }

    H5Sclose(fSpace);
    H5Pclose(fProp);
    H5Pclose(fDProp);
  }

  H5Tclose(fEvtType);
  H5Tclose(fChType);
}

template <typename T>
herr_t H5Event<T>::AppendEvent(const EventInfo_t & info,
                               const std::vector<T> & data)
{
  if (!fWriteTag) {
    return -1;
  }

  const std::uint16_t nhit = static_cast<std::uint16_t>(data.size());
  herr_t status = 0;

  hsize_t new_evt_dim[1] = { fNEvents + 1 };
  H5Dset_extent(fDsetInfo, new_evt_dim);

  hid_t file_space_info = H5Dget_space(fDsetInfo);

  hsize_t offset_evt[1] = { fNEvents };
  hsize_t count_evt[1]  = { 1 };

  H5Sselect_hyperslab(file_space_info, H5S_SELECT_SET,
                      offset_evt, nullptr, count_evt, nullptr);

  EventInfo_t info_local = info;
  info_local.nhit = nhit;

  status = H5Dwrite(fDsetInfo, fEvtType,
                    fSpace, file_space_info,
                    H5P_DEFAULT, &info_local);

  H5Sclose(file_space_info);
  fMemSize += info.GetSize();

  if (status < 0) {
    return status;
  }

  hsize_t new_idx_dim[1] = { fNEvents + 1 };
  H5Dset_extent(fDsetIndex, new_idx_dim);

  hid_t file_space_idx = H5Dget_space(fDsetIndex);

  H5Sselect_hyperslab(file_space_idx, H5S_SELECT_SET,
                      offset_evt, nullptr, count_evt, nullptr);

  std::uint64_t offset_value = fNChannels;

  status = H5Dwrite(fDsetIndex, H5T_NATIVE_ULLONG,
                    fSpace, file_space_idx,
                    H5P_DEFAULT, &offset_value);

  H5Sclose(file_space_idx);

  if (status < 0) {
    return status;
  }

  if (nhit > 0) {
    hsize_t new_ch_dim[1] = { fNChannels + nhit };
    H5Dset_extent(fDsetChs, new_ch_dim);

    hid_t file_space_chs = H5Dget_space(fDsetChs);

    hsize_t offset_ch[1] = { fNChannels };
    hsize_t count_ch[1]  = { nhit };

    H5Sselect_hyperslab(file_space_chs, H5S_SELECT_SET,
                        offset_ch, nullptr, count_ch, nullptr);

    hid_t mem_space_chs = H5Screate_simple(1, count_ch, nullptr);

    status = H5Dwrite(fDsetChs, fChType,
                      mem_space_chs, file_space_chs,
                      H5P_DEFAULT, data.data());

    H5Sclose(mem_space_chs);
    H5Sclose(file_space_chs);

    if (status < 0) {
      return status;
    }

    if (!data.empty()) {
      fMemSize += static_cast<hsize_t>(nhit) * data[0].GetSize();
    }
  }

  if (fSubRun.nevent == 0) {
    fSubRun.first = info.tnum;
  }
  fSubRun.last  = info.tnum;
  fSubRun.nevent += 1;

  fNEvents   += 1;
  fNChannels += nhit;

  return status;
}

template <typename T>
herr_t H5Event<T>::ReadEvent(int n)
{
  int evtno;
  hid_t fid = fChain && fChain->GetNFile() > 0
              ? fChain->GetFileId(n, evtno)
              : fFile;

  hid_t dset_info  = H5Dopen2(fid, "/events/info",  H5P_DEFAULT);
  hid_t dset_index = H5Dopen2(fid, "/events/index", H5P_DEFAULT);
  hid_t dset_chs   = H5Dopen2(fid, "/events/chs",   H5P_DEFAULT);

  if (dset_info < 0 || dset_index < 0 || dset_chs < 0) {
    if (dset_info  >= 0) H5Dclose(dset_info);
    if (dset_index >= 0) H5Dclose(dset_index);
    if (dset_chs   >= 0) H5Dclose(dset_chs);
    return -1;
  }

  herr_t status = 0;

  hsize_t offset_evt[1] = { static_cast<hsize_t>(evtno) };
  hsize_t count_evt[1]  = { 1 };

  hid_t file_space_info = H5Dget_space(dset_info);
  H5Sselect_hyperslab(file_space_info, H5S_SELECT_SET,
                      offset_evt, nullptr, count_evt, nullptr);

  if (fSpace < 0) {
    hsize_t one[1] = {1};
    fSpace = H5Screate_simple(1, one, one);
  }

  status = H5Dread(dset_info, fEvtType,
                   fSpace, file_space_info,
                   H5P_DEFAULT, &fEvtInfo);

  H5Sclose(file_space_info);

  if (status < 0) {
    H5Dclose(dset_info);
    H5Dclose(dset_index);
    H5Dclose(dset_chs);
    return status;
  }

  std::uint64_t offset_value = 0;

  hid_t file_space_idx = H5Dget_space(dset_index);
  H5Sselect_hyperslab(file_space_idx, H5S_SELECT_SET,
                      offset_evt, nullptr, count_evt, nullptr);

  status = H5Dread(dset_index, H5T_NATIVE_ULLONG,
                   fSpace, file_space_idx,
                   H5P_DEFAULT, &offset_value);

  H5Sclose(file_space_idx);

  if (status < 0) {
    H5Dclose(dset_info);
    H5Dclose(dset_index);
    H5Dclose(dset_chs);
    return status;
  }

  const std::uint16_t nhit = fEvtInfo.nhit;

  if (fData) {
    delete[] fData;
    fData = nullptr;
  }
  fData = (nhit > 0) ? new T[nhit] : nullptr;

  if (nhit > 0) {
    hsize_t offset_ch[1] = { static_cast<hsize_t>(offset_value) };
    hsize_t count_ch[1]  = { static_cast<hsize_t>(nhit) };

    hid_t file_space_chs = H5Dget_space(dset_chs);
    H5Sselect_hyperslab(file_space_chs, H5S_SELECT_SET,
                        offset_ch, nullptr, count_ch, nullptr);

    hid_t mem_space_chs = H5Screate_simple(1, count_ch, nullptr);

    status = H5Dread(dset_chs, fChType,
                     mem_space_chs, file_space_chs,
                     H5P_DEFAULT, fData);

    H5Sclose(mem_space_chs);
    H5Sclose(file_space_chs);
  }

  H5Dclose(dset_info);
  H5Dclose(dset_index);
  H5Dclose(dset_chs);

  return status;
}

template <typename T>
EventInfo_t H5Event<T>::GetEventInfo() const
{
  return fEvtInfo;
}

template <typename T>
T * H5Event<T>::GetData() const
{
  return fData;
}
