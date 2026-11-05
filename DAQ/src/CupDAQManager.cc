// CupDAQManager.cc
#include <algorithm>
#include <cstdint>

#include "CupDAQManager.hh"
#include "FADCSConf.hh"
#include "FADCTConf.hh"
#include "GADCSConf.hh"
#include "IADCTConf.hh"
#include "MADCSConf.hh"
#include "CupFADCS.hh"
#include "CupFADCT.hh"
#include "CupGADCS.hh"
#include "CupIADCT.hh"
#include "CupMADCS.hh"
#include "CupSADCT.hh"
#include "ELog.hh"

CupDAQManager::CupDAQManager()
{
  fIsDAQOpen = false;

  fRunNumber = 0;
  fSubRunNumber = 1;
  fSplitTargetSubrun = 1;
  fOutputFileOpened = false;

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

  fSendSleep = 100000;
}

CupDAQManager::~CupDAQManager()
{
  if (fRemainingBCount) {
    delete[] fRemainingBCount;
    fRemainingBCount = nullptr;
  }

  delete fBenchmark;
  fBenchmark = nullptr;
}

void CupDAQManager::AddADC(std::unique_ptr<AbsADC> adc)
{
  INFO("%s[sid=%2d] added to DAQ manager", GetADCName(fADCType), adc->GetSID());
  fADCList.push_back(std::move(adc));
}

bool CupDAQManager::AddADC(AbsConf * conf)
{
  std::unique_ptr<AbsADC> adc;

  if (!conf->IsEnabled()) {
    WARNING("%s[sid=%2d] disabled in config", GetADCName(fADCType), conf->SID());
    return true;
  }
  if (!conf->IsLinked()) {
    ERROR("%s[sid=%2d] enabled but not linked", GetADCName(fADCType), conf->SID());
    return false;
  }

  switch (fADCType) {
    case ADC::SADCT: adc = std::make_unique<CupSADCT>(conf); break;
    case ADC::FADCS: adc = std::make_unique<CupFADCS>(conf); break;
    case ADC::FADCT: adc = std::make_unique<CupFADCT>(conf); break;
    case ADC::GADCS: adc = std::make_unique<CupGADCS>(conf); break;
    case ADC::MADCS: adc = std::make_unique<CupMADCS>(conf); break;
    case ADC::IADCT: adc = std::make_unique<CupIADCT>(conf); break;
    default: break;
  }
  AddADC(std::move(adc));

  return true;
}

