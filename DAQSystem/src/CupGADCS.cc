#include <iostream>

#include "TSystem.h"

#include "DAQConfig/GADCSConf.hh"
#include "DAQSystem/CupGADCS.hh"
#include "DAQUtils/ELog.hh"
#include "Notice/NoticeNKFADC125S.hh"

ClassImp(CupGADCS)

CupGADCS::CupGADCS()
  : AbsADC()
{
}

CupGADCS::CupGADCS(int sid)
  : AbsADC(sid)
{
}

CupGADCS::CupGADCS(AbsConf * config)
  : AbsADC(config)
{
}

int CupGADCS::Open()
{
  int stat = NKFADC125Sopen(fSID, nullptr);
  if (stat != 0) {
    ERROR("GADCS [sid=%d]: open failed, check connection and power", fSID);
    return stat;
  }
  INFO("GADCS [sid=%d]: opened", fSID);

  if (fConfig) {
    auto * config = static_cast<GADCSConf *>(fConfig);
    fEventDataSize = kNCHGADC * 32 * config->RL();
  }

  return stat;
}

void CupGADCS::Close()
{
  StopTrigger();
  Reset();

  NKFADC125Sclose(fSID);
  INFO("GADCS [sid=%d]: closed", fSID);
}

int CupGADCS::ReadBCount() { return NKFADC125Sread_BCOUNT(fSID); }

int CupGADCS::ReadData(int bcount, unsigned char * data)
{
  int state = NKFADC125Sread_DATA(fSID, bcount, data);
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

int CupGADCS::ReadData(int bcount)
{
  auto chunk = std::make_unique<ChunkData>(bcount);
  int state = NKFADC125Sread_DATA(fSID, bcount, chunk->data);
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

void CupGADCS::UpdateTriggerAndTime(const unsigned char * tempdata)
{
  unsigned int itmp;
  unsigned long ltmp;

  std::unique_lock<std::mutex> lock(fMutex);

  fCurrentTrgNumber = tempdata[17] & 0xFF;
  itmp = tempdata[18] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 8);
  itmp = tempdata[19] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 16);
  itmp = tempdata[20] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 24);
  fCurrentTrgNumber += 1;

  fCurrentTime = tempdata[25] & 0xFF;
  fCurrentTime = fCurrentTime * 8;

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
}

