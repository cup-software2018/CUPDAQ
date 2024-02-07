/*
 *  Module:  CupDAQManager
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: DAQ helper class to simplify read data from ADCs
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/05/10 22:26:30 $
 *  CVS/RCS Revision: $Revision: 1.19 $
 *  Status:           $State: Exp $
 *
 */

#include "DAQ/CupDAQManager.hh"
#include "DAQConfig/FADCSConf.hh"
#include "DAQConfig/FADCTConf.hh"
#include "DAQConfig/GADCSConf.hh"
#include "DAQConfig/GADCTConf.hh"
#include "DAQConfig/IADCTConf.hh"
#include "DAQConfig/MADCSConf.hh"
#include "DAQConfig/SADCSConf.hh"
#include "DAQConfig/SADCTConf.hh"
#include "DAQSystem/CupFADCS.hh"
#include "DAQSystem/CupFADCT.hh"
#include "DAQSystem/CupGADCS.hh"
#include "DAQSystem/CupGADCT.hh"
#include "DAQSystem/CupIADCT.hh"
#include "DAQSystem/CupMADCS.hh"
#include "DAQSystem/CupSADCS.hh"
#include "DAQSystem/CupSADCT.hh"

using namespace std;

ClassImp(CupDAQManager)

    CupDAQManager::CupDAQManager()
    : TObjArray()
{
  fLog = ELogger::Instance();

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
  fUSB = nullptr;

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
  int nadc = GetEntries();

  for (int i = 0; i < nadc; i++) {
    if (!fADCRawBuffers.empty()) {
      ConcurrentDeque<AbsADCRaw *> * buffer = fADCRawBuffers.at(i);
      while (!buffer->empty()) {
        AbsADCRaw * adcraw = buffer->popfront();
        delete adcraw;
      }
    }
  }

  while (!fBuiltEventBuffer1.empty()) {
    auto * event = fBuiltEventBuffer1.popfront(false);
    delete event;
  }

  while (!fBuiltEventBuffer2.empty()) {
    auto * event = fBuiltEventBuffer2.popfront(false);
    delete event;
  }

  if (fIsOwnADC) { Clear(); }
  if (fRemainingBCount) { delete[] fRemainingBCount; }

  delete fBenchmark;
  fLog->DeleteInstance();
}

void CupDAQManager::AddADC(AbsADC * adc)
{
  Add(adc);
  fLog->Info("CupDAQManager::AddADC", "%s[sid=%2d] added to DAQ manager",
             GetADCName(fADCType), adc->GetSID());
  if (!fIsOwnADC) fIsOwnADC = true;
}

bool CupDAQManager::AddADC(AbsConf * conf)
{
  AbsADC * adc = nullptr;

  if (!conf->IsEnabled()) return true;
  if (!conf->IsLinked()) {
    fLog->Error("CupDAQManager::AddADC", "%s[sid=%2d] enable but not linked",
                GetADCName(fADCType), conf->SID());
    return false;
  }

  switch (fADCType) {
    case ADC::SADCS: adc = new CupSADCS(conf); break;
    case ADC::SADCT: adc = new CupSADCT(conf); break;
    case ADC::FADCS: adc = new CupFADCS(conf); break;
    case ADC::FADCT: adc = new CupFADCT(conf); break;
    case ADC::GADCS: adc = new CupGADCS(conf); break;
    case ADC::GADCT: adc = new CupGADCT(conf); break;
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
    fLog->Error("CupDAQManager::AddADC", "there is no %s",
                GetADCName(fADCType));
    return false;
  }

  for (int i = 0; i < nadc; i++) {
    AbsConf * conf = conflist->GetConfig(fADCType, i);
    if (conf->GetDAQID() == fDAQID) {
      if (!AddADC(conf)) return false;
    }
  }

  return true;
}

AbsADC * CupDAQManager::FindADC(int sid)
{
  Int_t nadc = GetAbsLast() + 1;
  for (Int_t i = 0; i < nadc; ++i) {
    auto * mod = (AbsADC *)fCont[i];

    if (mod && mod->GetMID() == sid) { return mod; }
  }

  fLog->Warning("CupDAQManager::FindADC", "no ADC [sid=%2d]", sid);
  return nullptr;
}

