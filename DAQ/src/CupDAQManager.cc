// CupDAQManager.cc
#include <cstdint>

#include "DAQ/CupDAQManager.hh"
#include "DAQConfig/FADCSConf.hh"
#include "DAQConfig/FADCTConf.hh"
#include "DAQConfig/GADCSConf.hh"
#include "DAQConfig/IADCTConf.hh"
#include "DAQConfig/MADCSConf.hh"
#include "DAQConfig/SADCTConf.hh"
#include "DAQSystem/CupFADCS.hh"
#include "DAQSystem/CupFADCT.hh"
#include "DAQSystem/CupGADCS.hh"
#include "DAQSystem/CupIADCT.hh"
#include "DAQSystem/CupMADCS.hh"
#include "DAQSystem/CupSADCT.hh"

ClassImp(CupDAQManager)

CupDAQManager::CupDAQManager()
  : TObjArray()
{
  fIsOwnADC = false;
  fIsDAQOpen = false;

  fRunNumber = 0;
  fSubRunNumber = 0;

  fDAQID = 0;
  fADCType = ADC::NONE;
  fTriggerMode = TRIGGER::GLOBAL;

  fStartDatime = 0;
  fEndDatime = 0;

  fTCB = nullptr;

  fMinimumBCount = kMINIMUMBCOUNT;
  fRecordLength = 0;
  fNDP = 0;

  fRunStatus = 0;
  fRunStatusTCB = 0;

  fReadStatus = NONE;
  fSortStatus = NONE;
  fBuildStatus = NONE;
  fWriteStatus = NONE;

  fDoConfigRun = false;
  fDoStartRun = false;
  fDoEndRun = false;
  fDoExit = false;

  fDoConfigRunTCB = false;
  fDoStartRunTCB = false;
  fDoEndRunTCB = false;
  fDoExitTCB = false;

  // thread sleep in microseconds
  fReadSleep = 100000;
  fSortSleep = 100000;
  fBuildSleep = 100000;
  fWriteSleep = 100000;
  fHistSleep = 100000;

  fOutputFileFormat = OUTPUT::ROOT;
  fCompressionLevel = 1;
  fROOTFile = nullptr;
  fROOTTree = nullptr;
#ifdef ENABLE_HDF5
  fHDF5File = nullptr;
  fH5Event = nullptr;
#endif
  fOutputSplitTime = 60 * 60;
  fDoSplitOutputFile = false;
  fDoSplitOutputFileTCB = false;

  fNBuiltEvent = 0;
  fTriggerNumber = 0;
  fTriggerTime = 0;
  fCurrentTime = 0;
  fRemainingBCount = nullptr;

  fTotalRawDataSize = 0;
  fTotalReadDataSize = 0;
  fTotalWrittenDataSize = 0;

  fVerboseLevel = 0;
  fTriggerMonTime = 1;
  fSetNEvent = 0;
  fSetDAQTime = 0;
  fMonitorServerOn = 0;

  fSoftTrigger = nullptr;

  fDoHistograming = false;
  fHistogramerEnded = false;

  fBenchmark = new TBenchmark();

  // for network merge
  fDoSendEvent = false;

  fSendStatus = NONE;
  fRecvStatus = NONE;
  fMergeStatus = NONE;

  fSendSleep = 100000;
  fMergeSleep = 100000;
}

CupDAQManager::~CupDAQManager()
{
  for (auto * buffer : fADCRawBuffers) {
    delete buffer;
  }
  fADCRawBuffers.clear();

  while (!fBuiltEventBuffer1.empty()) {
    fBuiltEventBuffer1.pop_front();
  }
  while (!fBuiltEventBuffer2.empty()) {
    fBuiltEventBuffer2.pop_front();
  }

  fRecvEventBuffer.clear();

  if (fIsOwnADC) { Clear(); }

  if (fRemainingBCount) {
    delete[] fRemainingBCount;
    fRemainingBCount = nullptr;
  }

  delete fBenchmark;
  fBenchmark = nullptr;
}

void CupDAQManager::AddADC(AbsADC * adc)
{
  Add(adc);
  INFO("%s[sid=%2d] added to DAQ manager", GetADCName(fADCType), adc->GetSID());
  if (!fIsOwnADC) { fIsOwnADC = true; }
}

bool CupDAQManager::AddADC(AbsConf * conf)
{
  AbsADC * adc = nullptr;

  if (!conf->IsEnabled()) { return true; }
  if (!conf->IsLinked()) {
    WARNING("%s[sid=%2d] enabled but not linked", GetADCName(fADCType), conf->SID());
  }

  switch (fADCType) {
    case ADC::SADCT: adc = new CupSADCT(conf); break;
    case ADC::FADCS: adc = new CupFADCS(conf); break;
    case ADC::FADCT: adc = new CupFADCT(conf); break;
    case ADC::GADCS: adc = new CupGADCS(conf); break;
    case ADC::MADCS: adc = new CupMADCS(conf); break;
    case ADC::IADCT: adc = new CupIADCT(conf); break;
    default: break;
  }
  Add(adc);

  return true;
}

