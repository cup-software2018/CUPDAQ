#include "DAQSystem/AmoreADC.hh"

ClassImp(AmoreADC)

AmoreADC::AmoreADC()
    : AbsADC()
{
  fTCB = CupTCB::Instance();
}

AmoreADC::AmoreADC(int sid)
    : AbsADC(sid)
{
  fMID = sid + 128;
  fTCB = CupTCB::Instance();
}

AmoreADC::AmoreADC(AbsConf * conf)
    : AbsADC(conf)
{
  fMID = conf->MID();
  fTCB = CupTCB::Instance();
}

AmoreADC::~AmoreADC() {}

int AmoreADC::ReadBCount() { return fTCB->ReadBCOUNT(fMID); }

int AmoreADC::ReadData(int bcount, unsigned char * data)
{
  int state = fTCB->ReadDATA(fMID, bcount, data);

  fTotalBCount += bcount;

  unsigned long ltmp;
  int j = kKILOBYTES * bcount - 64;

  std::unique_lock<std::mutex> lock(fMutex);
  // get local starting coarse time
  ltmp = data[j + 48] & 0xFF;
  fCurrentTime = ltmp * 1000;
  ltmp = data[j + 49] & 0xFF;
  ltmp = ltmp << 8;
  fCurrentTime = fCurrentTime + ltmp * 1000;
  ltmp = data[j + 50] & 0xFF;
  ltmp = ltmp << 16;
  fCurrentTime = fCurrentTime + ltmp * 1000;
  ltmp = data[j + 51] & 0xFF;
  ltmp = ltmp << 24;
  fCurrentTime = fCurrentTime + ltmp * 1000;
  ltmp = data[j + 52] & 0xFF;
  ltmp = ltmp << 32;
  fCurrentTime = fCurrentTime + ltmp * 1000;
  ltmp = data[j + 53] & 0xFF;
  ltmp = ltmp << 40;
  fCurrentTime = fCurrentTime + ltmp * 1000;
  lock.unlock();

  return state;
}

int AmoreADC::ReadData(int bcount)
{
  auto * chunk = new ChunkData(bcount);
  int state = fTCB->ReadDATA(fMID, bcount, chunk->data);
  fChunkDataBuffer.push_back(chunk);

  fTotalBCount += bcount;

  unsigned char * data = chunk->data;

  unsigned long ltmp;
  int j = kKILOBYTES * bcount - 64;

  std::unique_lock<std::mutex> lock(fMutex);
  // get local starting coarse time
  ltmp = data[j + 48] & 0xFF;
  fCurrentTime = ltmp * 1000;
  ltmp = data[j + 49] & 0xFF;
  ltmp = ltmp << 8;
  fCurrentTime = fCurrentTime + ltmp * 1000;
  ltmp = data[j + 50] & 0xFF;
  ltmp = ltmp << 16;
  fCurrentTime = fCurrentTime + ltmp * 1000;
  ltmp = data[j + 51] & 0xFF;
  ltmp = ltmp << 24;
  fCurrentTime = fCurrentTime + ltmp * 1000;
  ltmp = data[j + 52] & 0xFF;
  ltmp = ltmp << 32;
  fCurrentTime = fCurrentTime + ltmp * 1000;
  ltmp = data[j + 53] & 0xFF;
  ltmp = ltmp << 40;
  fCurrentTime = fCurrentTime + ltmp * 1000;
  lock.unlock();

  return state;
}