int CupDAQManager::FindADCAt(int sid)
{
  Int_t nadc = GetAbsLast() + 1;
  for (Int_t i = 0; i < nadc; ++i) {
    auto * mod = (AbsADC *)fCont[i];

    if (mod && mod->GetMID() == sid) { return i; }
  }

  fLog->Warning("CupDAQManager::FindADC", "no ADC [sid=%2d]", sid);
  return -1;
}

//
// For DAQ
//

bool CupDAQManager::OpenDAQ()
{
  fUSB = new usb3comroot;
  fUSB->USB3Init(nullptr);

  int nadc = GetEntries();
  for (int i = 0; i < nadc; i++) {
    auto * adc = (AbsADC *)fCont[i];
    if (adc->Open() != 0) return false;
  }

  fIsDAQOpen = true;

  fLog->Info("CupDAQManager::OpenDAQ", "all ADCs are opened");
  return true;
}

void CupDAQManager::CloseDAQ()
{
  if (!fIsDAQOpen) return;

  int nadc = GetEntries();
  for (int i = 0; i < nadc; i++) {
    auto * adc = (AbsADC *)fCont[i];
    adc->Close();
  }

  if (fUSB) {
    fUSB->USB3Exit(nullptr);
    delete fUSB;
  }

  fLog->Info("CupDAQManager::CloseDAQ", "all ADCs are closed");
}

bool CupDAQManager::PrepareDAQ()
{
  int nadc = GetEntries();
  if (nadc == 0) {
    fLog->Error("CupDAQManager::PrepareDAQ", "no ADC added");
    return false;
  }

  switch (fADCType) {
    case ADC::SADCS: {
      fMinimumBCount = kMINIMUMBCOUNT;
      fADCMode = ADC::SMODE;
      break;
    }
    case ADC::SADCT: {
      fMinimumBCount = kMINIMUMBCOUNT;
      fADCMode = ADC::SMODE;
      break;
    }
    case ADC::FADCS: {
      AbsConf * conf = ((AbsADC *)fCont[0])->GetConfig();
      fRecordLength = ((FADCSConf *)conf)->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      if (bcount <= kMINIMUMBCOUNT) { fMinimumBCount = kMINIMUMBCOUNT; }
      else {
        fMinimumBCount = bcount;
      }
      fADCMode = ADC::FMODE;
      break;
    }
    case ADC::FADCT: {
      AbsConf * conf = ((AbsADC *)fCont[0])->GetConfig();
      fRecordLength = ((FADCTConf *)conf)->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      if (bcount <= kMINIMUMBCOUNT) { fMinimumBCount = kMINIMUMBCOUNT; }
      else {
        fMinimumBCount = bcount;
      }
      fADCMode = ADC::FMODE;
      break;
    }
    case ADC::GADCS: {
      AbsConf * conf = ((AbsADC *)fCont[0])->GetConfig();
      fRecordLength = ((GADCSConf *)conf)->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      if (bcount <= kMINIMUMBCOUNT) { fMinimumBCount = kMINIMUMBCOUNT; }
      else {
        fMinimumBCount = bcount;
      }
      fADCMode = ADC::FMODE;
      break;
    }
    case ADC::GADCT: {
      AbsConf * conf = ((AbsADC *)fCont[0])->GetConfig();
      fRecordLength = ((GADCTConf *)conf)->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      if (bcount <= kMINIMUMBCOUNT) { fMinimumBCount = kMINIMUMBCOUNT; }
      else {
        fMinimumBCount = bcount;
      }
      fADCMode = ADC::FMODE;
      break;
    }
    case ADC::MADCS: {
      AbsConf * conf = ((AbsADC *)fCont[0])->GetConfig();
      fRecordLength = ((MADCSConf *)conf)->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      if (bcount <= kMINIMUMBCOUNT) { fMinimumBCount = kMINIMUMBCOUNT; }
      else {
        fMinimumBCount = bcount;
      }
      fADCMode = ADC::FMODE;
      break;
    }
    case ADC::IADCT: {
      auto * conf = (IADCTConf *)((AbsADC *)fCont[0])->GetConfig();
      fRecordLength = conf->RL();
      int bcount = GetADCEventDataSize() / kKILOBYTES;
      if (bcount <= kMINIMUMBCOUNT) { fMinimumBCount = kMINIMUMBCOUNT; }
      else {
        fMinimumBCount = bcount;
      }
      if (fRecordLength > 0) fADCMode = ADC::FMODE;
      else fADCMode = ADC::SMODE;
      break;
    }
    default: break;
  }

  fADCEventDataSize = GetADCEventDataSize();
  fADCChannelDataSize = GetADCChannelDataSize();
  fNDP = GetNDP();

  for (int i = 0; i < nadc; i++) {
    auto * buffer = new ConcurrentDeque<AbsADCRaw *>();
    fADCRawBuffers.push_back(buffer);
  }

  // Sort adcs by sid
  this->Sort();

  TString report = "\n";
  report += "============ CupDAQManager Prepare Report ==============\n";
  report +=
      Form("                            type: %s\n", GetADCName(fADCType));
  report += Form("                   number of ADC: %d\n", nadc);
  report += Form("           minimum buffer count : %d\n", fMinimumBCount);
  report += Form("                  record length : %d\n", fRecordLength);
  report += Form("           number of data point : %d\n", GetNDP());
  report +=
      Form("  minimum data size for reading : %d\n", GetADCEventDataSize());
  report += Form("         preset number of event : %d\n", fSetNEvent);
  report += Form("            preset daq time [s] : %d\n", fSetDAQTime);
  report += "=======================================================\n";

  fLog->PrintLog(report.Data());

  fRemainingBCount = new int[nadc];
  for (int i = 0; i < nadc; i++) {
    fRemainingBCount[i] = 0;
  }

  fLog->Info("CupDAQManager::PrepareDAQ", "prepared to take data from %s",
             GetADCName(fADCType));
  return true;
}

