#include <filesystem>

#include "HDF5Utils/H5DataWriter.hh"

ClassImp(H5DataWriter)

H5DataWriter::H5DataWriter()
  : TObject(),
    fFilename(),
    fFileId(H5I_INVALID_HID),
    fCompressionLevel(1),
    fData(nullptr),
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
    fData(nullptr),
    fFileSize(0),
    fMemorySize(0),
    fSubrun(0)
{
}

H5DataWriter::~H5DataWriter() { Close(); }

bool H5DataWriter::Open()
{
  if (!fData) {
    Error("Open", "no H5 Data (Event or Hit) connected");
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

  fData->SetWritable();
  fData->SetCompressionLevel(fCompressionLevel);
  fData->SetFileId(fFileId);
  fData->Open();

  return true;
}

void H5DataWriter::Close()
{
  if (fFileId < 0 || !fData) { return; }

  SubRun_t subrun{};
  subrun.subrun = static_cast<std::uint32_t>(fSubrun);

  // Utilize the new generalized AbsH5Base methods
  subrun.nevent = fData->GetSubRunEntries();

  unsigned int first_num = 0, last_num = 0;
  fData->GetSubRunNumbers(first_num, last_num);
  subrun.first = static_cast<std::uint32_t>(first_num);
  subrun.last = static_cast<std::uint32_t>(last_num);

  hid_t type = SubRun_t::BuildType();
  hsize_t onedim[1] = {1};
  hid_t space = H5Screate_simple(1, onedim, nullptr);

  hid_t dset = H5Dcreate2(fFileId, "subrun", type, space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  H5Dwrite(dset, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, &subrun);
  H5Dclose(dset);

  H5Sclose(space);
  H5Tclose(type);

  fData->Close();
  H5Fclose(fFileId);
  fFileId = H5I_INVALID_HID;
}

void H5DataWriter::PrintStats() const
{
  const double memsize = GetMemorySize() / 1024.0 / 1024.0;
  const double filesize = GetFileSize() / 1024.0 / 1024.0;
  const double ratio = (memsize > 0.0) ? (filesize / memsize * 100.0) : 0.0;

  std::string base;
  try {
    base = std::filesystem::path(fFilename).filename().string();
  }
  catch (...) {
    base = fFilename;
  }

  // Changed to use GetSubRunEntries so it reflects "Hits" or "Events" dynamically
  const int nentries = fData ? static_cast<int>(fData->GetSubRunEntries()) : 0;

  Info("PrintStats", "%d entries written in %s (%.2f | %.2f [MB], %.2f%%)", nentries, base.c_str(),
       memsize, filesize, ratio);
}