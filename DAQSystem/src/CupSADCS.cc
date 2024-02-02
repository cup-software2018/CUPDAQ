#include "DAQConfig/SADCSConf.hh"
#include "DAQSystem/CupSADCS.hh"
#include "Notice/NoticeM64ADCS.hh"

ClassImp(CupSADCS) 

CupSADCS::CupSADCS()
    : AbsADC()
{
}

CupSADCS::CupSADCS(int sid)
    : AbsADC(sid)
{
}

CupSADCS::CupSADCS(AbsConf * conf)
    : AbsADC(conf)
{
}

CupSADCS::~CupSADCS() {}

int CupSADCS::Open()
{
  int stat = M64ADCSopen(fSID, nullptr);
  if (stat != 0) {
    fLog->Error("CupSADS::Open",
                "SADCS [sid=%d]: open falied, check connection and power",
                fSID);
    return stat;                
  }

  fLog->Info("CupSADS::Open", "SADCS [sid=%d]: opened", fSID);
  return stat;
}

void CupSADCS::Close()
{
  StopTrigger();
  Reset();

  M64ADCSclose(fSID);
  fLog->Info("CupSADS::Close", "SADCS [sid=%d]: closed", fSID);
}

int CupSADCS::ReadBCount() { return M64ADCSread_BCOUNT(fSID); }

int CupSADCS::ReadData(int bcount, unsigned char * data)
{
  int state = M64ADCSread_DATA(fSID, bcount, data);

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

int CupSADCS::ReadData(int bcount)
{
  auto * chunk = new ChunkData(bcount);
  int state = M64ADCSread_DATA(fSID, bcount, chunk->data);
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

bool CupSADCS::Configure()
{
  if (!fConfig) {
    fLog->Error("CupSADS::Configure", "SADCS [sid=%d]: no configuration", fSID);
    return false;
  }

  if (!TString(fConfig->GetName()).EqualTo("SADCS")) {
    fLog->Error("CupSADS::Configure",
                "SADCS [sid=%d]: configuration not matched with GADCS", fSID);
    return false;
  }

  Reset();
  M64ADCS_ADCALIGN_64(fSID);
  M64ADCSwrite_DRAMON(fSID, 1);
  gSystem->Sleep(10);

  unsigned long dramon = M64ADCSread_DRAMON(fSID);
  if (dramon) {
    fLog->Info("CupSADS::Configure", "SADCS [sid=%d]: DRAM on", fSID);
  }
  else {
    fLog->Error("CupSADS::Configure",
                "SADCS [sid=%d]: error occurred during turning DRAM on", fSID);
    return false;
  }

  auto * conf = (SADCSConf *)fConfig;
  conf->PrintConf();

  fSID = conf->SID();

  WritePTRIG(conf->PTRG());
  WriteMTHR(conf->MTHR());

  WriteCW(conf->CW());
  // WriteGW(conf->GW());
  WritePSW(1, conf->PSW());

  for (int i = 0; i < kNCHSADC; i++) {
    WriteTHR(i + 1, conf->THR(i));
    WriteDLY(i + 1, conf->DLY(i));
  }

  return true;
}

bool CupSADCS::Initialize() { return true; }

void CupSADCS::StartTrigger() { M64ADCSstart(fSID); }

void CupSADCS::StopTrigger() { M64ADCSstop(fSID); }

void CupSADCS::Reset() { M64ADCSreset(fSID); }

void CupSADCS::ResetTIMER() { M64ADCSresetTIMER(fSID); }

void CupSADCS::WriteCW(unsigned long data) { M64ADCSwrite_CW(fSID, data); }

unsigned long CupSADCS::ReadCW() { return M64ADCSread_CW(fSID); }

void CupSADCS::WriteDRAMON(unsigned long data)
{
  M64ADCSwrite_DRAMON(fSID, data);
}

unsigned long CupSADCS::ReadDRAMON() { return M64ADCSread_DRAMON(fSID); }

unsigned long CupSADCS::ReadPED(unsigned long ch)
{
  return M64ADCSread_PED(fSID, ch);
}

void CupSADCS::WriteDLY(unsigned long ch, unsigned long data)
{
  M64ADCSwrite_DLY(fSID, ch, data);
}

unsigned long CupSADCS::ReadDLY(unsigned long ch)
{
  return M64ADCSread_DLY(fSID, ch);
}

void CupSADCS::WriteTHR(unsigned long ch, unsigned long data)
{
  M64ADCSwrite_THR(fSID, ch, data);
}

unsigned long CupSADCS::ReadTHR(unsigned long ch)
{
  return M64ADCSread_THR(fSID, ch);
}

void CupSADCS::WritePSW(unsigned long ch, unsigned long data)
{
  M64ADCSwrite_PSW(fSID, ch, data);
}

unsigned long CupSADCS::ReadPSW(unsigned long ch)
{
  return M64ADCSread_PSW(fSID, ch);
}

void CupSADCS::WritePTRIG(unsigned long data)
{
  M64ADCSwrite_PTRIG(fSID, data);
}

unsigned long CupSADCS::ReadPTRIG() { return M64ADCSread_PTRIG(fSID); }

void CupSADCS::WriteTM(unsigned long data) { M64ADCSwrite_TM(fSID, data); }

unsigned long CupSADCS::ReadTM() { return M64ADCSread_TM(fSID); }
void CupSADCS::WriteMTHR(unsigned long data) { M64ADCSwrite_MTHR(fSID, data); }

unsigned long CupSADCS::ReadMTHR() { return M64ADCSread_MTHR(fSID); }

void CupSADCS::SendTRIG() { M64ADCSsend_TRIG(fSID); }

void CupSADCS::SendADCRST() { M64ADCSsend_ADCRST(fSID); }

void CupSADCS::SendADCCAL() { M64ADCSsend_ADCCAL(fSID); }

void CupSADCS::WriteADCDLY(unsigned long ch, unsigned long data)
{
  M64ADCSwrite_ADCDLY(fSID, ch, data);
}

void CupSADCS::WriteADCALIGN(unsigned long data)
{
  M64ADCSwrite_ADCALIGN(fSID, data);
}

unsigned long CupSADCS::ReadADCSTAT() { return M64ADCSread_ADCSTAT(fSID); }

void CupSADCS::WriteBITSLIP(unsigned long ch, unsigned long data)
{
  M64ADCSwrite_BITSLIP(fSID, ch, data);
}
void CupSADCS::WriteFMUX(unsigned long ch) { M64ADCSwrite_FMUX(fSID, ch); }

unsigned long CupSADCS::ReadFMUX() { return M64ADCSread_FMUX(fSID); }

void CupSADCS::ArmFADC() { M64ADCSarm_FADC(fSID); }

unsigned long CupSADCS::ReadFREADY() { return M64ADCSread_FREADY(fSID); }

void CupSADCS::ALIGNSADCS() { M64ADCS_ADCALIGN_64(fSID); }

void CupSADCS::ReadFADCBUF(unsigned long * data)
{
  M64ADCSread_FADCBUF(fSID, data);
}
