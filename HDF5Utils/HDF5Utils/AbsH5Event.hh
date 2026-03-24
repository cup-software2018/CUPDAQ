#pragma once

#include <cstdint>

#include "TObject.h"

#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5DataReader.hh"

#include "hdf5.h"

class AbsH5Event : public TObject {
public:
  AbsH5Event() = default;
  ~AbsH5Event() override = default;

  virtual void Open() = 0;
  virtual void Close() = 0;

  void SetFileId(hid_t fileid);
  void SetWritable();
  void SetCompressionLevel(int level);
  void SetSubRun(const SubRun_t & sub);
  void SetChainFile(H5ChainFile * chain);

  unsigned int GetNEvent() const;
  void GetEventNumbers(unsigned int & evti, unsigned int & evtf) const;
  virtual hsize_t GetSize() const;

  void SetBufferEventCapacity(int n);
  void SetBufferMaxBytes(std::size_t nbytes);
  EventInfo_t GetEventInfo() const;

protected:
  virtual herr_t FlushBuffer() = 0;

  hid_t fFile{H5I_INVALID_HID};
  hid_t fSpace{H5I_INVALID_HID};
  hid_t fEvtType{H5I_INVALID_HID};
  hid_t fChType{H5I_INVALID_HID};
  hid_t fSubType{H5I_INVALID_HID};
  hid_t fProp{H5I_INVALID_HID};
  hid_t fDProp{H5I_INVALID_HID};

  hid_t fDsetInfo{H5I_INVALID_HID};
  hid_t fDsetIndex{H5I_INVALID_HID};
  hid_t fDsetChs{H5I_INVALID_HID};

  EventInfo_t fEvtInfo{};
  SubRun_t fSubRun{};

  bool fWriteTag{false};
  int fCompressionLevel{1};
  hsize_t fMemSize{0};

  H5ChainFile * fChain{nullptr};

  int fBufEventCap{100};
  std::size_t fBufMaxBytes{32 * 1024 * 1024}; // maximum buffer size. 32 MB
  int fBufEventCount{0};
  std::size_t fBufBytesUsed{0};

  std::uint64_t fTotalEvents{0};

  std::vector<EventInfo_t> fEvtBuf;

  ClassDef(AbsH5Event, 0)
};

inline void AbsH5Event::SetFileId(hid_t fileid) { fFile = fileid; }

inline void AbsH5Event::SetWritable() { fWriteTag = true; }

inline void AbsH5Event::SetCompressionLevel(int level) { fCompressionLevel = level; }

inline void AbsH5Event::SetSubRun(const SubRun_t & sub) { fSubRun = sub; }

inline void AbsH5Event::SetChainFile(H5ChainFile * chain) { fChain = chain; }

inline unsigned int AbsH5Event::GetNEvent() const
{
  return static_cast<unsigned int>(fSubRun.nevent);
}

inline void AbsH5Event::GetEventNumbers(unsigned int & evti, unsigned int & evtf) const
{
  evti = static_cast<unsigned int>(fSubRun.first);
  evtf = static_cast<unsigned int>(fSubRun.last);
}

inline hsize_t AbsH5Event::GetSize() const { return fMemSize; }


inline void AbsH5Event::SetBufferEventCapacity(int n)
{
  if (n > 0) { fBufEventCap = n; }
}

inline void AbsH5Event::SetBufferMaxBytes(std::size_t nbytes)
{
  if (nbytes > 0) { fBufMaxBytes = nbytes; }
}

inline EventInfo_t AbsH5Event::GetEventInfo() const { return fEvtInfo; }