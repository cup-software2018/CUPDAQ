#include "DAQSystem/AbsADC.hh"

#include "TSystem.h"

ClassImp(AbsADC)

    AbsADC::AbsADC()
    : TObject()
{
  fSID = 0;
  fMID = 0;
  fConfig = nullptr;
  fTotalBCount = 0;
  fCurrentTime = 0;
  fCurrentTrgNumber = 0;
  fEventDataSize = 0;

  fLog = ELogger::Instance(true);
}

AbsADC::AbsADC(int sid)
    : TObject()
{
  fSID = sid;
  fMID = 0;
  fConfig = nullptr;
  fTotalBCount = 0;
  fCurrentTime = 0;
  fCurrentTrgNumber = 0;
  fEventDataSize = 0;

  fLog = ELogger::Instance(true);
}

AbsADC::AbsADC(AbsConf * config)
    : TObject()
{
  fConfig = config;
  fSID = fConfig->SID();
  fMID = fConfig->MID();
  fTotalBCount = 0;
  fCurrentTime = 0;
  fCurrentTrgNumber = 0;
  fEventDataSize = 0;

  fLog = ELogger::Instance(true);
}

AbsADC::~AbsADC()
{
  while (!fChunkDataBuffer.empty()) {
    ChunkData * data = fChunkDataBuffer.popfront();
    delete data;
  }
}

int AbsADC::Compare(const TObject * object) const
{
  auto * comp = (AbsADC *)object;

  if (this->GetSID() > comp->GetSID()) return 1;
  else if (this->GetSID() < comp->GetSID()) return -1;

  return 0;
}

void AbsADC::Bclear() { fChunkDataBuffer.clear(); }

void AbsADC::Bshrink_to_fit() { fChunkDataBuffer.shrink_to_fit(); }

bool AbsADC::Bempty() { return fChunkDataBuffer.empty(); }

int AbsADC::Bsize() { return fChunkDataBuffer.size(); }

ChunkData * AbsADC::Bpopfront(bool wait)
{
  return fChunkDataBuffer.popfront(wait);
}

ChunkData * AbsADC::Bfront(bool wait) { return fChunkDataBuffer.front(wait); }

void AbsADC::Bpop_front() { fChunkDataBuffer.pop_front(); }