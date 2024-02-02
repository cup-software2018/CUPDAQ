#ifndef H5DataWriter_hh
#define H5DataWriter_hh

#include <vector>

#include "TObject.h"
#include "TString.h"
#include "hdf5.h"

#include "HDF5Utils/AbsH5Event.hh"
#include "HDF5Utils/EDM.hh"

class H5DataWriter : public TObject {
public:
  H5DataWriter();
  H5DataWriter(const char * fname, int compress = 1);
  virtual ~H5DataWriter();

  bool Open();
  void Close();

  void SetFilename(const char * name);
  void SetCompressionLevel(int level);
  void SetEvent(AbsH5Event * event);
  void SetSubrun(int sub);

  hid_t GetFileId() const;
  hsize_t GetFileSize() const;
  hsize_t GetMemorySize() const;
  AbsH5Event * GetEvent();
  const char * GetFilename() const;
  bool IsOpen() const;

  void PrintStats() const;

private:
  TString fFilename;
  hid_t fFileId;
  int fCompressionLevel;
  AbsH5Event * fEvent;
  hsize_t fFileSize;
  hsize_t fMemorySize;
  int fSubrun;

  ClassDef(H5DataWriter, 0)
};

inline void H5DataWriter::SetFilename(const char * fname) { fFilename = fname; }

inline void H5DataWriter::SetCompressionLevel(int level)
{
  fCompressionLevel = level;
}

inline void H5DataWriter::SetEvent(AbsH5Event * event) { fEvent = event; }

inline void H5DataWriter::SetSubrun(int sub) { fSubrun = sub; }

inline hid_t H5DataWriter::GetFileId() const { return fFileId; }

inline hsize_t H5DataWriter::GetFileSize() const
{
  hsize_t size;
  H5Fget_filesize(fFileId, &size);
  return size;
}

inline hsize_t H5DataWriter::GetMemorySize() const { return fEvent->GetSize(); }

inline AbsH5Event * H5DataWriter::GetEvent() { return fEvent; }

inline const char * H5DataWriter::GetFilename() const
{
  return fFilename.Data();
}

inline bool H5DataWriter::IsOpen() const
{
  if (fFileId) return true;
  return false;
}

#endif