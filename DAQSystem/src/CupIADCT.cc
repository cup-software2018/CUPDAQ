#include "DAQSystem/CupIADCT.hh"
#include "DAQConfig/IADCTConf.hh"
#include "Notice/NoticeMUONDAQ.hh"

ClassImp(CupIADCT)

    CupIADCT::CupIADCT()
    : AbsADC()
{
  fMode = 0;
}

CupIADCT::CupIADCT(int sid)
    : AbsADC(sid)
{
  fMode = 0;
}

CupIADCT::CupIADCT(AbsConf * config)
    : AbsADC(config)
{
  if (config) {
    auto * conf = (IADCTConf *)config;
    fMode = conf->MODE();
    if (fMode > 0) fEventDataSize = 512 * conf->RL();
    else fEventDataSize = kBYTESPEREVENTIADC;
  }
}

CupIADCT::~CupIADCT() {}

int CupIADCT::Open()
{
  int stat = MUONDAQopen(fSID, nullptr);
  if (stat != 0) {
    fLog->Error("CupIADCT::Open",
                "IADCT [sid=%d]: open falied, check connection and power",
                fSID);
    return stat;                
  }
  fLog->Info("CupIADCT::Open", "IADCT [sid=%d]: opened", fSID);

  return stat;
}

void CupIADCT::Close()
{
  MUONDAQclose(fSID);
  fLog->Info("CupIADCT::Close", "IADCT [sid=%d]: closed", fSID);
}

int CupIADCT::ReadBCount() { return MUONDAQread_BCOUNT(fSID); }

int CupIADCT::ReadData(int bcount, unsigned char * data)
{
  int state = MUONDAQread_DATA(fSID, bcount, data);

  fTotalBCount += bcount;

  if (fEventDataSize == 0) { return state; }

  int n = kKILOBYTES * bcount / fEventDataSize;
  unsigned char * tempdata = &(data[fEventDataSize * (n - 1)]);

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

  // get loc starting fine time
  finetime = tempdata[25] & 0xFF;
  finetime = finetime * 8;
  // get loc starting coarse time
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

int CupIADCT::ReadData(int bcount)
{
  auto * chunk = new ChunkData(bcount);
  int state = MUONDAQread_DATA(fSID, bcount, chunk->data);
  fChunkDataBuffer.push_back(chunk);

  fTotalBCount += bcount;

  if (fEventDataSize == 0) { return state; }

  unsigned char * data = chunk->data;
  int n = kKILOBYTES * bcount / fEventDataSize;
  unsigned char * tempdata = &(data[fEventDataSize * (n - 1)]);

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

  // get loc starting fine time
  finetime = tempdata[25] & 0xFF;
  finetime = finetime * 8;
  // get loc starting coarse time
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