bool CupGADCS::Configure()
{
  if (!fConfig) {
    ERROR("GADCS [sid=%d]: no configuration", fSID);
    return false;
  }

  if (!TString(fConfig->GetName()).EqualTo("GADCS")) {
    ERROR("GADCS [sid=%d]: configuration not matched with GADCS", fSID);
    return false;
  }

  Reset();

  NKFADC125S_ADCALIGN_125(fSID);
  NKFADC125Swrite_DRAMON(fSID, 1);
  gSystem->Sleep(10);

  unsigned long dramon = NKFADC125Sread_DRAMON(fSID);
  if (dramon) { INFO("GADCS [sid=%d]: DRAM on", fSID); }
  else {
    ERROR("GADCS [sid=%d]: error occurred during turning DRAM on", fSID);
    return false;
  }

  NKFADC125S_ADCALIGN_DRAM(fSID);

  auto * conf = static_cast<GADCSConf *>(fConfig);
  conf->PrintConf();

  Reset();
  WriteRL(conf->RL());
  WriteDSR(conf->DSR());
  WriteTLT(conf->TLT());
  WriteTRIGENABLE(conf->TRGON());
  WritePTRIG(conf->PTRG());

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
  unsigned long rPSC = 0;

  std::cout << Form(" ++ GADC register: SID(%lu) MID(%lu) NCH(%1d) RL(%lu) "
                    "DSR(%lu) TLT(%lX) TRGON(%lu) PTRG(%lu) PSC(%lu)",
                    sid, mid, nch, rRL, rDSR, rTLT, rTRGON, rPTRG, rPSC)
            << std::endl;

  std::cout << " -----------------------------------------------" << std::endl;
  std::cout << "        CID : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8d", conf->CID(i));
  }
  std::cout << std::endl;
  std::cout << "        POL : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", ReadPOL(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "     DACOFF : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", ReadDACOFF(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "        DLY : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", ReadDLY(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "         CW : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", ReadCW(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "         TM : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", ReadTM(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "        THR : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", ReadTHR(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "        PCT : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", ReadPCT(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "        PCI : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", ReadPCI(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "        PWT : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", ReadPWT(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "        PSW : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", ReadPSW(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << " -----------------------------------------------" << std::endl;
  std::cout << std::endl;

  INFO("GADCS [sid=%d]: configuration done", fSID);

  return true;
}

bool CupGADCS::Initialize()
{
  gSystem->Sleep(4000);

  auto * conf = static_cast<GADCSConf *>(fConfig);
  int nch = conf->NCH();
  std::cout << "+++++++++++ GADC PEDESTALS ++++++++++++" << std::endl;
  std::cout << Form("  [sid=%2d]  ", fSID) << std::flush;
  for (int i = 0; i < nch; i++) {
    ULong_t cid = conf->CID(i);
    MeasurePED(cid);
    std::cout << Form("%4lu  ", ReadPED(cid)) << std::flush;
  }
  std::cout << std::endl;
  std::cout << "+++++++++++ GADC PEDESTALS ++++++++++++" << std::endl;

  INFO("GADCS [sid=%d]: initialized", fSID);

  return true;
}

void CupGADCS::StartTrigger()
{
  Reset();
  ResetTIMER();

  NKFADC125Sstart(fSID);
  INFO("GADCS [sid=%d]: trigger started", fSID);
}

void CupGADCS::StopTrigger()
{
  NKFADC125Sstop(fSID);
  Reset();

  INFO("GADCS [sid=%d]: trigger stopped", fSID);
}

void CupGADCS::Reset() { NKFADC125Sreset(fSID); }

void CupGADCS::ResetTIMER() { NKFADC125SresetTIMER(fSID); }

void CupGADCS::WriteCW(unsigned long ch, unsigned long data) { NKFADC125Swrite_CW(fSID, ch, data); }

unsigned long CupGADCS::ReadCW(unsigned long ch) { return NKFADC125Sread_CW(fSID, ch); }

void CupGADCS::WriteRL(unsigned long data) { NKFADC125Swrite_RL(fSID, data); }

unsigned long CupGADCS::ReadRL() { return NKFADC125Sread_RL(fSID); }

void CupGADCS::WriteDACOFF(unsigned long ch, unsigned long data) { NKFADC125Swrite_DACOFF(fSID, ch, data); }

unsigned long CupGADCS::ReadDACOFF(unsigned long ch) { return NKFADC125Sread_DACOFF(fSID, ch); }

void CupGADCS::MeasurePED(unsigned long ch) { NKFADC125Smeasure_PED(fSID, ch); }

unsigned long CupGADCS::ReadPED(unsigned long ch) { return NKFADC125Sread_PED(fSID, ch); }

void CupGADCS::WriteDLY(unsigned long ch, unsigned long data) { NKFADC125Swrite_DLY(fSID, ch, data); }

unsigned long CupGADCS::ReadDLY(unsigned long ch) { return NKFADC125Sread_DLY(fSID, ch); }

void CupGADCS::WriteTHR(unsigned long ch, unsigned long data) { NKFADC125Swrite_THR(fSID, ch, data); }

unsigned long CupGADCS::ReadTHR(unsigned long ch) { return NKFADC125Sread_THR(fSID, ch); }

void CupGADCS::WritePOL(unsigned long ch, unsigned long data) { NKFADC125Swrite_POL(fSID, ch, data); }

unsigned long CupGADCS::ReadPOL(unsigned long ch) { return NKFADC125Sread_POL(fSID, ch); }

void CupGADCS::WritePSW(unsigned long ch, unsigned long data) { NKFADC125Swrite_PSW(fSID, ch, data); }

unsigned long CupGADCS::ReadPSW(unsigned long ch) { return NKFADC125Sread_PSW(fSID, ch); }

void CupGADCS::WritePCT(unsigned long ch, unsigned long data) { NKFADC125Swrite_PCT(fSID, ch, data); }

unsigned long CupGADCS::ReadPCT(unsigned long ch) { return NKFADC125Sread_PCT(fSID, ch); }

void CupGADCS::WritePCI(unsigned long ch, unsigned long data) { NKFADC125Swrite_PCI(fSID, ch, data); }

unsigned long CupGADCS::ReadPCI(unsigned long ch) { return NKFADC125Sread_PCI(fSID, ch); }

void CupGADCS::WritePWT(unsigned long ch, unsigned long data) { NKFADC125Swrite_PWT(fSID, ch, data); }

unsigned long CupGADCS::ReadPWT(unsigned long ch) { return NKFADC125Sread_PWT(fSID, ch); }

void CupGADCS::WriteDT(unsigned long ch, unsigned long data) { NKFADC125Swrite_DT(fSID, ch, data); }

unsigned long CupGADCS::ReadDT(unsigned long ch) { return NKFADC125Sread_DT(fSID, ch); }

void CupGADCS::WritePTRIG(unsigned long data) { NKFADC125Swrite_PTRIG(fSID, data); }

unsigned long CupGADCS::ReadPTRIG() { return NKFADC125Sread_PTRIG(fSID); }

void CupGADCS::SendTRIG() { NKFADC125Ssend_TRIG(fSID); }

void CupGADCS::WriteTRIGENABLE(unsigned long data) { NKFADC125Swrite_TRIGENABLE(fSID, data); }

unsigned long CupGADCS::ReadTRIGENABLE() { return NKFADC125Sread_TRIGENABLE(fSID); }

void CupGADCS::WriteTM(unsigned long ch, unsigned long data) { NKFADC125Swrite_TM(fSID, ch, data); }

unsigned long CupGADCS::ReadTM(unsigned long ch) { return NKFADC125Sread_TM(fSID, ch); }

void CupGADCS::WriteTLT(unsigned long data) { NKFADC125Swrite_TLT(fSID, data); }

unsigned long CupGADCS::ReadTLT() { return NKFADC125Sread_TLT(fSID); }

// void CupGADCS::WriteZEROSUP(unsigned long ch, unsigned long data) { NKFADC125Swrite_ZEROSUP(fSID, ch, data); }
// unsigned long CupGADCS::ReadZEROSUP(unsigned long ch) { return NKFADC125Sread_ZEROSUP(fSID, ch); }

void CupGADCS::WriteDSR(unsigned long data) { NKFADC125Swrite_DSR(fSID, data); }

unsigned long CupGADCS::ReadDSR() { return NKFADC125Sread_DSR(fSID); }