bool CupDAQManager::AddADC(AbsConfList * conflist)
{
  int nadc = conflist->GetNADC(fADCType);
  if (nadc == 0) {
    ERROR("there is no %s", GetADCName(fADCType));
    return false;
  }

  for (int i = 0; i < nadc; i++) {
    AbsConf * conf = conflist->GetConfig(fADCType, i);
    if (conf->GetDAQID() == fDAQID) {
      if (!AddADC(conf)) { return false; }
    }
  }

  return true;
}

AbsADC * CupDAQManager::FindADC(int sid)
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; ++i) {
    auto * mod = static_cast<AbsADC *>(fCont[i]);
    if (mod != nullptr && mod->GetMID() == sid) { return mod; }
  }

  WARNING("no ADC [sid=%2d]", sid);
  return nullptr;
}

int CupDAQManager::FindADCAt(int sid)
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; ++i) {
    auto * mod = static_cast<AbsADC *>(fCont[i]);
    if (mod != nullptr && mod->GetMID() == sid) { return i; }
  }

  WARNING("no ADC [sid=%2d]", sid);
  return -1;
}

bool CupDAQManager::OpenDAQ()
{
  int nadc = GetEntries();
  for (int i = 0; i < nadc; i++) {
    auto * adc = static_cast<AbsADC *>(fCont[i]);
    if (adc->Open() != 0) { return false; }
  }

  fIsDAQOpen = true;

  INFO("all ADCs are opened");
  return true;
}

void CupDAQManager::CloseDAQ()
{
  if (!fIsDAQOpen) { return; }

  int nadc = GetEntries();
  for (int i = 0; i < nadc; i++) {
    auto * adc = static_cast<AbsADC *>(fCont[i]);
    adc->Close();
  }

  INFO("all ADCs are closed");
}

bool CupDAQManager::PrepareDAQ()
{
  int nadc = GetEntries();
  if (nadc == 0) {
    ERROR("no ADC added");
    return false;
  }

  switch (fADCType) {
    case ADC::SADCT: {
      fMinimumBCount = kMINIMUMBCOUNT;
      fADCMode = ADC::SMODE;
      break;
    }
    case ADC::FADCS: {
      auto * conf = static_cast<FADCSConf *>(static_cast<AbsADC *>(fCont[0])->GetConfig());
      fRecordLength = conf->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      fMinimumBCount = (bcount <= kMINIMUMBCOUNT) ? kMINIMUMBCOUNT : bcount;
      fADCMode = ADC::FMODE;
      break;
    }
    case ADC::FADCT: {
      auto * conf = static_cast<FADCTConf *>(static_cast<AbsADC *>(fCont[0])->GetConfig());
      fRecordLength = conf->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      fMinimumBCount = (bcount <= kMINIMUMBCOUNT) ? kMINIMUMBCOUNT : bcount;
      fADCMode = ADC::FMODE;
      break;
    }
    case ADC::GADCS: {
      auto * conf = static_cast<GADCSConf *>(static_cast<AbsADC *>(fCont[0])->GetConfig());
      fRecordLength = conf->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      fMinimumBCount = (bcount <= kMINIMUMBCOUNT) ? kMINIMUMBCOUNT : bcount;
      fADCMode = ADC::FMODE;
      break;
    }
    case ADC::MADCS: {
      auto * conf = static_cast<MADCSConf *>(static_cast<AbsADC *>(fCont[0])->GetConfig());
      fRecordLength = conf->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      fMinimumBCount = (bcount <= kMINIMUMBCOUNT) ? kMINIMUMBCOUNT : bcount;
      fADCMode = ADC::FMODE;
      break;
    }
    case ADC::IADCT: {
      auto * conf = static_cast<IADCTConf *>(static_cast<AbsADC *>(fCont[0])->GetConfig());
      fRecordLength = conf->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      fMinimumBCount = (bcount <= kMINIMUMBCOUNT) ? kMINIMUMBCOUNT : bcount;
      fADCMode = (fRecordLength > 0) ? ADC::FMODE : ADC::SMODE;
      break;
    }
    default: break;
  }

  fADCEventDataSize = GetADCEventDataSize();
  fADCChannelDataSize = GetADCChannelDataSize();
  fNDP = GetNDP();

  for (int i = 0; i < nadc; i++) {
    auto * buffer = new ConcurrentDeque<std::unique_ptr<AbsADCRaw>>();
    fADCRawBuffers.push_back(buffer);
  }

  Sort();

  TString report = "\n";
  report += "============ CupDAQManager Prepare Report ==============\n";
  report += Form("                            type: %s\n", GetADCName(fADCType));
  report += Form("                   number of ADC: %d\n", nadc);
  report += Form("           minimum buffer count : %d\n", fMinimumBCount);
  report += Form("                  record length : %d\n", fRecordLength);
  report += Form("           number of data point : %d\n", GetNDP());
  report += Form("  minimum data size for reading : %d\n", GetADCEventDataSize());
  report += Form("         preset number of event : %d\n", fSetNEvent);
  report += Form("            preset daq time [s] : %d\n", fSetDAQTime);
  report += "=======================================================\n";

  INFO("%s", report.Data());

  fRemainingBCount = new int[nadc];
  for (int i = 0; i < nadc; i++) {
    fRemainingBCount[i] = 0;
  }

  INFO("prepared to take data from %s", GetADCName(fADCType));
  return true;
}

