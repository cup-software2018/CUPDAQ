#include <iostream>

#include "DAQConfig/MADCSConf.hh"
#include "DAQSystem/CupMADCS.hh"
#include "Notice/NoticeNKFADC125S.hh"

using namespace std;

ClassImp(CupMADCS) 

CupMADCS::CupMADCS()
    : AbsADC()
{
}

CupMADCS::CupMADCS(int sid)
    : AbsADC(sid)
{
}

CupMADCS::CupMADCS(AbsConf * config)
    : AbsADC(config)
{
}

CupMADCS::~CupMADCS() {}

int CupMADCS::Open()
{
  int stat = NKFADC125Sopen(fSID, nullptr);
  if (stat != 0) {
    fLog->Error("CupMADS::Open",
                "MADCS [sid=%d]: open falied, check connection and power",
                fSID);
    return stat;                
  }
  fLog->Info("CupMADS::Open", "MADCS [sid=%d]: opened", fSID);

  if (fConfig) {
    auto * config = (MADCSConf *)fConfig;
    fEventDataSize = kNCHMADC * 32 * config->RL();
  }

  return stat;
}

void CupMADCS::Close()
{
  StopTrigger();
  Reset();

  NKFADC125Sclose(fSID);
  fLog->Info("CupMADS::Close", "MADCS [sid=%d]: closed", fSID);
}

int CupMADCS::ReadBCount() { return NKFADC125Sread_BCOUNT(fSID); }

int CupMADCS::ReadData(int bcount, unsigned char * data)
{
  int state = NKFADC125Sread_DATA(fSID, bcount, data);

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

int CupMADCS::ReadData(int bcount)
{
  auto * chunk = new ChunkData(bcount);
  int state = NKFADC125Sread_DATA(fSID, bcount, chunk->data);
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

bool CupMADCS::Configure()
{
  if (!fConfig) {
    fLog->Error("CupMADS::Configure", "MADCS [sid=%d]: no configuration", fSID);
    return false;
  }

  if (!TString(fConfig->GetName()).EqualTo("MADCS")) {
    fLog->Error("CupMADS::Configure",
                "MADCS [sid=%d]: configuration not matched with MADCS", fSID);
    return false;
  }

  Reset();

  // set common registers
  NKFADC125S_ADCALIGN_125(fSID);
  NKFADC125Swrite_DRAMON(fSID, 1);
  gSystem->Sleep(10);

  unsigned long dramon = NKFADC125Sread_DRAMON(fSID);
  if (dramon) {
    fLog->Info("CupMADS::Configre", "MADCS [sid=%d]: DRAM on", fSID);
  }
  else {
    fLog->Error("CupMADS::Configre",
                "MADCS [sid=%d]: error occurred during turning DRAM on", fSID);
    return false;
  }

  NKFADC125S_ADCALIGN_DRAM(fSID);

  auto * conf = (MADCSConf *)fConfig;
  conf->PrintConf();

  Reset();
  WriteRL(conf->RL());
  WriteDSR(conf->DSR());
  WriteTLT(conf->TLT());
  WriteTRIGENABLE(conf->TRGON());
  WritePTRIG(conf->PTRG());
  WritePSCALE(conf->PSC());

  int nch = conf->NCH();
  for (int i = 0; i < nch; i++) {
    ULong_t cid = conf->CID(i);
    WritePOL(cid, conf->POL(i));
    WriteCW(cid, conf->CW(i));
    WriteTHR(cid, conf->THR(i));
    WriteDLY(cid, conf->DLY(i));
    WriteDT(cid, conf->DT(i));
    WriteTM(cid, conf->TM(i));
    WritePCT(cid, conf->PCT(i));
    WritePCI(cid, conf->PCI(i));
    WritePWT(cid, conf->PWT(i));
    WritePSW(cid, conf->PSW(i));
    WriteDACOFF(cid, conf->DACOFF(i));
  }

  unsigned long sid = conf->SID();
  unsigned long mid = conf->MID();
  unsigned long rRL = ReadRL();
  unsigned long rTLT = ReadTLT();
  unsigned long rDSR = ReadDSR();
  unsigned long rTRGON = ReadTRIGENABLE();
  unsigned long rPTRG = ReadPTRIG();
  unsigned long rPSC = ReadPSCALE();

  cout << Form(" ++ MADC register: SID(%lu) MID(%lu) NCH(%1d) RL(%lu) DSR(%lu) "
               "TLT(%lX) TRGON(%lu) PTRG(%lu) PSC(%lu)",
               sid, mid, nch, rRL, rDSR, rTLT, rTRGON, rPTRG, rPSC)
       << endl;

  cout << " -----------------------------------------------" << endl;
  cout << "        CID : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8d", conf->CID(i));
  }
  cout << endl;
  cout << "        POL : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8lu", ReadPOL(conf->CID(i)));
  }
  cout << endl;
  cout << "     DACOFF : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8lu", ReadDACOFF(conf->CID(i)));
  }
  cout << endl;
  cout << "        DLY : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8lu", ReadDLY(conf->CID(i)));
  }
  cout << endl;
  cout << "         CW : ";
  for (int i = 0; i < nch; i++)
    cout << Form("%8lu", ReadCW(conf->CID(i)));
  cout << endl;
  cout << "         TM : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8lu", ReadTM(conf->CID(i)));
  }
  cout << endl;
  cout << "        THR : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8lu", ReadTHR(conf->CID(i)));
  }
  cout << endl;
  cout << "        PCT : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8lu", ReadPCT(conf->CID(i)));
  }
  cout << endl;
  cout << "        PCI : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8lu", ReadPCI(conf->CID(i)));
  }
  cout << endl;
  cout << "        PWT : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8lu", ReadPWT(conf->CID(i)));
  }
  cout << endl;
  cout << "        PSW : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8lu", ReadPSW(conf->CID(i)));
  }
  cout << endl;
  cout << " -----------------------------------------------" << endl;
  cout << endl;

  fLog->Info("CupMADS::Configre", "MADCS [sid=%d]: configuration done", fSID);

  return true;
}

