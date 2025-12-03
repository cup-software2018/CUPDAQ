#include "DAQSystem/AmoreADC.hh"

ClassImp(AmoreADC)

AmoreADC::AmoreADC()
  : AbsADC(),
    fMID(0),
    fTCB(CupTCB::Instance())
{
}

AmoreADC::AmoreADC(int sid)
  : AbsADC(sid),
    fMID(sid + 128),
    fTCB(CupTCB::Instance())
{
}

AmoreADC::AmoreADC(AbsConf * conf)
  : AbsADC(conf),
    fMID(conf->MID()),
    fTCB(CupTCB::Instance())
{
}

int AmoreADC::ReadBCount() { return fTCB->ReadBCount(fMID); }

int AmoreADC::ReadData(int bcount, unsigned char * data)
{
  int state = fTCB->ReadData(fMID, bcount, data);

  fTotalBCount += bcount;

  UpdateCurrentTime(data, bcount);

  return state;
}

int AmoreADC::ReadData(int bcount)
{
  auto chunk = std::make_unique<ChunkData>(bcount);
  int state = fTCB->ReadData(fMID, bcount, chunk->data);

  fTotalBCount += bcount;

  UpdateCurrentTime(chunk->data, bcount);

  fChunkDataBuffer.push_back(std::move(chunk));

  return state;
}

void AmoreADC::UpdateCurrentTime(const unsigned char * data, int bcount)
{
  unsigned long ltmp;
  int j = kKILOBYTES * bcount - 64;

  std::unique_lock<std::mutex> lock(fMutex);
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
}