#include "DAQSystem/CupGADCT.hh"
#include "DAQConfig/GADCTConf.hh"
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

CupGADCT::CupGADCT(AbsConf * config)
    : AbsADC(config)
{
}

CupGADCT::~CupGADCT() {}

int CupGADCT::Open()
{
  int stat = NKFADC125open(fSID, nullptr);
  if (stat != 0) {
    fLog->Error("CupGADCT::Open",
                "GADCT [sid=%d]: open falied, check connection and power",
                fSID);
    return stat;                
  }
  fLog->Info("CupGADCT::Open", "GADCT [sid=%d]: opened", fSID);

  if (fConfig) {
    auto * config = (GADCTConf *)fConfig;
    fEventDataSize = kNCHGADC * 32 * config->RL();
  }

  return stat;
}

void CupGADCT::Close()
{
  NKFADC125close(fSID);
  fLog->Info("CupGADCT::Close", "GADCT [sid=%d]: closed", fSID);
}

int CupGADCT::ReadBCount() { return NKFADC125read_BCOUNT(fSID); }

int CupGADCT::ReadData(int bcount, unsigned char * data)
{
  int state = NKFADC125read_DATA(fSID, bcount, data);

  fTotalBCount += bcount;

  if (fEventDataSize == 0) { return state; }

  int n = kKILOBYTES * bcount / fEventDataSize;
  unsigned char * tempdata = &(data[fEventDataSize * (n - 1)]);

  unsigned int itmp;
  unsigned long ltmp;

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
  fCurrentTime = tempdata[25] & 0xFF;
  fCurrentTime = fCurrentTime * 8;
  // get local starting coarse time
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

  return state;
}

int CupGADCT::ReadData(int bcount)
{
  auto * chunk = new ChunkData(bcount);
  int state = NKFADC125read_DATA(fSID, bcount, chunk->data);
  fChunkDataBuffer.push_back(chunk);

  fTotalBCount += bcount;

  if (fEventDataSize == 0) { return state; }

  unsigned char * data = chunk->data;
  int n = kKILOBYTES * bcount / fEventDataSize;
  unsigned char * tempdata = &(data[fEventDataSize * (n - 1)]);

  unsigned int itmp;
  unsigned long ltmp;

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
  fCurrentTime = tempdata[25] & 0xFF;
  fCurrentTime = fCurrentTime * 8;
  // get local starting coarse time
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

  return state;
}