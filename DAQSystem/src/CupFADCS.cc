#include <iostream>

#include "TSystem.h"

#include "DAQConfig/FADCSConf.hh"
#include "DAQSystem/CupFADCS.hh"
#include "Notice/NoticeNKFADC500S.hh"

using namespace std;

ClassImp(CupFADCS)

    CupFADCS::CupFADCS()
    : AbsADC()
{
}

CupFADCS::CupFADCS(int sid)
    : AbsADC(sid)
{
}

CupFADCS::CupFADCS(AbsConf * config)
    : AbsADC(config)
{
}

CupFADCS::~CupFADCS() {}

int CupFADCS::Open()
{
  int stat = NKFADC500Sopen(fSID, nullptr);
  if (stat != 0) {
    fLog->Error("CupFADCS::Open",
                "FADCS [sid=%d]: open falied, check connection and power",
                fSID);
    return stat;
  }
  fLog->Info("CupFADCS::Open", "FADCS [sid=%d]: opened", fSID);

  if (fConfig) {
    auto * config = (FADCSConf *)fConfig;
    fEventDataSize = kNCHFADC * 128 * config->RL();
  }

  return stat;
}

void CupFADCS::Close()
{
  StopTrigger();
  Reset();

  NKFADC500Sclose(fSID);

  fLog->Info("CupFADCS::Close", "FADCS [sid=%d]: closed", fSID);
}

int CupFADCS::ReadBCount() { return NKFADC500Sread_BCOUNT(fSID); }

int CupFADCS::ReadData(int bcount, unsigned char * data)
{
  int state = NKFADC500Sread_DATA(fSID, bcount, data);

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

int CupFADCS::ReadData(int bcount)
{
  auto * chunk = new ChunkData(bcount);
  int state = NKFADC500Sread_DATA(fSID, bcount, chunk->data);
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

bool CupFADCS::Configure()
{
  if (!fConfig) {
    fLog->Error("CupFADCS::Configure", "FADCS [sid=%d]: no configuration",
                fSID);
    return false;
  }

  if (!TString(fConfig->GetName()).EqualTo("FADCS")) {
    fLog->Error("CupFADCS::Configure",
                "FADCS [sid=%d]: configuration not matched with FADCS", fSID);
    return false;
  }

  Reset();

  // set common registers
  NKFADC500S_ADCALIGN_500(fSID);
  NKFADC500Swrite_DRAMON(fSID, 1);
  gSystem->Sleep(10);

  unsigned long dramon = NKFADC500Sread_DRAMON(fSID);
  if (dramon) {
    fLog->Info("CupFADCS::Configure", "FADCS [sid=%d]: DRAM on", fSID);
  }
  else {
    fLog->Error("CupFADCS::Configure",
                "FADCS [sid=%d]: error occurred during turning DRAM on", fSID);
    return false;
  }

  NKFADC500S_ADCALIGN_DRAM(fSID);

  auto * conf = (FADCSConf *)fConfig;
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

    WriteAMODE(cid, conf->AMD(i));
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

  cout << Form(" ++ FADC register: SID(%lu) MID(%lu) NCH(%1d) RL(%lu) TLT(%lX) "
               "DSR(%lu) TRGON(%lu) PTRG(%lu) PSC(%lu)",
               sid, mid, nch, rRL, rTLT, rDSR, rTRGON, rPTRG, rPSC)
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
  cout << "      AMODE : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8lu", ReadAMODE(conf->CID(i)));
  }
  cout << endl;
  cout << "        DLY : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8lu", ReadDLY(conf->CID(i)));
  }
  cout << endl;
  cout << "      DTIME : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8lu", ReadDT(conf->CID(i)));
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

  fLog->Info("CupFADCS::Configure", "FADCS [sid=%d]: configuration done", fSID);

  return true;
}

bool CupFADCS::Initialize()
{
  gSystem->Sleep(4000);

  auto * conf = (FADCSConf *)fConfig;
  int nch = conf->NCH();
  cout << "+++++++++++ FADC PEDESTALS ++++++++++++" << endl;
  cout << Form("  [sid=%2d]  ", fSID) << flush;
  for (int i = 0; i < nch; i++) {
    ULong_t cid = conf->CID(i);
    MeasurePED(cid);
    cout << Form("%4lu  ", ReadPED(cid)) << flush;
  }
  cout << endl;
  cout << "+++++++++++ FADC PEDESTALS ++++++++++++" << endl;

  fLog->Info("CupFADCS::initialize", "FADCS [sid=%d]: initialized", fSID);

  return true;
}

void CupFADCS::StartTrigger()
{
  Reset();
  ResetTIMER();

  NKFADC500Sstart(fSID);
  fLog->Info("CupFADCS::StartTrigger", "FADCS [sid=%d]: trigger started", fSID);
}

void CupFADCS::StopTrigger()
{
  NKFADC500Sstop(fSID);
  Reset();

  fLog->Info("CupFADCS::StopTrigger", "CupFADCS [sid=%d]: trigger stopped",
             fSID);
}

void CupFADCS::Reset() { NKFADC500Sreset(fSID); }
void CupFADCS::ResetTIMER() { NKFADC500SresetTIMER(fSID); }

