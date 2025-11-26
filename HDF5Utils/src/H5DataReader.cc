#include <string>

#include "TList.h"
#include "TObjString.h"
#include "TRegexp.h"
#include "TString.h"
#include "TSystem.h"

#include "HDF5Utils/AbsH5Event.hh"
#include "HDF5Utils/H5DataReader.hh"

ClassImp(H5ChainFile)

H5ChainFile::H5ChainFile()
  : fCurrentFile(H5I_INVALID_HID),
    fFiles()
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
  for (auto * file : fFiles) {
    if (file->fid >= 0) {
      H5Fclose(file->fid);
      file->fid = -1;
    }
  }
}

int H5ChainFile::GetNFile() const { return static_cast<int>(fFiles.size()); }

hid_t H5ChainFile::GetFileId(int entno, int & evtno)
{
  hid_t fid = H5I_INVALID_HID;

  for (auto * file : fFiles) {
    auto search = file->entries.find(entno);
    if (search != file->entries.end()) {
      if (file->fid < 0) {
        if (fCurrentFile >= 0) { H5Fclose(fCurrentFile); }
        fid = H5Fopen(file->filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
        file->fid = fid;
        fCurrentFile = fid;
      }
      else {
        fid = file->fid;
      }
      evtno = search->second;
      break;
    }
  }

  return fid;
}

ClassImp(H5DataReader)

H5DataReader::H5DataReader()
  : fFiles(new H5ChainFile()),
    fEvent(nullptr),
    fNEvent(0),
    fSubType(SubRun_t::BuildType())
{
}

H5DataReader::H5DataReader(const char * fname)
  : fFiles(new H5ChainFile()),
    fEvent(nullptr),
    fNEvent(0),
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
  TString basename = fname;

  if (!basename.MaybeWildcard()) { return AddFile(fname); }

  Int_t slashpos = basename.Last('/');
  TString directory;
  if (slashpos >= 0) {
    directory = basename(0, slashpos);
    basename.Remove(0, slashpos + 1);
  }
  else {
    directory = gSystem->UnixPathName(gSystem->WorkingDirectory());
  }

  const char * file;
  const char * epath = gSystem->ExpandPathName(directory.Data());
  void * dir = gSystem->OpenDirectory(epath);
  delete[] epath;

  if (dir) {
    TList l;
    TRegexp re(basename, kTRUE);
    while ((file = gSystem->GetDirEntry(dir))) {
      if (!std::strcmp(file, ".") || !std::strcmp(file, "..")) { continue; }
      TString s = file;
      if ((basename != file) && s.Index(re) == kNPOS) { continue; }
      l.Add(new TObjString(file));
    }
    gSystem->FreeDirectory(dir);
    l.Sort();
    TIter next(&l);
    TObjString * obj;
    while ((obj = static_cast<TObjString *>(next()))) {
      file = obj->GetName();
      TString full = TString::Format("%s/%s", directory.Data(), file);
      AddFile(full.Data());
    }
    l.Delete();
  }

  return true;
}

bool H5DataReader::AddFile(const char * fname)
{
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
  herr_t err = H5Dread(did, fSubType, H5S_ALL, H5S_ALL, H5P_DEFAULT, &subrun);
  H5Dclose(did);

  if (err < 0) {
    Error("AddFile", "fail to read dataset[subrun]");
    H5Fclose(fid);
    delete file;
    return false;
  }

  const int nevt = static_cast<int>(subrun.nevent);
  int ient = fNEvent;
  int ievt = static_cast<int>(subrun.first);

  for (int i = 0; i < nevt; ++i) {
    file->entries.insert(std::pair{ient + i, ievt + i});
  }

  file->fid = -1;
  fFiles->AddFile(file);

  fNEvent += nevt;

  H5Fclose(fid);

  return true;
}

bool H5DataReader::Open()
{
  if (fFiles->GetNFile() == 0) {
    Error("Open", "no file connected");
    return false;
  }

  if (!fEvent) {
    Error("Open", "no H5Event connected");
    return false;
  }

  fEvent->SetChainFile(fFiles);
  fEvent->Open();

  return true;
}

void H5DataReader::Close()
{
  if (fEvent) { fEvent->Close(); }
}
