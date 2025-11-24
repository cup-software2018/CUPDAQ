#include "DAQConfig/GADCTConf.hh"
#include "DAQSystem/CupGADCT.hh"
#include "DAQUtils/ELog.hh"
#include "Notice/NoticeNKFADC125.hh"

ClassImp(CupGADCT)

CupGADCT::CupGADCT()
  : AbsADC()
{
}

CupGADCT::CupGADCT(int sid)
  : AbsADC(sid)
{
}

CupGADCT::CupGADCT(AbsConf * conf)
  : AbsADC(conf)
{
}

int CupGADCT::Open()
{
  int stat = NKFADC125open(fSID, nullptr);
  if (stat != 0) {
    ERROR("GADCT [sid=%d]: open failed, check connection and power", fSID);
    return stat;
  }
  INFO("GADCT [sid=%d]: opened", fSID);

  if (fConfig) {
    auto * config = static_cast<GADCTConf *>(fConfig);
    fEventDataSize = kNCHGADC * 32 * config->RL();
  }

  return stat;
}

void CupGADCT::Close()
{
  NKFADC125close(fSID);
  INFO("GADCT [sid=%d]: closed", fSID);
}

int CupGADCT::ReadBCount() { return NKFADC125read_BCOUNT(fSID); }

int CupGADCT::ReadData(int bcount, unsigned char * data)
{
  int state = NKFADC125read_DATA(fSID, bcount, data);
  if (state != 0) { return state; }

  fTotalBCount += bcount;

  if (fEventDataSize > 0) {
    int n = kKILOBYTES * bcount / fEventDataSize;
    if (n > 0) {
      unsigned char * tempdata = &(data[fEventDataSize * (n - 1)]);
      UpdateTriggerAndTime(tempdata);
    }
  }

  return state;
}

int CupGADCT::ReadData(int bcount)
{
  auto chunk = std::make_unique<ChunkData>(bcount);
  int state = NKFADC125read_DATA(fSID, bcount, chunk->data);
  if (state != 0) { return state; }

  if (fEventDataSize > 0) {
    unsigned char * data = chunk->data;
    int n = kKILOBYTES * bcount / fEventDataSize;
    if (n > 0) {
      unsigned char * tempdata = &(data[fEventDataSize * (n - 1)]);
      UpdateTriggerAndTime(tempdata);
    }
  }

  fTotalBCount += static_cast<unsigned long>(bcount);
  fChunkDataBuffer.push_back(std::move(chunk));

  return state;
}

void CupGADCT::UpdateTriggerAndTime(const unsigned char * tempdata)
{
  unsigned int itmp;
  unsigned long ltmp;

  std::unique_lock<std::mutex> lock(fMutex);

  fCurrentTrgNumber = tempdata[17] & 0xFF;
  itmp = tempdata[18] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 8);
  itmp = tempdata[19] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 16);
  itmp = tempdata[20] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 24);
  fCurrentTrgNumber += 1;

  fCurrentTime = tempdata[25] & 0xFF;
  fCurrentTime = fCurrentTime * 8;

  ltmp = tempdata[26] & 0xFF;
  fCurrentTime += ltmp * 1000;
  ltmp = tempdata[27] & 0xFF;
  ltmp = ltmp << 8;
  fCurrentTime += ltmp * 1000;
  ltmp = tempdata[28] & 0xFF;
  ltmp = ltmp << 16;
  fCurrentTime += ltmp * 1000;
  ltmp = tempdata[29] & 0xFF;
  ltmp = ltmp << 24;
  fCurrentTime += ltmp * 1000;
  ltmp = tempdata[30] & 0xFF;
  ltmp = ltmp << 32;
  fCurrentTime += ltmp * 1000;
  ltmp = tempdata[31] & 0xFF;
  ltmp = ltmp << 40;
  fCurrentTime += ltmp * 1000;
}
