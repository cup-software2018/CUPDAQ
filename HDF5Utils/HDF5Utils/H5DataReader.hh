#ifndef H5DataReader_hh
#define H5DataReader_hh

#include <vector>

#include "TObject.h"
#include "TString.h"
#include "hdf5.h"

#include "EDM.hh"

class H5ChainFile : public TObject {
public:
  H5ChainFile();
  ~H5ChainFile();

  void AddFile(DataFile_t * file);
  void Close();

  int GetNFile() const;
  hid_t GetFileId(int entno, int & evtno);

private:
  hid_t fCurrentFile;
  std::vector<DataFile_t *> fFiles;

  ClassDef(H5ChainFile, 0)
};

class AbsH5Event;
class H5DataReader : public TObject {
public:
  H5DataReader();
  H5DataReader(const char * fname);
  virtual ~H5DataReader();

  void SetFilename(const char * fname);
  bool Add(const char * fname);
  bool AddFile(const char * fname);
  void SetEvent(AbsH5Event * event);

  bool Open();
  void Close();

  int GetNEvent() const;
  int GetEntries() const;

private:
  H5ChainFile * fFiles;
  AbsH5Event * fEvent;
  int fNEvent;
  hid_t fSubType;

  ClassDef(H5DataReader, 0)
};

inline void H5DataReader::SetEvent(AbsH5Event * event) { fEvent = event; }

inline int H5DataReader::GetNEvent() const { return fNEvent; }

inline int H5DataReader::GetEntries() const { return fNEvent; }
#endif