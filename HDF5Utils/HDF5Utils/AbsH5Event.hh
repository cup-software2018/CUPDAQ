#ifndef AbsH5Event_hh
#define AbsH5Event_hh

#include "TObject.h"
#include "hdf5.h"

#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5DataReader.hh"

class AbsH5Event : public TObject {
public:
  AbsH5Event();
  virtual ~AbsH5Event();

  virtual void Open() = 0;
  virtual void Close() = 0;

  void SetFileId(hid_t fileid);
  void SetWritable();
  void SetCompressionLevel(int level);
  void SetSubRun(SubRun_t sub);
  void SetChainFile(H5ChainFile * chain);

  unsigned int GetNEvent() const;
  void GetEventNumbers(unsigned int & evti, unsigned int & evtf) const;

  virtual hsize_t GetSize() const;

protected:
  hid_t fFile;
  hid_t fSpace;
  hid_t fEvtType;
  hid_t fChType;
  hid_t fSubType;
  hid_t fProp;
  hid_t fDProp;

  EventInfo_t fEvtInfo;
  SubRun_t fSubRun;

  bool fWriteTag;
  int fCompressionLevel;
  hsize_t fMemSize;

  H5ChainFile * fChain;

  ClassDef(AbsH5Event, 0)
};

inline void AbsH5Event::SetFileId(hid_t fileid) { fFile = fileid; }

inline void AbsH5Event::SetWritable() { fWriteTag = true; }

inline void AbsH5Event::SetCompressionLevel(int level)
{
  fCompressionLevel = level;
}

inline void AbsH5Event::SetSubRun(SubRun_t sub)
{
  fSubRun.subrun = sub.subrun;
  fSubRun.nevent = sub.nevent;
  fSubRun.first = sub.first;
  fSubRun.last = sub.last;
}

inline void AbsH5Event::SetChainFile(H5ChainFile * chain) { fChain = chain; }

inline unsigned int AbsH5Event::GetNEvent() const { return fSubRun.nevent; }

inline void AbsH5Event::GetEventNumbers(unsigned int & evti,
                                        unsigned int & evtf) const
{
  evti = fSubRun.first;
  evtf = fSubRun.last;
}

inline hsize_t AbsH5Event::GetSize() const { return fMemSize; }

#endif