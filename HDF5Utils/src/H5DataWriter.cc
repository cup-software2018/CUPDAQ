#include <iostream>

#include "HDF5Utils/AbsH5Event.hh"
#include "HDF5Utils/H5DataWriter.hh"
#include "TSystem.h"

using namespace std;

ClassImp(H5DataWriter)

H5DataWriter::H5DataWriter()
    : TObject()
{
  fFileId = 0;

  fCompressionLevel = 1;
  fFileSize = 0;
  fMemorySize = 0;

  fEvent = nullptr;
}

H5DataWriter::H5DataWriter(const char * fname, int compress)
    : TObject()
{
  fFileId = 0;
  fFilename = fname;

  fCompressionLevel = compress;
  fFileSize = 0;
  fMemorySize = 0;

  fEvent = nullptr;
}

H5DataWriter::~H5DataWriter() {}

bool H5DataWriter::Open()
{
  if (!fEvent) {
    Error("Open", "no H5Event connected");
    return false;
  }

  fFileId = H5Fcreate(fFilename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  if (fFileId < 0) return false;

  fEvent->SetWritable();
  fEvent->SetCompressionLevel(fCompressionLevel);
  fEvent->SetFileId(fFileId);
  fEvent->Open();
  return true;
}

void H5DataWriter::Close()
{
  SubRun_t subrun;
  subrun.subrun = fSubrun;
  subrun.nevent = fEvent->GetNEvent();
  fEvent->GetEventNumbers(subrun.first, subrun.last);

  hid_t type = subrun.BuildType();
  hsize_t onedim[1] = {1};
  hid_t space = H5Screate_simple(1, onedim, NULL);

  hid_t dset = H5Dcreate2(fFileId, "subrun", type, space, H5P_DEFAULT,
                          H5P_DEFAULT, H5P_DEFAULT);
  H5Dwrite(dset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, &subrun);
  H5Dclose(dset);

  H5Sclose(space);
  H5Tclose(type);

  fEvent->Close();
  H5Fclose(fFileId);
}

void H5DataWriter::PrintStats() const
{
  double memsize = GetMemorySize() / 1024. / 1024.;
  double filesize = GetFileSize() / 1024. / 1024.;
  double ratio = filesize / memsize;

  const char * fname = gSystem->BaseName(fFilename.Data());
  int nevent = fEvent->GetNEvent();

  Info("PrintStats", "%d events written in %s (%.2f | %.2f [MB], %.2f%%)",
       nevent, fname, memsize, filesize, ratio);
}