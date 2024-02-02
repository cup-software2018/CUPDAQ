#ifndef H5Event_hh
#define H5Event_hh

#include <iostream>
#include <vector>

#include "TString.h"
#include "hdf5.h"

#include "HDF5Utils/AbsH5Event.hh"
#include "HDF5Utils/EDM.hh"

template <typename T>
class H5Event : public AbsH5Event {
public:
  H5Event();
  virtual ~H5Event();

  virtual void Open();
  virtual void Close();

  herr_t WriteEvent(EventInfo_t info, std::vector<T> data);
  herr_t ReadEvent(int n);

  EventInfo_t GetEventInfo() const;
  T * GetData() const;

private:
  T * fData;

  ClassDef(H5Event, 0)
};

ClassImp(H5Event<FChannel_t>) 
ClassImp(H5Event<AChannel_t>)
ClassImp(H5Event<Crystal_t>)

template <typename T>
H5Event<T>::H5Event()
    : AbsH5Event()
{
  fData = nullptr;
}

template <typename T>
H5Event<T>::~H5Event()
{
  if (fData) delete[] fData;
}

template <typename T>
void H5Event<T>::Open()
{
  // build type
  fEvtType = fEvtInfo.BuildType();
  T temp;
  fChType = temp.BuildType();

  if (fWriteTag) {
    hsize_t onedim[1] = {1};
    fSpace = H5Screate_simple(1, onedim, NULL);

    fProp = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_deflate(fProp, fCompressionLevel);
    H5Pset_chunk(fProp, 1, onedim);

    fDProp = H5Pcreate(H5P_DATASET_CREATE);
    H5Pset_deflate(fDProp, fCompressionLevel);

    H5Tcommit2(fFile, "evttype", fEvtType, H5P_DEFAULT, H5P_DEFAULT,
               H5P_DEFAULT);
    H5Tcommit2(fFile, "chtype", fChType, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    fSubRun.nevent = 0;
    fSubRun.first = 0;
    fSubRun.last = 0;

    fMemSize = 0;
  }
}

template <typename T>
void H5Event<T>::Close()
{
  if (fWriteTag) {
    H5Sclose(fSpace);
    H5Pclose(fProp);
    H5Pclose(fDProp);
  }
  H5Tclose(fEvtType);
  H5Tclose(fChType);
}

template <typename T>
herr_t H5Event<T>::WriteEvent(EventInfo_t info, std::vector<T> data)
{
  hid_t groupid = H5Gcreate2(fFile, Form("event%d", info.tnum), H5P_DEFAULT,
                             H5P_DEFAULT, H5P_DEFAULT);
  herr_t status = 0;

  hid_t dset = H5Dcreate2(groupid, "eventinfo", fEvtType, fSpace, H5P_DEFAULT,
                          H5P_DEFAULT, H5P_DEFAULT);
  status = H5Dwrite(dset, fEvtType, H5S_ALL, H5S_ALL, H5P_DEFAULT, &info);
  H5Dclose(dset);

  fMemSize += info.GetSize();

  hsize_t onedim[1] = {data.size()};
  hid_t space = H5Screate_simple(1, onedim, NULL);

  H5Pset_chunk(fDProp, 1, onedim);

  dset = H5Dcreate2(groupid, "chs", fChType, space, H5P_DEFAULT, fDProp,
                    H5P_DEFAULT);
  status = H5Dwrite(dset, fChType, H5S_ALL, H5S_ALL, H5P_DEFAULT, data.data());
  H5Dclose(dset);

  H5Sclose(space);

  fMemSize += data.size() * data[0].GetSize();

  if (fSubRun.nevent == 0) { fSubRun.first = info.tnum; }
  fSubRun.last = info.tnum;
  fSubRun.nevent += 1;

  H5Gclose(groupid);
  return status;
}

template <typename T>
herr_t H5Event<T>::ReadEvent(int n)
{
  int evtno;
  hid_t fid = fChain->GetNFile() > 0 ? fChain->GetFileId(n, evtno) : fFile;
  hid_t group = H5Gopen2(fid, Form("/event%d", evtno), H5P_DEFAULT);

  // read event info
  hid_t dset = H5Dopen2(group, "eventinfo", H5P_DEFAULT);
  herr_t status =
      H5Dread(dset, fEvtType, H5S_ALL, H5S_ALL, H5P_DEFAULT, &fEvtInfo);
  H5Dclose(dset);

  // prepare read channel data
  int nhit = fEvtInfo.nhit;
  if (fData) { delete[] fData; }
  fData = new T[nhit];

  // read channel data
  dset = H5Dopen2(group, "chs", H5P_DEFAULT);
  status = H5Dread(dset, fChType, H5S_ALL, H5S_ALL, H5P_DEFAULT, fData);
  H5Dclose(dset);

  H5Gclose(group);
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

#endif