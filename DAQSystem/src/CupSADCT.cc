#include "DAQSystem/CupSADCT.hh"
#include "DAQUtils/ELog.hh"
#include "Notice/NoticeM64ADC.hh"

ClassImp(CupSADCT)

CupSADCT::CupSADCT()
  : AbsADC()
{
}

CupSADCT::CupSADCT(int sid)
  : AbsADC(sid)
{
}

CupSADCT::CupSADCT(AbsConf * conf)
  : AbsADC(conf)
{
}

int CupSADCT::Open()
{
  int stat = M64ADCopen(fSID, nullptr);
  if (stat != 0) {
    ERROR("SADCT [sid=%d]: open failed, check connection and power", fSID);
    return stat;
  }

  INFO("SADCT [sid=%d]: opened", fSID);
  return stat;
}

void CupSADCT::Close()
{
  M64ADCclose(fSID);
  INFO("SADCT [sid=%d]: closed", fSID);
}

int CupSADCT::ReadBCount() { return M64ADCread_BCOUNT(fSID); }

int CupSADCT::ReadData(int bcount, unsigned char * data)
{
  int state = M64ADCread_DATA(fSID, bcount, data);
  if (state != 0) { return state; }

  fTotalBCount += static_cast<unsigned long>(bcount);

  if (fEventDataSize > 0) {
    int n = kKILOBYTES * bcount / kBYTESPEREVENTSADC;
    unsigned char * tempdata = &(data[kBYTESPEREVENTSADC * (n - 1)]);

    UpdateTriggerAndTime(tempdata);
  }
  return state;
}

int CupSADCT::ReadData(int bcount)
{
  auto chunk = std::make_unique<ChunkData>(bcount);
  int state = M64ADCread_DATA(fSID, bcount, chunk->data);
  if (state != 0) { return state; }

  fTotalBCount += static_cast<unsigned long>(bcount);
  fChunkDataBuffer.push_back(std::move(chunk));

  if (fEventDataSize > 0) {
    unsigned char * data = chunk->data;
    int n = kKILOBYTES * bcount / kBYTESPEREVENTSADC;
    unsigned char * tempdata = &(data[kBYTESPEREVENTSADC * (n - 1)]);

    UpdateTriggerAndTime(tempdata);
  }

  return state;
}

void CupSADCT::UpdateTriggerAndTime(const unsigned char * tempdata)
{
  unsigned int itmp;
  unsigned long ltmp, finetime, coarsetime;

  std::unique_lock<std::mutex> lock(fMutex);

  fCurrentTrgNumber = tempdata[17] & 0xFF;
  itmp = tempdata[18] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 8);
  itmp = tempdata[19] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 16);
  itmp = tempdata[20] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 24);
  fCurrentTrgNumber += 1;

  finetime = tempdata[25] & 0xFF;
  finetime *= 8;

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
