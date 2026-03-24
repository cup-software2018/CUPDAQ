#include "DAQConfig/FADCTConf.hh"
#include "DAQSystem/CupFADCT.hh"
#include "DAQUtils/ELog.hh"

ClassImp(CupFADCT)

CupFADCT::CupFADCT(int sid)
  : AbsADC(sid)
{
  fFADC.SetSID(sid);
}

CupFADCT::CupFADCT(AbsConf * config)
  : AbsADC(config)
{
  fFADC.SetSID(config->SID());
}

int CupFADCT::Open()
{
  int stat = fFADC.Open();
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
  fFADC.Close();
  INFO("FADCT [sid=%d]: closed", fSID);
}

int CupFADCT::ReadBCount() { return fFADC.ReadBCount(); }

int CupFADCT::ReadData(int bcount, unsigned char * data)
{
  int state = fFADC.ReadData(bcount, data);
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
  int state = fFADC.ReadData(bcount, chunk->data);
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
  unsigned long finetime = 0;
  unsigned long coarsetime = 0;

  std::unique_lock<std::mutex> lock(fMutex);

  fCurrentTrgNumber = static_cast<unsigned int>(tempdata[68] & 0xFFu);
  fCurrentTrgNumber |= static_cast<unsigned int>(tempdata[72] & 0xFFu) << 8;
  fCurrentTrgNumber |= static_cast<unsigned int>(tempdata[76] & 0xFFu) << 16;
  fCurrentTrgNumber |= static_cast<unsigned int>(tempdata[80] & 0xFFu) << 24;
  fCurrentTrgNumber += 1;

  finetime = static_cast<unsigned long>(tempdata[100] & 0xFFu) * 8ul;

  coarsetime = static_cast<unsigned long>(tempdata[104] & 0xFFu);
  coarsetime |= static_cast<unsigned long>(tempdata[108] & 0xFFu) << 8;
  coarsetime |= static_cast<unsigned long>(tempdata[112] & 0xFFu) << 16;
  coarsetime |= static_cast<unsigned long>(tempdata[116] & 0xFFu) << 24;
  coarsetime |= static_cast<unsigned long>(tempdata[120] & 0xFFu) << 32;
  coarsetime |= static_cast<unsigned long>(tempdata[124] & 0xFFu) << 40;
  coarsetime *= 1000ul;

  fCurrentTime = coarsetime + finetime;
}