bool CupMADCS::Initialize()
{
  gSystem->Sleep(4000);

  auto * conf = (MADCSConf *)fConfig;
  int nch = conf->NCH();
  cout << "+++++++++++ MADC PEDESTALS ++++++++++++" << endl;
  cout << Form("  [sid=%2d]  ", fSID) << flush;
  for (int i = 0; i < nch; i++) {
    ULong_t cid = conf->CID(i);
    MeasurePED(cid);
    cout << Form("%4lu  ", ReadPED(cid)) << flush;
  }
  cout << endl;
  cout << "+++++++++++ MADC PEDESTALS ++++++++++++" << endl;

  fLog->Info("CupMADS::Initialize", "MADCS [sid=%d]: initialized", fSID);

  return true;
}

void CupMADCS::StartTrigger()
{
  Reset();
  ResetTIMER();

  NKFADC125Sstart(fSID);
  fLog->Info("CupMADS::StartTrigger", "MADCS [sid=%d]: trigger started", fSID);
}

void CupMADCS::StopTrigger()
{
  NKFADC125Sstop(fSID);
  Reset();

  fLog->Info("CupMADS::StopTrigger", "MADCS [sid=%d]: trigger stopped", fSID);
}

void CupMADCS::Reset() { NKFADC125Sreset(fSID); }

void CupMADCS::ResetTIMER() { NKFADC125SresetTIMER(fSID); }

void CupMADCS::WriteCW(unsigned long ch, unsigned long data)
{
  NKFADC125Swrite_CW(fSID, ch, data);
}
unsigned long CupMADCS::ReadCW(unsigned long ch)
{
  return NKFADC125Sread_CW(fSID, ch);
}
void CupMADCS::WriteRL(unsigned long data) { NKFADC125Swrite_RL(fSID, data); }
unsigned long CupMADCS::ReadRL() { return NKFADC125Sread_RL(fSID); }

