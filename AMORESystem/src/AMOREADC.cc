#include "AMORESystem/AMOREADC.hh"

ClassImp(AMOREADC)

AMOREADC::AMOREADC()
  : AbsADC(),
    fMID(0),
    fTCB(AMORETCB::Instance())
{
}

AMOREADC::AMOREADC(int sid)
  : AbsADC(sid),
    fMID(sid + 128),
    fTCB(AMORETCB::Instance())
{
}

AMOREADC::AMOREADC(AbsConf * conf)
  : AbsADC(conf),
    fMID(conf->MID()),
    fTCB(AMORETCB::Instance())
{
}

int AMOREADC::ReadBCount() { return fTCB.ReadBCount(fMID); }

int AMOREADC::ReadData(int bcount, unsigned char * data)
{
  int state = fTCB.ReadData(fMID, bcount, data);
  if (state != 0) { return state; }

  fTotalBCount += bcount;
  UpdateCurrentTime(data, bcount);

  return state;
}

int AMOREADC::ReadData(int bcount)
{
  auto chunk = std::make_unique<ChunkData>(bcount);
  int state = fTCB.ReadData(fMID, bcount, chunk->data);
  if (state != 0) { return state; }

  fTotalBCount += bcount;
  UpdateCurrentTime(chunk->data, bcount);

  fChunkDataBuffer.push_back(std::move(chunk));

  return state;
}

void AMOREADC::UpdateCurrentTime(const unsigned char * data, int bcount)
{
  unsigned long coarsetime = 0;
  int j = kKILOBYTES * bcount - 64;

  std::unique_lock<std::mutex> lock(fMutex);

  coarsetime = static_cast<unsigned long>(data[j + 48] & 0xFFu);
  coarsetime |= static_cast<unsigned long>(data[j + 49] & 0xFFu) << 8;
  coarsetime |= static_cast<unsigned long>(data[j + 50] & 0xFFu) << 16;
  coarsetime |= static_cast<unsigned long>(data[j + 51] & 0xFFu) << 24;
  coarsetime |= static_cast<unsigned long>(data[j + 52] & 0xFFu) << 32;
  coarsetime |= static_cast<unsigned long>(data[j + 53] & 0xFFu) << 40;

  fCurrentTime = coarsetime * 1000ul;
}