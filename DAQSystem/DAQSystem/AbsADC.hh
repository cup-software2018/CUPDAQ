#ifndef AbsADC_HH
#define AbsADC_HH

#include <mutex>

#include "TObject.h"

#include "DAQConfig/AbsConf.hh"
#include "DAQUtils/ConcurrentDeque.hh"
#include "DAQUtils/ELogger.hh"
#include "OnlConsts/adcconsts.hh"

class ChunkData {
public:
  ChunkData(int s)
  {
    size = s;
    data = new unsigned char[kKILOBYTES * size];
  }
  ~ChunkData() { delete[] data; }

  int size;
  unsigned char * data;
};

class AbsADC : public TObject {
public:
  AbsADC();
  AbsADC(int sid);
  AbsADC(AbsConf * config);
  virtual ~AbsADC();

  virtual void SetSID(int sid);
  virtual void SetMID(int mid);
  virtual void SetConfig(AbsConf * config);
  virtual void SetEventDataSize(int size);

  virtual int GetSID() const;
  virtual int GetMID() const;
  virtual AbsConf * GetConfig() const;

  virtual int Open() = 0;
  virtual void Close() = 0;

  virtual int ReadBCount() = 0;
  virtual int ReadData(int bcount, unsigned char * data) = 0;
  virtual int ReadData(int bcount) = 0;

  virtual bool Configure() = 0;
  virtual bool Initialize() = 0;
  virtual void StartTrigger() = 0;
  virtual void StopTrigger() = 0;

  virtual bool IsSortable() const;
  virtual int Compare(const TObject * object) const;
  virtual unsigned long GetTotalBCount() const;
  virtual unsigned long GetCurrentTime();
  virtual unsigned long GetCurrentTrgNumber();

  // for chunk data buffer operation
  virtual void Bclear();
  virtual void Bshrink_to_fit();
  virtual bool Bempty();
  virtual int Bsize();
  virtual ChunkData * Bpopfront(bool wait);
  virtual ChunkData * Bfront(bool wait);
  virtual void Bpop_front();

protected:
  int fSID;
  int fMID;

  ELogger * fLog;
  AbsConf * fConfig;
  ConcurrentDeque<ChunkData *> fChunkDataBuffer;

  unsigned long fTotalBCount;
  unsigned long fCurrentTime;
  unsigned long fCurrentTrgNumber;

  // mutex for current time
  std::mutex fMutex;

  int fEventDataSize;

  ClassDef(AbsADC, 0)
};

inline void AbsADC::SetSID(int sid) { fSID = sid; }

inline void AbsADC::SetMID(int mid) { fMID = mid; }

inline void AbsADC::SetConfig(AbsConf * config)
{
  fConfig = config;
  fSID = fConfig->SID();
  fMID = fConfig->MID();
}

inline void AbsADC::SetEventDataSize(int size) { fEventDataSize = size; }

inline int AbsADC::GetSID() const { return fSID; }

inline int AbsADC::GetMID() const { return fMID; }

inline AbsConf * AbsADC::GetConfig() const { return fConfig; }

inline bool AbsADC::IsSortable() const { return true; }

inline unsigned long AbsADC::GetTotalBCount() const { return fTotalBCount; }

inline unsigned long AbsADC::GetCurrentTime()
{
  std::lock_guard<std::mutex> lock(fMutex);
  return fCurrentTime;
}

inline unsigned long AbsADC::GetCurrentTrgNumber()
{
  std::lock_guard<std::mutex> lock(fMutex);
  return fCurrentTrgNumber;
}

#endif