bool CupDAQManager::AddADC(AbsConfList * conflist)
{
  int nadc = conflist->GetNADC(fADCType);
  if (nadc == 0) {
    ERROR("there is no %s in config", GetADCName(fADCType));
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
  int nadc = static_cast<int>(fADCList.size());
  for (int i = 0; i < nadc; ++i) {
    auto * mod = fADCList[i].get();
    if (mod != nullptr && mod->GetMID() == sid) { return mod; }
  }

  WARNING("no ADC [sid=%2d]", sid);
  return nullptr;
}

int CupDAQManager::FindADCAt(int sid)
{
  int nadc = static_cast<int>(fADCList.size());
  for (int i = 0; i < nadc; ++i) {
    auto * mod = fADCList[i].get();
    if (mod != nullptr && mod->GetMID() == sid) { return i; }
  }

  WARNING("no ADC [sid=%2d]", sid);
  return -1;
}

bool CupDAQManager::OpenDAQ()
{
  int nadc = static_cast<int>(fADCList.size());
  for (int i = 0; i < nadc; i++) {
    auto * adc = fADCList[i].get();
    if (adc->Open() != 0) { return false; }
  }

  fIsDAQOpen = true;

  INFO("all ADCs are opened");
  return true;
}

void CupDAQManager::CloseDAQ()
{
  if (!fIsDAQOpen) { return; }

  int nadc = static_cast<int>(fADCList.size());
  for (int i = 0; i < nadc; i++) {
    auto * adc = fADCList[i].get();
    adc->Close();
  }

  INFO("all ADCs are closed");
}

bool CupDAQManager::PrepareDAQ()
{
  int nadc = static_cast<int>(fADCList.size());
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
      auto * conf = static_cast<FADCSConf *>(fADCList[0]->GetConfig());
      fRecordLength = conf->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      fMinimumBCount = (bcount <= kMINIMUMBCOUNT) ? kMINIMUMBCOUNT : bcount;
      fADCMode = ADC::FMODE;
      break;
    }
    case ADC::FADCT: {
      auto * conf = static_cast<FADCTConf *>(fADCList[0]->GetConfig());
      fRecordLength = conf->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      fMinimumBCount = (bcount <= kMINIMUMBCOUNT) ? kMINIMUMBCOUNT : bcount;
      fADCMode = ADC::FMODE;
      break;
    }
    case ADC::GADCS: {
      auto * conf = static_cast<GADCSConf *>(fADCList[0]->GetConfig());
      fRecordLength = conf->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      fMinimumBCount = (bcount <= kMINIMUMBCOUNT) ? kMINIMUMBCOUNT : bcount;
      fADCMode = ADC::FMODE;
      break;
    }
    case ADC::MADCS: {
      auto * conf = static_cast<MADCSConf *>(fADCList[0]->GetConfig());
      fRecordLength = conf->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      fMinimumBCount = (bcount <= kMINIMUMBCOUNT) ? kMINIMUMBCOUNT : bcount;
      fADCMode = ADC::FMODE;
      break;
    }
    case ADC::IADCT: {
      auto * conf = static_cast<IADCTConf *>(fADCList[0]->GetConfig());
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

  std::sort(fADCList.begin(), fADCList.end(),
            [](const std::unique_ptr<AbsADC> & a, const std::unique_ptr<AbsADC> & b) {
              return a->GetSID() < b->GetSID();
            });

  char buf[128];
  std::string report = "\n\n";
  report += "============ CupDAQManager Preparation Report ==============\n";
  snprintf(buf, sizeof(buf), "                            type: %s\n", GetADCName(fADCType));
  report += buf;
  snprintf(buf, sizeof(buf), "                   number of ADC: %d\n", nadc);
  report += buf;
  snprintf(buf, sizeof(buf), "           minimum buffer count : %d\n", fMinimumBCount);
  report += buf;
  snprintf(buf, sizeof(buf), "                  record length : %d\n", fRecordLength);
  report += buf;
  snprintf(buf, sizeof(buf), "           number of data point : %d\n", GetNDP());
  report += buf;
  snprintf(buf, sizeof(buf), "  minimum data size for reading : %d\n", GetADCEventDataSize());
  report += buf;
  snprintf(buf, sizeof(buf), "         preset number of event : %d\n", fSetNEvent);
  report += buf;
  snprintf(buf, sizeof(buf), "            preset daq time [s] : %d\n", fSetDAQTime);
  report += buf;
  report += "============================================================\n";

  INFO("%s", report.c_str());

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
    int nadc = static_cast<int>(fADCList.size());
    for (int i = 0; i < nadc; i++) {
      auto * adc = fADCList[i].get();
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
    int nadc = static_cast<int>(fADCList.size());
    for (int i = 0; i < nadc; i++) {
      auto * adc = fADCList[i].get();
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
    int nadc = static_cast<int>(fADCList.size());
    for (int i = 0; i < nadc; i++) {
      auto * adc = fADCList[i].get();
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
    int nadc = static_cast<int>(fADCList.size());
    for (int i = 0; i < nadc; i++) {
      auto * adc = fADCList[i].get();
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
  auto * adc = fADCList[n].get();
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

  int nadc = static_cast<int>(fADCList.size());
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

  int nadc = static_cast<int>(fADCList.size());
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

  auto * adc = fADCList[n].get();
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

  int nadc = static_cast<int>(fADCList.size());
  for (int i = 0; i < nadc; i++) {
    ReadADCData(i, bcount, databuffer[i]);
  }

  return bcount;
}