bool CupDAQManager::ConfigureDAQ()
{
  bool status = true;

  if (IsStandaloneDAQ()) {
    int nadc = GetEntries();
    for (int i = 0; i < nadc; i++) {
      auto * adc = (AbsADC *)fCont[i];
      status &= adc->Configure();
    }
  }
  else {
    fLog->Error("CupDAQManager::ConfigureDAQ", "not for %s",
                GetADCName(fADCType));
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
      auto * adc = (AbsADC *)fCont[i];
      status &= adc->Initialize();
    }
  }
  else {
    fLog->Error("CupDAQManager::InitializeDAQ", "not for %s",
                GetADCName(fADCType));
    return false;
  }

  return status;
}

void CupDAQManager::StartTrigger()
{
  if (IsStandaloneDAQ()) {
    int nadc = GetEntries();
    for (int i = 0; i < nadc; i++) {
      auto * adc = (AbsADC *)fCont[i];
      adc->StartTrigger();
    }
    time(&fStartDatime);
  }
  else {
    fLog->Warning("CupDAQManager::StartTrigger", "not for %s",
                  GetADCName(fADCType));
  }
}

void CupDAQManager::StopTrigger()
{
  if (IsStandaloneDAQ()) {
    int nadc = GetEntries();
    for (int i = 0; i < nadc; i++) {
      auto * adc = (AbsADC *)fCont[i];
      adc->StopTrigger();
    }
    time(&fEndDatime);
  }
  else {
    fLog->Warning("CupDAQManager::StopTrigger", "not for %s",
                  GetADCName(fADCType));
  }
}

int CupDAQManager::ReadBCount(int n)
{
  auto * adc = (AbsADC *)fCont[n];
  int bcount = adc->ReadBCount();
  if (bcount < 0) {
    fLog->Error("CupDAQManager::ReadBCount",
                "error in reading buffer count [sid=%d]", adc->GetSID());
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
    if (bcounts) { bcounts[i] = bcount; }
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
    if (bcounts) { bcounts[i] = bcount; }
    if (bcount > max) { max = bcount; }
  }
  return max;
}

int CupDAQManager::ReadADCData(int n, int bcount, unsigned char * databuffer)
{
  int stat;

  auto * adc = (AbsADC *)fCont[n];
  if (databuffer) { stat = adc->ReadData(bcount, databuffer); }
  else {
    stat = adc->ReadData(bcount);
  }

  if (stat < 0) {
    fLog->Error("CupDAQManager::ReadADCData",
                "error in reading %s data [sid=%d]", GetADCName(fADCType),
                adc->GetSID());
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