void CupMADCS::WriteDACOFF(unsigned long ch, unsigned long data)
{
  NKFADC125Swrite_DACOFF(fSID, ch, data);
}
unsigned long CupMADCS::ReadDACOFF(unsigned long ch)
{
  return NKFADC125Sread_DACOFF(fSID, ch);
}
void CupMADCS::MeasurePED(unsigned long ch) { NKFADC125Smeasure_PED(fSID, ch); }
unsigned long CupMADCS::ReadPED(unsigned long ch)
{
  return NKFADC125Sread_PED(fSID, ch);
}
void CupMADCS::WriteDLY(unsigned long ch, unsigned long data)
{
  NKFADC125Swrite_DLY(fSID, ch, data);
}
unsigned long CupMADCS::ReadDLY(unsigned long ch)
{
  return NKFADC125Sread_DLY(fSID, ch);
}
void CupMADCS::WriteTHR(unsigned long ch, unsigned long data)
{
  NKFADC125Swrite_THR(fSID, ch, data);
}
unsigned long CupMADCS::ReadTHR(unsigned long ch)
{
  return NKFADC125Sread_THR(fSID, ch);
}
void CupMADCS::WritePOL(unsigned long ch, unsigned long data)
{
  NKFADC125Swrite_POL(fSID, ch, data);
}
unsigned long CupMADCS::ReadPOL(unsigned long ch)
{
  return NKFADC125Sread_POL(fSID, ch);
}
void CupMADCS::WritePSW(unsigned long ch, unsigned long data)
{
  NKFADC125Swrite_PSW(fSID, ch, data);
}
unsigned long CupMADCS::ReadPSW(unsigned long ch)
{
  return NKFADC125Sread_PSW(fSID, ch);
}
void CupMADCS::WritePCT(unsigned long ch, unsigned long data)
{
  NKFADC125Swrite_PCT(fSID, ch, data);
}
unsigned long CupMADCS::ReadPCT(unsigned long ch)
{
  return NKFADC125Sread_PCT(fSID, ch);
}
void CupMADCS::WritePCI(unsigned long ch, unsigned long data)
{
  NKFADC125Swrite_PCI(fSID, ch, data);
}
unsigned long CupMADCS::ReadPCI(unsigned long ch)
{
  return NKFADC125Sread_PCI(fSID, ch);
}
void CupMADCS::WritePWT(unsigned long ch, unsigned long data)
{
  NKFADC125Swrite_PWT(fSID, ch, data);
}
unsigned long CupMADCS::ReadPWT(unsigned long ch)
{
  return NKFADC125Sread_PWT(fSID, ch);
}
void CupMADCS::WriteDT(unsigned long ch, unsigned long data)
{
  NKFADC125Swrite_DT(fSID, ch, data);
}
unsigned long CupMADCS::ReadDT(unsigned long ch)
{
  return NKFADC125Sread_DT(fSID, ch);
}
void CupMADCS::WritePTRIG(unsigned long data)
{
  NKFADC125Swrite_PTRIG(fSID, data);
}
unsigned long CupMADCS::ReadPTRIG() { return NKFADC125Sread_PTRIG(fSID); }
void CupMADCS::SendTRIG() { NKFADC125Ssend_TRIG(fSID); }
void CupMADCS::WriteTRIGENABLE(unsigned long data)
{
  NKFADC125Swrite_TRIGENABLE(fSID, data);
}
unsigned long CupMADCS::ReadTRIGENABLE()
{
  return NKFADC125Sread_TRIGENABLE(fSID);
}
void CupMADCS::WriteTM(unsigned long ch, unsigned long data)
{
  NKFADC125Swrite_TM(fSID, ch, data);
}
unsigned long CupMADCS::ReadTM(unsigned long ch)
{
  return NKFADC125Sread_TM(fSID, ch);
}
void CupMADCS::WriteTLT(unsigned long data) { NKFADC125Swrite_TLT(fSID, data); }
unsigned long CupMADCS::ReadTLT() { return NKFADC125Sread_TLT(fSID); }
void CupMADCS::WritePSCALE(unsigned long data)
{
  NKFADC125Swrite_PSCALE(fSID, data);
}
unsigned long CupMADCS::ReadPSCALE() { return NKFADC125Sread_PSCALE(fSID); }
void CupMADCS::WriteDSR(unsigned long data) { NKFADC125Swrite_DSR(fSID, data); }
unsigned long CupMADCS::ReadDSR() { return NKFADC125Sread_DSR(fSID); }
