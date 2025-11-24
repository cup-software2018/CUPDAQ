#include "DAQConfig/FADCTConf.hh"
#include "DAQSystem/CupFADCT.hh"
#include "DAQUtils/ELog.hh"
#include "Notice/NoticeNKFADC500.hh"

ClassImp(CupFADCT)

CupFADCT::CupFADCT()
  : AbsADC()
{
}

CupFADCT::CupFADCT(int sid)
  : AbsADC(sid)
{
}

CupFADCT::CupFADCT(AbsConf * config)
  : AbsADC(config)
{
}

int CupFADCT::Open()
{
  int stat = NKFADC500open(fSID, nullptr);
  if (stat != 0) {
    ERROR("FADCT [sid=%d]: open failed, check connection and power", fSID);
    return stat;
  }
  INFO("FADCT [sid=%d]: opened", fSID);

  if (fConfig) {
    auto * config = static_cast<FADCTConf *>(fConfig);
    fEventDataSize = kNCHFADC * 128 * config->RL();
  }

  return stat;
}

void CupFADCT::Close()
{
  NKFADC500close(fSID);
  INFO("FADCT [sid=%d]: closed", fSID);
}

int CupFADCT::ReadBCount() { return NKFADC500read_BCOUNT(fSID); }

int CupFADCT::ReadData(int bcount, unsigned char * data)
{
  int state = NKFADC500read_DATA(fSID, bcount, data);
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

int CupFADCT::ReadData(int bcount)
{
  auto chunk = std::make_unique<ChunkData>(bcount);
  int state = NKFADC500read_DATA(fSID, bcount, chunk->data);
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

void CupFADCT::UpdateTriggerAndTime(const unsigned char * tempdata)
{
  unsigned int itmp;
  unsigned long ltmp;
  unsigned long finetime;
  unsigned long coarsetime;

  std::unique_lock<std::mutex> lock(fMutex);

  fCurrentTrgNumber = tempdata[68] & 0xFF;
  itmp = tempdata[72] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 8);
  itmp = tempdata[76] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 16);
  itmp = tempdata[80] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 24);
  fCurrentTrgNumber += 1;

  finetime = tempdata[100] & 0xFF;
  finetime = finetime * 8;

  ltmp = tempdata[104] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[108] & 0xFF) << 8;
  coarsetime += ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[112] & 0xFF) << 16;
  coarsetime += ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[116] & 0xFF) << 24;
  coarsetime += ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[120] & 0xFF) << 32;
  coarsetime += ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[124] & 0xFF) << 40;
  coarsetime += ltmp * 1000;

  fCurrentTime = coarsetime + finetime;
}
