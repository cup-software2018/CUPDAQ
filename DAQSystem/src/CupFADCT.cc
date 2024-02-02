#include "DAQSystem/CupFADCT.hh"
#include "DAQConfig/FADCTConf.hh"
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

CupFADCT::~CupFADCT() {}

int CupFADCT::Open()
{
  int stat = NKFADC500open(fSID, nullptr);
  if (stat != 0) {
    fLog->Error("CupFADCT::Open",
                "FADCT [sid=%d]: open falied, check connection and power",
                fSID);
    return stat;                
  }
  fLog->Info("CupFADCT::Open", "FADCT [sid=%d]: opened", fSID);

  if (fConfig) {
    auto * config = (FADCTConf *)fConfig;
    fEventDataSize = kNCHFADC * 128 * config->RL();
  }

  return stat;
}

void CupFADCT::Close()
{
  NKFADC500close(fSID);
  fLog->Info("CupFADCT::Close", "FADCT [sid=%d]: closed", fSID);
}

int CupFADCT::ReadBCount() { return NKFADC500read_BCOUNT(fSID); }

int CupFADCT::ReadData(int bcount, unsigned char * data)
{
  int state = NKFADC500read_DATA(fSID, bcount, data);

  fTotalBCount += bcount;

  if (fEventDataSize == 0) { return state; }

  int n = kKILOBYTES * bcount / fEventDataSize;
  unsigned char * tempdata = &(data[fEventDataSize * (n - 1)]);

  unsigned int itmp;
  unsigned long ltmp, finetime, coarsetime;

  std::unique_lock<std::mutex> lock(fMutex);

  // get local trigger number
  fCurrentTrgNumber = tempdata[68] & 0xFF;
  itmp = tempdata[72] & 0xFF;
  fCurrentTrgNumber += (unsigned int)(itmp << 8);
  itmp = tempdata[76] & 0xFF;
  fCurrentTrgNumber += (unsigned int)(itmp << 16);
  itmp = tempdata[80] & 0xFF;
  fCurrentTrgNumber += (unsigned int)(itmp << 24);
  fCurrentTrgNumber += 1;

  // get loc starting fine time
  finetime = tempdata[100] & 0xFF;
  finetime = finetime * 8;
  // get loc starting coarse time
  ltmp = tempdata[104] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = (unsigned long)(tempdata[108] & 0xFF) << 8;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[112] & 0xFF) << 16;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[116] & 0xFF) << 24;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[120] & 0xFF) << 32;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[124] & 0xFF) << 40;
  coarsetime += ltmp * 1000;
  fCurrentTime = coarsetime + finetime;

  return state;
}

int CupFADCT::ReadData(int bcount)
{
  auto * chunk = new ChunkData(bcount);
  int state = NKFADC500read_DATA(fSID, bcount, chunk->data);
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
  fCurrentTrgNumber = tempdata[68] & 0xFF;
  itmp = tempdata[72] & 0xFF;
  fCurrentTrgNumber += (unsigned int)(itmp << 8);
  itmp = tempdata[76] & 0xFF;
  fCurrentTrgNumber += (unsigned int)(itmp << 16);
  itmp = tempdata[80] & 0xFF;
  fCurrentTrgNumber += (unsigned int)(itmp << 24);
  fCurrentTrgNumber += 1;

  // get loc starting fine time
  finetime = tempdata[100] & 0xFF;
  finetime = finetime * 8;
  // get loc starting coarse time
  ltmp = tempdata[104] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = (unsigned long)(tempdata[108] & 0xFF) << 8;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[112] & 0xFF) << 16;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[116] & 0xFF) << 24;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[120] & 0xFF) << 32;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(tempdata[124] & 0xFF) << 40;
  coarsetime += ltmp * 1000;
  fCurrentTime = coarsetime + finetime;

  return state;
}