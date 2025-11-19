#include "DAQConfig/IADCTConf.hh"
#include "DAQSystem/CupIADCT.hh"
#include "DAQUtils/ELog.hh"
#include "Notice/NoticeMUONDAQ.hh"

ClassImp(CupIADCT)

CupIADCT::CupIADCT()
  : AbsADC()
{
}

CupIADCT::CupIADCT(int sid)
  : AbsADC(sid)
{
}

CupIADCT::CupIADCT(AbsConf * config)
  : AbsADC(config)
{
  if (config) {
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
  int stat = MUONDAQopen(fSID, nullptr);
  if (stat != 0) {
    ERROR("IADCT [sid=%d]: open failed, check connection and power", fSID);
    return stat;
  }

  INFO("IADCT [sid=%d]: opened", fSID);
  return stat;
}

void CupIADCT::Close()
{
  MUONDAQclose(fSID);
  INFO("IADCT [sid=%d]: closed", fSID);
}

int CupIADCT::ReadBCount() { return MUONDAQread_BCOUNT(fSID); }

int CupIADCT::ReadData(int bcount, unsigned char * data)
{
  int state = MUONDAQread_DATA(fSID, bcount, data);
  if (state != 0) { return state; }

  fTotalBCount += static_cast<unsigned long>(bcount);

  if (fEventDataSize > 0) {
    int n = kKILOBYTES * bcount / fEventDataSize;
    unsigned char * tempdata = &(data[fEventDataSize * (n - 1)]);
    UpdateTriggerAndTime(tempdata);
  }

  return state;
}

int CupIADCT::ReadData(int bcount)
{
  auto chunk = std::make_unique<ChunkData>(bcount);
  int state = MUONDAQread_DATA(fSID, bcount, chunk->data);
  if (state != 0) { return state; }

  fTotalBCount += static_cast<unsigned long>(bcount);

  if (fEventDataSize > 0) {
    unsigned char * data = chunk->data;
    int n = kKILOBYTES * bcount / fEventDataSize;
    unsigned char * tempdata = &(data[fEventDataSize * (n - 1)]);
    UpdateTriggerAndTime(tempdata);
  }

  fChunkDataBuffer.push_back(std::move(chunk));

  return state;
}

void CupIADCT::UpdateTriggerAndTime(const unsigned char * tempdata)
{
  unsigned int itmp;
  unsigned long ltmp, finetime, coarsetime;

  std::unique_lock<std::mutex> lock(fMutex);

  // local trigger number
  fCurrentTrgNumber = tempdata[17] & 0xFF;
  itmp = tempdata[18] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 8);
  itmp = tempdata[19] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 16);
  itmp = tempdata[20] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 24);
  fCurrentTrgNumber += 1;

  // local starting fine time
  finetime = tempdata[25] & 0xFF;
  finetime *= 8;

  // local starting coarse time
  ltmp = tempdata[26] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[27] & 0xFF) << 8;
  coarsetime += ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[28] & 0xFF) << 16;
  coarsetime += ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[29] & 0xFF) << 24;
  coarsetime += ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[30] & 0xFF) << 32;
  coarsetime += ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[31] & 0xFF) << 40;
  coarsetime += ltmp * 1000;

  fCurrentTime = coarsetime + finetime;
}
