#include <chrono>
#include <iostream>
#include <thread>

#include "DAQConfig/FADCSConf.hh"
#include "DAQSystem/CupFADCS.hh"
#include "DAQUtils/ELog.hh"
#include "Notice/NoticeNKFADC500S.hh"

ClassImp(CupFADCS)

CupFADCS::CupFADCS(int sid)
  : AbsADC(sid)
{
  fFADC.SetSID(sid);
}

CupFADCS::CupFADCS(AbsConf * config)
  : AbsADC(config)
{
  fFADC.SetSID(config->SID());
}

int CupFADCS::Open()
{
  int stat = NKFADC500Sopen(fSID, nullptr);
  if (stat != 0) {
    ERROR("FADCS [sid=%d]: open failed, check connection and power", fSID);
    return stat;
  }
  INFO("FADCS [sid=%d]: opened", fSID);

  if (fConfig) {
    auto * config = static_cast<FADCSConf *>(fConfig);
    fEventDataSize = kNCHFADC * 128 * config->RL();
  }

  return stat;
}

void CupFADCS::Close()
{
  fFADC.Stop();
  fFADC.Reset();
  fFADC.Close();

  INFO("FADCS [sid=%d]: closed", fSID);
}

int CupFADCS::ReadBCount() { return fFADC.ReadBCount(); }

int CupFADCS::ReadData(int bcount, unsigned char * data)
{
  int state = fFADC.ReadData(bcount, data);
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

int CupFADCS::ReadData(int bcount)
{
  auto chunk = std::make_unique<ChunkData>(bcount);
  int state = fFADC.ReadData(bcount, chunk->data);
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

void CupFADCS::UpdateTriggerAndTime(const unsigned char * tempdata)
{
  unsigned int itmp;
  unsigned long ltmp;
  unsigned long finetime;
  unsigned long coarsetime;

  std::unique_lock<std::mutex> lock(fMutex);

  fCurrentTrgNumber = tempdata[68] & 0xFF;
  itmp = tempdata[72] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 8);
  itmp = tempdata[76] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 16);
  itmp = tempdata[80] & 0xFF;
  fCurrentTrgNumber += static_cast<unsigned int>(itmp << 24);
  fCurrentTrgNumber += 1;

  finetime = tempdata[100] & 0xFF;
  finetime = finetime * 8;

  ltmp = tempdata[104] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[108] & 0xFF) << 8;
  coarsetime += ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[112] & 0xFF) << 16;
  coarsetime += ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[116] & 0xFF) << 24;
  coarsetime += ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[120] & 0xFF) << 32;
  coarsetime += ltmp * 1000;
  ltmp = static_cast<unsigned long>(tempdata[124] & 0xFF) << 40;
  coarsetime += ltmp * 1000;

  fCurrentTime = coarsetime + finetime;
}

bool CupFADCS::Configure()
{
  if (!fConfig) {
    ERROR("FADCS [sid=%d]: no configuration", fSID);
    return false;
  }

  if (!TString(fConfig->GetName()).EqualTo("FADCS")) {
    ERROR("FADCS [sid=%d]: configuration not matched with FADCS", fSID);
    return false;
  }

  fFADC.Reset();

  fFADC.AlignADC();
  fFADC.WriteDRAMON(1);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  unsigned long dramon = fFADC.ReadDRAMON();
  if (dramon) { INFO("FADCS [sid=%d]: DRAM on", fSID); }
  else {
    ERROR("FADCS [sid=%d]: error occurred during turning DRAM on", fSID);
    return false;
  }

  fFADC.AlignDRAM();

  auto * conf = static_cast<FADCSConf *>(fConfig);
  conf->PrintConf();

  fFADC.Reset();
  fFADC.WriteRL(conf->RL());
  fFADC.WriteDSR(conf->DSR());
  fFADC.WriteTLT(conf->TLT());
  fFADC.WriteTRIGENABLE(conf->TRGON());
  fFADC.WritePTRIG(conf->PTRG());
  fFADC.WritePSCALE(conf->PSC());

  int nch = conf->NCH();
  for (int i = 0; i < nch; i++) {
    ULong_t cid = conf->CID(i);

    fFADC.WriteAMODE(cid, conf->AMD(i));
    fFADC.WritePOL(cid, conf->POL(i));
    fFADC.WriteCW(cid, conf->CW(i));
    fFADC.WriteTHR(cid, conf->THR(i));
    fFADC.WriteDLY(cid, conf->DLY(i));
    fFADC.WriteDT(cid, conf->DT(i));
    fFADC.WriteTM(cid, conf->TM(i));
    fFADC.WritePCT(cid, conf->PCT(i));
    fFADC.WritePCI(cid, conf->PCI(i));
    fFADC.WritePWT(cid, conf->PWT(i));
    fFADC.WritePSW(cid, conf->PSW(i));
    fFADC.WriteDACOFF(cid, conf->DACOFF(i));
  }

  unsigned long sid = conf->SID();
  unsigned long mid = conf->MID();
  unsigned long rRL = fFADC.ReadRL();
  unsigned long rTLT = fFADC.ReadTLT();
  unsigned long rDSR = fFADC.ReadDSR();
  unsigned long rTRGON = fFADC.ReadTRIGENABLE();
  unsigned long rPTRG = fFADC.ReadPTRIG();
  unsigned long rPSC = fFADC.ReadPSCALE();

  std::cout << Form(" ++ FADC register: SID(%lu) MID(%lu) NCH(%1d) RL(%lu) TLT(%lX) "
                    "DSR(%lu) TRGON(%lu) PTRG(%lu) PSC(%lu)",
                    sid, mid, nch, rRL, rTLT, rDSR, rTRGON, rPTRG, rPSC)
            << std::endl;

  std::cout << " -----------------------------------------------" << std::endl;
  std::cout << "        CID : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8d", conf->CID(i));
  }
  std::cout << std::endl;
  std::cout << "        POL : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadPOL(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "     DACOFF : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadDACOFF(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "      AMODE : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadAMODE(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "        DLY : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadDLY(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "      DTIME : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadDT(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "         CW : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadCW(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "         TM : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadTM(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "        THR : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadTHR(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "        PCT : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadPCT(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "        PCI : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadPCI(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "        PWT : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadPWT(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "        PSW : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadPSW(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << " -----------------------------------------------" << std::endl;
  std::cout << std::endl;

  INFO("FADCS [sid=%d]: configuration done", fSID);

  return true;
}

bool CupFADCS::Initialize()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(4000));

  auto * conf = static_cast<FADCSConf *>(fConfig);
  int nch = conf->NCH();
  std::cout << "+++++++++++ FADC PEDESTALS ++++++++++++" << std::endl;
  std::cout << Form("  [sid=%2d]  ", fSID) << std::flush;
  for (int i = 0; i < nch; i++) {
    ULong_t cid = conf->CID(i);
    fFADC.MeasurePED(cid);
    std::cout << Form("%4lu  ", fFADC.ReadPED(cid)) << std::flush;
  }
  std::cout << std::endl;
  std::cout << "+++++++++++ FADC PEDESTALS ++++++++++++" << std::endl;

  INFO("FADCS [sid=%d]: initialized", fSID);

  return true;
}

void CupFADCS::StartTrigger()
{
  fFADC.Reset();
  fFADC.ResetTimer();
  fFADC.Start();
  INFO("FADCS [sid=%d]: trigger started", fSID);
}

void CupFADCS::StopTrigger()
{
  fFADC.Stop();
  fFADC.Reset();

  INFO("FADCS [sid=%d]: trigger stopped", fSID);
}
