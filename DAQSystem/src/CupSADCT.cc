#include "DAQSystem/CupSADCT.hh"
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

CupSADCT::~CupSADCT() {}

int CupSADCT::Open()
{
  int stat = M64ADCopen(fSID, nullptr);
  if (stat != 0) {
    fLog->Error("CupSADCT::Open",
                "SADCT [sid=%d]: open falied, check connection and power",
                fSID);
    return stat;                
  }

  fLog->Info("CupSADCT::Open", "SADCT [sid=%d]: opened", fSID);
  return stat;
}

void CupSADCT::Close()
{
  M64ADCclose(fSID);
  fLog->Info("CupSADCT::Open", "SADCT [sid=%d]: closed", fSID);
}

int CupSADCT::ReadBCount() { return M64ADCread_BCOUNT(fSID); }

int CupSADCT::ReadData(int bcount, unsigned char * data)
{
  int state = M64ADCread_DATA(fSID, bcount, data);

  fTotalBCount += bcount;

  int n = 1024 * bcount / kBYTESPEREVENTSADC;
  unsigned char * tempdata = &(data[kBYTESPEREVENTSADC * (n - 1)]);

  unsigned int itmp;
  unsigned long ltmp, finetime, coarsetime;

  std::unique_lock<std::mutex> lock(fMutex);

  // get local trigger number
  fCurrentTrgNumber = tempdata[17] & 0xFF;
  itmp = tempdata[18] & 0xFF;
  fCurrentTrgNumber += (unsigned int)(itmp << 8);
  itmp = tempdata[19] & 0xFF;
  fCurrentTrgNumber += (unsigned int)(itmp << 16);
  itmp = tempdata[20] & 0xFF;
  fCurrentTrgNumber += (unsigned int)(itmp << 24);
  fCurrentTrgNumber += 1;

  // get local starting fine time
  finetime = tempdata[25] & 0xFF;
  finetime = finetime * 8;
  // get local starting coarse time
  ltmp = tempdata[26] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = (unsigned long)(tempdata[27] & 0xFF) << 8;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[28] & 0xFF) << 16;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[29] & 0xFF) << 24;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[30] & 0xFF) << 32;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[31] & 0xFF) << 40;
  coarsetime += ltmp * 1000;

  fCurrentTime = coarsetime + finetime;

  return state;
}

int CupSADCT::ReadData(int bcount)
{
  auto * chunk = new ChunkData(bcount);
  int state = M64ADCread_DATA(fSID, bcount, chunk->data);
  fChunkDataBuffer.push_back(chunk);

  fTotalBCount += bcount;

  int n = 1024 * bcount / kBYTESPEREVENTSADC;
  unsigned char * data = chunk->data;
  unsigned char * tempdata = &(data[kBYTESPEREVENTSADC * (n - 1)]);

  unsigned int itmp;
  unsigned long ltmp, finetime, coarsetime;

  std::unique_lock<std::mutex> lock(fMutex);

  // get local trigger number
  fCurrentTrgNumber = tempdata[17] & 0xFF;
  itmp = tempdata[18] & 0xFF;
  fCurrentTrgNumber += (unsigned int)(itmp << 8);
  itmp = tempdata[19] & 0xFF;
  fCurrentTrgNumber += (unsigned int)(itmp << 16);
  itmp = tempdata[20] & 0xFF;
  fCurrentTrgNumber += (unsigned int)(itmp << 24);
  fCurrentTrgNumber += 1;

  // get local starting fine time
  finetime = tempdata[25] & 0xFF;
  finetime = finetime * 8;
  // get local starting coarse time
  ltmp = tempdata[26] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = (unsigned long)(tempdata[27] & 0xFF) << 8;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[28] & 0xFF) << 16;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[29] & 0xFF) << 24;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[30] & 0xFF) << 32;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[31] & 0xFF) << 40;
  coarsetime += ltmp * 1000;

  fCurrentTime = coarsetime + finetime;

  return state;
}