void CupFADCS::WriteCW(unsigned long ch, unsigned long data)
{
  NKFADC500Swrite_CW(fSID, ch, data);
}
unsigned long CupFADCS::ReadCW(unsigned long ch)
{
  return NKFADC500Sread_CW(fSID, ch);
}
void CupFADCS::WriteRL(unsigned long data) { NKFADC500Swrite_RL(fSID, data); }
unsigned long CupFADCS::ReadRL() { return NKFADC500Sread_RL(fSID); }

void CupFADCS::WriteDACOFF(unsigned long ch, unsigned long data)
{
  NKFADC500Swrite_DACOFF(fSID, ch, data);
}
unsigned long CupFADCS::ReadDACOFF(unsigned long ch)
{
  return NKFADC500Sread_DACOFF(fSID, ch);
}
void CupFADCS::MeasurePED(unsigned long ch) { NKFADC500Smeasure_PED(fSID, ch); }
unsigned long CupFADCS::ReadPED(unsigned long ch)
{
  return NKFADC500Sread_PED(fSID, ch);
}
void CupFADCS::WriteDLY(unsigned long ch, unsigned long data)
{
  NKFADC500Swrite_DLY(fSID, ch, data);
}
unsigned long CupFADCS::ReadDLY(unsigned long ch)
{
  return NKFADC500Sread_DLY(fSID, ch);
}
void CupFADCS::WriteTHR(unsigned long ch, unsigned long data)
{
  NKFADC500Swrite_THR(fSID, ch, data);
}
unsigned long CupFADCS::ReadTHR(unsigned long ch)
{
  return NKFADC500Sread_THR(fSID, ch);
}
void CupFADCS::WritePOL(unsigned long ch, unsigned long data)
{
  NKFADC500Swrite_POL(fSID, ch, data);
}
unsigned long CupFADCS::ReadPOL(unsigned long ch)
{
  return NKFADC500Sread_POL(fSID, ch);
}
void CupFADCS::WritePSW(unsigned long ch, unsigned long data)
{
  NKFADC500Swrite_PSW(fSID, ch, data);
}
unsigned long CupFADCS::ReadPSW(unsigned long ch)
{
  return NKFADC500Sread_PSW(fSID, ch);
}
void CupFADCS::WriteAMODE(unsigned long ch, unsigned long data)
{
  NKFADC500Swrite_AMODE(fSID, ch, data);
}
unsigned long CupFADCS::ReadAMODE(unsigned long ch)
{
  return NKFADC500Sread_AMODE(fSID, ch);
}
void CupFADCS::WritePCT(unsigned long ch, unsigned long data)
{
  NKFADC500Swrite_PCT(fSID, ch, data);
}
unsigned long CupFADCS::ReadPCT(unsigned long ch)
{
  return NKFADC500Sread_PCT(fSID, ch);
}
void CupFADCS::WritePCI(unsigned long ch, unsigned long data)
{
  NKFADC500Swrite_PCI(fSID, ch, data);
}
unsigned long CupFADCS::ReadPCI(unsigned long ch)
{
  return NKFADC500Sread_PCI(fSID, ch);
}
void CupFADCS::WritePWT(unsigned long ch, unsigned long data)
{
  NKFADC500Swrite_PWT(fSID, ch, data);
}
unsigned long CupFADCS::ReadPWT(unsigned long ch)
{
  return NKFADC500Sread_PWT(fSID, ch);
}
void CupFADCS::WriteDT(unsigned long ch, unsigned long data)
{
  NKFADC500Swrite_DT(fSID, ch, data);
}
unsigned long CupFADCS::ReadDT(unsigned long ch)
{
  return NKFADC500Sread_DT(fSID, ch);
}
void CupFADCS::WritePTRIG(unsigned long data)
{
  NKFADC500Swrite_PTRIG(fSID, data);
}
unsigned long CupFADCS::ReadPTRIG() { return NKFADC500Sread_PTRIG(fSID); }
void CupFADCS::SendTRIG() { NKFADC500Ssend_TRIG(fSID); }
void CupFADCS::WriteTRIGENABLE(unsigned long data)
{
  NKFADC500Swrite_TRIGENABLE(fSID, data);
}
unsigned long CupFADCS::ReadTRIGENABLE()
{
  return NKFADC500Sread_TRIGENABLE(fSID);
}
void CupFADCS::WriteTM(unsigned long ch, unsigned long data)
{
  NKFADC500Swrite_TM(fSID, ch, data);
}
unsigned long CupFADCS::ReadTM(unsigned long ch)
{
  return NKFADC500Sread_TM(fSID, ch);
}
void CupFADCS::WriteTLT(unsigned long data) { NKFADC500Swrite_TLT(fSID, data); }
unsigned long CupFADCS::ReadTLT() { return NKFADC500Sread_TLT(fSID); }
void CupFADCS::WritePSCALE(unsigned long data)
{
  NKFADC500Swrite_PSCALE(fSID, data);
}
unsigned long CupFADCS::ReadPSCALE() { return NKFADC500Sread_PSCALE(fSID); }
void CupFADCS::WriteDSR(unsigned long data) { NKFADC500Swrite_DSR(fSID, data); }
unsigned long CupFADCS::ReadDSR() { return NKFADC500Sread_DSR(fSID); }
