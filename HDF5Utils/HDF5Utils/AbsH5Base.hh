#pragma once

#include <cstddef>
#include <cstdint>

#include "TObject.h"

#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5DataReader.hh"

#include "hdf5.h"

class AbsH5Base : public TObject {
public:
  AbsH5Base() = default;
  ~AbsH5Base() override = default;

  virtual void Open() = 0;
  virtual void Close() = 0;

  void SetFileId(hid_t fileid);
  void SetWritable();
  void SetCompressionLevel(int level);
  void SetChainFile(H5ChainFile * chain);

  void SetSubRun(const SubRun_t & sub);
  unsigned int GetSubRunEntries() const;
  void GetSubRunNumbers(unsigned int & first, unsigned int & last) const;

  virtual hsize_t GetSize() const;

  void SetBufferCapacity(int n);
  void SetBufferMaxBytes(std::size_t nbytes);

protected:
  virtual herr_t FlushBuffer() = 0;

  // Helper methods for SubRun management
  void InitSubRun();
  void UpdateSubRun(std::uint32_t tnum);
  void CloseSubRun();

  hid_t fFile{H5I_INVALID_HID};
  hid_t fSpace{H5I_INVALID_HID};
  hid_t fProp{H5I_INVALID_HID};
  hid_t fDProp{H5I_INVALID_HID};
  hid_t fSubType{H5I_INVALID_HID};

  SubRun_t fSubRun{};

  bool fWriteTag{false};
  int fCompressionLevel{1};
  hsize_t fMemSize{0};

  H5ChainFile * fChain{nullptr};

  int fBufCap{100};
  std::size_t fBufMaxBytes{32 * 1024 * 1024};
  int fBufCount{0};
  std::size_t fBufBytesUsed{0};

  ClassDef(AbsH5Base, 0)
};

inline void AbsH5Base::SetFileId(hid_t fileid) { fFile = fileid; }

inline void AbsH5Base::SetWritable() { fWriteTag = true; }

inline void AbsH5Base::SetCompressionLevel(int level) { fCompressionLevel = level; }

inline void AbsH5Base::SetChainFile(H5ChainFile * chain) { fChain = chain; }

inline void AbsH5Base::SetSubRun(const SubRun_t & sub) { fSubRun = sub; }

inline unsigned int AbsH5Base::GetSubRunEntries() const
{
  return static_cast<unsigned int>(fSubRun.nevent);
}

inline void AbsH5Base::GetSubRunNumbers(unsigned int & first, unsigned int & last) const
{
  first = static_cast<unsigned int>(fSubRun.first);
  last = static_cast<unsigned int>(fSubRun.last);
}

inline hsize_t AbsH5Base::GetSize() const { return fMemSize; }

inline void AbsH5Base::SetBufferCapacity(int n)
{
  if (n > 0) { fBufCap = n; }
}

inline void AbsH5Base::SetBufferMaxBytes(std::size_t nbytes)
{
  if (nbytes > 0) { fBufMaxBytes = nbytes; }
}