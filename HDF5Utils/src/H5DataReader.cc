#include <algorithm>
#include <cstring> // for std::strcmp
#include <filesystem>
#include <regex>
#include <string>
#include <vector>

#include "HDF5Utils/AbsH5Base.hh"
#include "HDF5Utils/H5DataReader.hh"

//------------------------------------------------------------------------------
// H5ChainFile
//------------------------------------------------------------------------------

ClassImp(H5ChainFile)

H5ChainFile::H5ChainFile()
  : fFiles()
{
}

H5ChainFile::~H5ChainFile()
{
  for (auto * file : fFiles) {
    delete file;
  }
}

void H5ChainFile::AddFile(DataFile_t * file) { fFiles.push_back(file); }

void H5ChainFile::Close()
{
  // Close all open HDF5 file handles
  for (auto * file : fFiles) {
    if (file->fid >= 0) {
      H5Fclose(file->fid);
      file->fid = -1;
    }
  }
}

int H5ChainFile::GetNFile() const { return static_cast<int>(fFiles.size()); }

hid_t H5ChainFile::GetFileId(int entno, int & local_entry, bool * file_changed)
{
  hid_t fid = H5I_INVALID_HID;
  if (file_changed) { *file_changed = false; }

  for (auto * file : fFiles) {
    // Range check instead of heavy std::map lookup
    if (entno >= file->global_start && entno < file->global_start + file->nevent) {

      if (fCurrentFilePtr != file) {
        if (file_changed) { *file_changed = true; }

        // Close the previous file if it was open
        if (fCurrentFilePtr && fCurrentFilePtr->fid >= 0) {
          H5Fclose(fCurrentFilePtr->fid);
          fCurrentFilePtr->fid = -1;
        }

        // Open the new file
        file->fid = H5Fopen(file->filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
        fCurrentFilePtr = file;
      }

      fid = file->fid;
      // Calculate local offset linearly (can be Event index or Hit index)
      local_entry = entno - file->global_start;
      break;
    }
  }

  return fid;
}

//------------------------------------------------------------------------------
// H5DataReader
//------------------------------------------------------------------------------

ClassImp(H5DataReader)

H5DataReader::H5DataReader()
  : fFiles(new H5ChainFile()),
    fData(nullptr),
    fEntries(0),
    fSubType(SubRun_t::BuildType())
{
}

H5DataReader::H5DataReader(const char * fname)
  : fFiles(new H5ChainFile()),
    fData(nullptr),
    fEntries(0),
    fSubType(SubRun_t::BuildType())
{
  SetFilename(fname);
}

H5DataReader::~H5DataReader()
{
  if (fSubType >= 0) { H5Tclose(fSubType); }
  delete fFiles;
}

void H5DataReader::SetFilename(const char * fname) { AddFile(fname); }

bool H5DataReader::Add(const char * fname)
{
  if (!fname || fname[0] == '\0') { return false; }

  std::string path_str = fname;

  if (path_str.find('*') == std::string::npos && path_str.find('?') == std::string::npos) {
    return AddFile(fname);
  }

  namespace fs = std::filesystem;
  fs::path filepath(path_str);

  fs::path directory = filepath.parent_path();
  std::string pattern = filepath.filename().string();

  if (directory.empty()) { directory = fs::current_path(); }

  std::string regex_str = "^";
  for (char c : pattern) {
    if (c == '.') { regex_str += "\\."; }
    else if (c == '*') {
      regex_str += ".*";
    }
    else if (c == '?') {
      regex_str += ".";
    }
    else {
      regex_str += c;
    }
  }
  regex_str += "$";

  std::regex re(regex_str);
  std::vector<std::string> matched_files;

  if (fs::exists(directory) && fs::is_directory(directory)) {
    for (const auto & entry : fs::directory_iterator(directory)) {
      if (entry.is_regular_file()) {
        std::string filename = entry.path().filename().string();
        if (std::regex_match(filename, re)) { matched_files.push_back(entry.path().string()); }
      }
    }
  }
  else {
    Error("Add", "Directory does not exist: %s", directory.string().c_str());
    return false;
  }

  std::sort(matched_files.begin(), matched_files.end());

  bool success = true;
  for (const auto & file : matched_files) {
    if (!AddFile(file.c_str())) { success = false; }
  }

  return success;
}

bool H5DataReader::AddFile(const char * fname)
{
  // Open HDF5 file and register its subrun/event/hit mapping
  if (!fname || fname[0] == '\0') {
    Error("AddFile", "no file name; no files connected");
    return false;
  }

  hid_t fid = H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (fid < 0) {
    Error("AddFile", "fail to open data file %s", fname);
    return false;
  }

  auto * file = new DataFile_t;
  file->filename = std::string(fname);

  hsize_t fsize = 0;
  H5Fget_filesize(fid, &fsize);
  file->filesize = fsize;

  SubRun_t subrun{};
  hid_t did = H5Dopen2(fid, "subrun", H5P_DEFAULT);
  if (did < 0) {
    Error("AddFile", "fail to open dataset[subrun]");
    H5Fclose(fid);
    delete file;
    return false;
  }

  herr_t err = H5Dread(did, fSubType, H5S_ALL, H5S_ALL, H5P_DEFAULT, &subrun);
  H5Dclose(did);

  if (err < 0) {
    Error("AddFile", "fail to read dataset[subrun]");
    H5Fclose(fid);
    delete file;
    return false;
  }

  // nevent here represents the number of entries (either global events or individual hits)
  const int nentries = static_cast<int>(subrun.nevent);
  int ient = fEntries;

  file->global_start = ient;
  file->nevent = nentries; // Kept as nevent in DataFile_t but acts as entries

  file->fid = -1; // will be opened lazily in H5ChainFile::GetFileId
  fFiles->AddFile(file);

  fEntries += nentries;

  H5Fclose(fid);

  return true;
}

bool H5DataReader::Open()
{
  if (fFiles->GetNFile() == 0) {
    Error("Open", "no file connected");
    return false;
  }

  if (!fData) {
    Error("Open", "no H5 data object connected");
    return false;
  }

  // Connect chain information to the AbsH5Base instance
  fData->SetChainFile(fFiles);
  fData->Open();

  return true;
}

void H5DataReader::Close()
{
  if (fData) { fData->Close(); }
}