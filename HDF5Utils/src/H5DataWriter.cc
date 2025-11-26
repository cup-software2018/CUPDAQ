#include "TSystem.h"

#include "HDF5Utils/H5DataWriter.hh"

ClassImp(H5DataWriter)

H5DataWriter::H5DataWriter()
  : TObject(),
    fFilename(),
    fFileId(H5I_INVALID_HID),
    fCompressionLevel(1),
    fEvent(nullptr),
    fFileSize(0),
    fMemorySize(0),
    fSubrun(0)
{
}

H5DataWriter::H5DataWriter(const char * fname, int compress)
  : TObject(),
    fFilename(fname ? fname : ""),
    fFileId(H5I_INVALID_HID),
    fCompressionLevel(compress),
    fEvent(nullptr),
    fFileSize(0),
    fMemorySize(0),
    fSubrun(0)
{
}

H5DataWriter::~H5DataWriter() { Close(); }

bool H5DataWriter::Open()
{
  if (!fEvent) {
    Error("Open", "no H5Event connected");
    return false;
  }

  if (fFilename.empty()) {
    Error("Open", "filename is empty");
    return false;
  }

  if (fFileId >= 0) { Close(); }

  fFileId = H5Fcreate(fFilename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  if (fFileId < 0) {
    Error("Open", "fail to create data file %s", fFilename.c_str());
    return false;
  }

  fEvent->SetWritable();
  fEvent->SetCompressionLevel(fCompressionLevel);
  fEvent->SetFileId(fFileId);
  fEvent->Open();
  
  return true;
}

void H5DataWriter::Close()
{
  if (fFileId < 0 || !fEvent) { return; }

  SubRun_t subrun{};
  subrun.subrun = static_cast<std::uint32_t>(fSubrun);
  subrun.nevent = fEvent->GetNEvent();
  fEvent->GetEventNumbers(subrun.first, subrun.last);

  hid_t type = SubRun_t::BuildType();
  hsize_t onedim[1] = {1};
  hid_t space = H5Screate_simple(1, onedim, nullptr);

  hid_t dset = H5Dcreate2(fFileId, "subrun", type, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Dwrite(dset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, &subrun);
  H5Dclose(dset);

  H5Sclose(space);
  H5Tclose(type);

  fEvent->Close();
  H5Fclose(fFileId);
  fFileId = H5I_INVALID_HID;
}

void H5DataWriter::PrintStats() const
{
  const double memsize = GetMemorySize() / 1024.0 / 1024.0;
  const double filesize = GetFileSize() / 1024.0 / 1024.0;
  const double ratio = (memsize > 0.0) ? (filesize / memsize) : 0.0;

  const char * fname = gSystem->BaseName(fFilename.c_str());
  const int nevent = fEvent ? fEvent->GetNEvent() : 0;

  Info("PrintStats", "%d events written in %s (%.2f | %.2f [MB], %.2f%%)", nevent, fname, memsize, filesize, ratio);
}
