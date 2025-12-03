#include "DAQConfig/IADCTConf.hh"
#include "DAQSystem/CupIADCT.hh"
#include "DAQUtils/ELog.hh"

ClassImp(CupIADCT)

CupIADCT::CupIADCT(int sid)
  : AbsADC(sid)
{
  fFADC.SetSID(sid);
}

CupIADCT::CupIADCT(AbsConf * config)
  : AbsADC(config)
{
  if (config) {
    fFADC.SetSID(config->SID());

    auto * conf = static_cast<IADCTConf *>(config);
    fMode = conf->MODE();
    if (fMode > 0) { fEventDataSize = 512 * conf->RL(); }
    else {
      fEventDataSize = kBYTESPEREVENTIADC;
    }
  }
}

int CupIADCT::Open()
{
  int stat = fFADC.Open();
  if (stat != 0) {
    ERROR("IADCT [sid=%d]: open failed, check connection and power", fSID);
    return stat;
  }

  INFO("IADCT [sid=%d]: opened", fSID);
  return stat;
}

void CupIADCT::Close()
{
  fFADC.Close();
  INFO("IADCT [sid=%d]: closed", fSID);
}

int CupIADCT::ReadBCount() { return fFADC.ReadBCount(); }

int CupIADCT::ReadData(int bcount, unsigned char * data)
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

int CupIADCT::ReadData(int bcount)
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

void CupIADCT::UpdateTriggerAndTime(const unsigned char * tempdata)
{
  unsigned long finetime = 0;
  unsigned long coarsetime = 0;

  std::unique_lock<std::mutex> lock(fMutex);

  // local trigger number
  fCurrentTrgNumber = static_cast<unsigned int>(tempdata[17] & 0xFFu);
  fCurrentTrgNumber |= static_cast<unsigned int>(tempdata[18] & 0xFFu) << 8;
  fCurrentTrgNumber |= static_cast<unsigned int>(tempdata[19] & 0xFFu) << 16;
  fCurrentTrgNumber |= static_cast<unsigned int>(tempdata[20] & 0xFFu) << 24;
  fCurrentTrgNumber += 1;

  // local starting fine time
  finetime = static_cast<unsigned long>(tempdata[25] & 0xFFu) * 8ul;

  // local starting coarse time
  coarsetime = static_cast<unsigned long>(tempdata[26] & 0xFFu);
  coarsetime |= static_cast<unsigned long>(tempdata[27] & 0xFFu) << 8;
  coarsetime |= static_cast<unsigned long>(tempdata[28] & 0xFFu) << 16;
  coarsetime |= static_cast<unsigned long>(tempdata[29] & 0xFFu) << 24;
  coarsetime |= static_cast<unsigned long>(tempdata[30] & 0xFFu) << 32;
  coarsetime |= static_cast<unsigned long>(tempdata[31] & 0xFFu) << 40;
  coarsetime *= 1000ul;

  fCurrentTime = coarsetime + finetime;
}
