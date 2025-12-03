#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "DAQConfig/MADCSConf.hh"
#include "DAQSystem/CupMADCS.hh"
#include "DAQUtils/ELog.hh"

ClassImp(CupMADCS)

CupMADCS::CupMADCS(int sid)
  : AbsADC(sid)
{
  fFADC.SetSID(sid);
}

CupMADCS::CupMADCS(AbsConf * config)
  : AbsADC(config)
{
  fFADC.SetSID(config->SID());
}

int CupMADCS::Open()
{
  int stat = fFADC.Open();
  if (stat != 0) {
    ERROR("MADCS [sid=%d]: open failed, check connection and power", fSID);
    return stat;
  }
  INFO("MADCS [sid=%d]: opened", fSID);

  if (fConfig) {
    auto * config = static_cast<MADCSConf *>(fConfig);
    fEventDataSize = kNCHMADC * 32 * config->RL();
  }

  return stat;
}

void CupMADCS::Close()
{
  fFADC.Stop();
  fFADC.Reset();
  fFADC.Close();

  INFO("MADCS [sid=%d]: closed", fSID);
}

int CupMADCS::ReadBCount() { return fFADC.ReadBCount(); }

int CupMADCS::ReadData(int bcount, unsigned char * data)
{
  int state = fFADC.ReadData(bcount, data);
  if (state != 0) { return state; }

  fTotalBCount += static_cast<unsigned long>(bcount);

  if (fEventDataSize > 0) {
    int n = kKILOBYTES * bcount / fEventDataSize;
    if (n > 0) {
      unsigned char * tempdata = &(data[fEventDataSize * (n - 1)]);
      UpdateTriggerAndTime(tempdata);
    }
  }

  return state;
}

int CupMADCS::ReadData(int bcount)
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

void CupMADCS::UpdateTriggerAndTime(const unsigned char * tempdata)
{
  unsigned long finetime = 0;
  unsigned long coarsetime = 0;

  std::unique_lock<std::mutex> lock(fMutex);

  // trigger number
  fCurrentTrgNumber = static_cast<unsigned int>(tempdata[68] & 0xFFu);
  fCurrentTrgNumber |= static_cast<unsigned int>(tempdata[72] & 0xFFu) << 8;
  fCurrentTrgNumber |= static_cast<unsigned int>(tempdata[76] & 0xFFu) << 16;
  fCurrentTrgNumber |= static_cast<unsigned int>(tempdata[80] & 0xFFu) << 24;
  fCurrentTrgNumber += 1;

  // fine time
  finetime = static_cast<unsigned long>(tempdata[100] & 0xFFu) * 8ul;

  // coarse time
  coarsetime = static_cast<unsigned long>(tempdata[104] & 0xFFu);
  coarsetime |= static_cast<unsigned long>(tempdata[108] & 0xFFu) << 8;
  coarsetime |= static_cast<unsigned long>(tempdata[112] & 0xFFu) << 16;
  coarsetime |= static_cast<unsigned long>(tempdata[116] & 0xFFu) << 24;
  coarsetime |= static_cast<unsigned long>(tempdata[120] & 0xFFu) << 32;
  coarsetime |= static_cast<unsigned long>(tempdata[124] & 0xFFu) << 40;
  coarsetime *= 1000ul;

  fCurrentTime = coarsetime + finetime;
}

bool CupMADCS::Configure()
{
  if (!fConfig) {
    ERROR("MADCS [sid=%d]: no configuration", fSID);
    return false;
  }

  if (!TString(fConfig->GetName()).EqualTo("MADCS")) {
    ERROR("MADCS [sid=%d]: configuration not matched with MADCS", fSID);
    return false;
  }

  fFADC.Reset();
  fFADC.AlignADC();
  fFADC.WriteDRAMON(1);
  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  unsigned long dramon = fFADC.ReadDRAMON();
  if (dramon) { INFO("MADCS [sid=%d]: DRAM on", fSID); }
  else {
    ERROR("MADCS [sid=%d]: error occurred during turning DRAM on", fSID);
    return false;
  }

  fFADC.AlignDRAM();

  auto * conf = static_cast<MADCSConf *>(fConfig);
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

  std::cout << Form(" ++ MADC register: SID(%lu) MID(%lu) NCH(%1d) RL(%lu) DSR(%lu) "
                    "TLT(%lX) TRGON(%lu) PTRG(%lu) PSC(%lu)",
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
    std::cout << Form("%8lu", fFADC.ReadPOL(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "     DACOFF : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadDACOFF(conf->CID(i)));
  }
  std::cout << std::endl;
  std::cout << "        DLY : ";
  for (int i = 0; i < nch; i++) {
    std::cout << Form("%8lu", fFADC.ReadDLY(conf->CID(i)));
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

  INFO("MADCS [sid=%d]: configuration done", fSID);

  return true;
}

bool CupMADCS::Initialize()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(4000));

  auto * conf = static_cast<MADCSConf *>(fConfig);
  int nch = conf->NCH();
  std::cout << "+++++++++++ MADC PEDESTALS ++++++++++++" << std::endl;
  std::cout << Form("  [sid=%2d]  ", fSID) << std::flush;
  for (int i = 0; i < nch; i++) {
    ULong_t cid = conf->CID(i);
    fFADC.MeasurePED(cid);
    std::cout << Form("%4lu  ", fFADC.ReadPED(cid)) << std::flush;
  }
  std::cout << std::endl;
  std::cout << "+++++++++++ MADC PEDESTALS ++++++++++++" << std::endl;

  INFO("MADCS [sid=%d]: initialized", fSID);

  return true;
}

void CupMADCS::StartTrigger()
{
  fFADC.Reset();
  fFADC.ResetTimer();
  fFADC.Start();

  INFO("MADCS [sid=%d]: trigger started", fSID);
}

void CupMADCS::StopTrigger()
{
  fFADC.Stop();
  fFADC.Reset();

  INFO("MADCS [sid=%d]: trigger stopped", fSID);
}