bool CupDAQManager::ConfigureDAQ()
{
  bool status = true;

  if (IsStandaloneDAQ()) {
    int nadc = GetEntries();
    for (int i = 0; i < nadc; i++) {
      auto * adc = static_cast<AbsADC *>(fCont[i]);
      status &= adc->Configure();
    }
  }
  else {
    ERROR("ConfigureDAQ is not for %s", GetADCName(fADCType));
    return false;
  }

  return status;
}

bool CupDAQManager::InitializeDAQ()
{
  bool status = true;

  if (IsStandaloneDAQ()) {
    int nadc = GetEntries();
    for (int i = 0; i < nadc; i++) {
      auto * adc = static_cast<AbsADC *>(fCont[i]);
      status &= adc->Initialize();
    }
  }
  else {
    ERROR("InitializeDAQ is not for %s", GetADCName(fADCType));
    return false;
  }

  return status;
}

void CupDAQManager::StartTrigger()
{
  if (IsStandaloneDAQ()) {
    int nadc = GetEntries();
    for (int i = 0; i < nadc; i++) {
      auto * adc = static_cast<AbsADC *>(fCont[i]);
      adc->StartTrigger();
    }
    time(&fStartDatime);
  }
  else {
    WARNING("StartTrigger is not for %s", GetADCName(fADCType));
  }
}

void CupDAQManager::StopTrigger()
{
  if (IsStandaloneDAQ()) {
    int nadc = GetEntries();
    for (int i = 0; i < nadc; i++) {
      auto * adc = static_cast<AbsADC *>(fCont[i]);
      adc->StopTrigger();
    }
    time(&fEndDatime);
  }
  else {
    WARNING("StopTrigger is not for %s", GetADCName(fADCType));
  }
}

int CupDAQManager::ReadBCount(int n)
{
  auto * adc = static_cast<AbsADC *>(fCont[n]);
  int bcount = adc->ReadBCount();
  if (bcount < 0) {
    ERROR("error in reading buffer count [sid=%d]", adc->GetSID());
    return -1;
  }
  return bcount;
}

int CupDAQManager::ReadBCountMin(int * bcounts)
{
  int min = INT32_MAX;

  int nadc = GetEntries();
  for (int i = 0; i < nadc; i++) {
    int bcount = ReadBCount(i);
    if (bcount < 0) { return -1; }
    if (bcounts != nullptr) { bcounts[i] = bcount; }
    if (bcount < min) { min = bcount; }
  }
  return min;
}

int CupDAQManager::ReadBCountMax(int * bcounts)
{
  int max = INT32_MIN;

  int nadc = GetEntries();
  for (int i = 0; i < nadc; i++) {
    int bcount = ReadBCount(i);
    if (bcount < 0) { return -1; }
    if (bcounts != nullptr) { bcounts[i] = bcount; }
    if (bcount > max) { max = bcount; }
  }
  return max;
}

int CupDAQManager::ReadADCData(int n, int bcount, unsigned char * databuffer)
{
  int stat;

  auto * adc = static_cast<AbsADC *>(fCont[n]);
  if (databuffer != nullptr) { stat = adc->ReadData(bcount, databuffer); }
  else {
    stat = adc->ReadData(bcount);
  }

  if (stat < 0) {
    ERROR("error in reading %s data [sid=%d]", GetADCName(fADCType), adc->GetSID());
    return -1;
  }

  return bcount;
}

int CupDAQManager::ReadData(int bcount, unsigned char ** databuffer)
{
  int bcountmin = ReadBCountMin();
  if (bcountmin < 0) { return -1; }
  if (bcountmin < bcount) { return 0; }

  int nadc = GetEntries();
  for (int i = 0; i < nadc; i++) {
    ReadADCData(i, bcount, databuffer[i]);
  }

  return bcount;
}
