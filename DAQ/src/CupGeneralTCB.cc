#include <algorithm>
#include <iostream>

#include "TSystem.h"

#include "DAQ/CupGeneralTCB.hh"
#include "OnlConsts/onlconsts.hh"

using namespace std;

ClassImp(CupGeneralTCB)

    CupGeneralTCB::CupGeneralTCB()
    : TObject()
{
  fLog = ELogger::Instance(true);

  fTCBType = TCB::V1;
  fADCType = ADC::NONE;
  fTCB = nullptr;
  fTCBConfig = nullptr;
  fConfigs = nullptr;
}

CupGeneralTCB::~CupGeneralTCB() { delete fTCB; }

int CupGeneralTCB::Open()
{
  fTCBConfig = (TCBConf *)fConfigs->GetTCBConfig();

  fTCBType = fTCBConfig->TCBTYPE();

  if (fTCBType == TCB::MINI) {
    fTCB = new CupMiniTCB();
    ((CupMiniTCB *)fTCB)->SetIPAddress("192.168.0.2");
    fLog->Info("CupGeneralTCB::Open", "open with MiniTCB");
  }
  else {
    fTCB = CupTCB::Instance();
    fLog->Info("CupGeneralTCB::Open", "open with TCB");
  }

  int state = fTCB->Open();
  if (state != 0) {
    fLog->Error("CupGeneralTCB::Open",
                "tcb open failed, check power or link status");
  }
  else {
    fLog->Info("CupGeneralTCB::Open", "tcb opened");
  }
  return state;
}

void CupGeneralTCB::Close()
{
  // turn off SiPM HVs
  int nadc = fConfigs->GetNADC(ADC::IADCT, true);
  if (nadc > 0) {
    for (int i = 0; i < nadc; i++) {
      AbsConf * config = fConfigs->GetConfig(ADC::IADCT, i);
      for (int j = 0; j < 5; j++) {
        fTCB->WriteHV(config->MID(), j + 1, 0.0);
      }
    }
    fLog->Info("CupGeneralTCB::Close", "turned off all SIPM HVs");
    gSystem->Sleep(1000);
  }

  fTCB->Close();
  fLog->Info("CupGeneralTCB::Close", "tcb closed");
}

bool CupGeneralTCB::Config()
{
  fLog->Info("CupGeneralTCB::Config", "module configuration start");

  int nconf = fConfigs->GetEntries();

  if (nconf == 0) {
    fLog->Error("CupGeneralTCB::Config", "no module in config list");
    return false;
  }

  int nlinked = CheckLinkStatus();
  if (nlinked == 0) {
    fLog->Error("CupGeneralTCB::Config", "no module linked");
    return false;
  }

  for (int i = 0; i < nconf; i++) {
    auto * conf = (AbsConf *)fConfigs->At(i);
    if (!conf->IsEnabled() || !conf->IsLinked()) continue;

    TString name = conf->GetName();
    if (name.Contains("TCB")) continue;

    unsigned long mid = conf->MID();

    fTCB->WriteDRAMON(mid, 1);
    gSystem->Sleep(100);
    unsigned long dramon = fTCB->ReadDRAMON(mid);
    if (dramon) {
      fLog->Info("CupGeneralTCB::Config", "%s[mid=%2d] DRAM on", name.Data(),
                 mid);
    }
    else {
      fLog->Error("CupGeneralTCB::Config",
                  "%s[mid=%2d] error occurred during turning DRAM on",
                  name.Data(), mid);
      return false;
    }

    if (name.Contains("AMOREADC")) {
      // flushing data buffer
      int bcount = fTCB->ReadBCOUNT(mid);
      if (bcount) {
        auto * data = new unsigned char[bcount * kKILOBYTES];
        fTCB->ReadDATA(mid, bcount, data);
      }
    }

    if (!name.Contains("SADC")) { fTCB->AlignDRAM(mid); }
  }

  fTCB->Reset();

  bool retval = true;
  for (int i = 0; i < nconf; i++) {
    auto * conf = (AbsConf *)fConfigs->At(i);
    TString name = conf->GetName();

    if (name.EqualTo("STRG") || name.EqualTo("DAQ")) continue;
    if (name.EqualTo("TCB")) { retval &= ConfigTCB((TCBConf *)conf); }
    else if (name.EqualTo("FADCT")) {
      retval &= ConfigFADC((FADCTConf *)conf);
    }
    else if (name.EqualTo("GADCT")) {
      retval &= ConfigGADC((GADCTConf *)conf);
    }
    else if (name.EqualTo("SADCT")) {
      retval &= ConfigSADC((SADCTConf *)conf);
    }
    else if (name.EqualTo("IADCT")) {
      retval &= ConfigIADC((IADCTConf *)conf);
    }
    else {
      fLog->Warning("CupGeneralTCB::Config", "unknown kind of module : %s",
                    name.Data());
      continue;
    }
  }

  if (retval) {
    fLog->Info("CupGeneralTCB::Config", "all modules configuration done");
  }

  return retval;
}

bool CupGeneralTCB::StartTrigger()
{
  // wait for finishing DAQOFF setting
  gSystem->Sleep(2000);

  // measure FADC pedestal
  int nadc = fConfigs->GetNADC(ADC::FADCT, true);
  if (nadc > 0) {
    fLog->Info("CupGeneralTCB::StartTrigger", "Measuring FADC pedestals");
    cout << endl;
    cout << "+++++++++++ FADC PEDESTALS ++++++++++++" << endl;
    for (int i = 0; i < nadc; i++) {
      auto * conf = (FADCTConf *)fConfigs->GetConfig(ADC::FADCT, i);
      if (conf->IsLinked() && conf->IsEnabled())
        fTCB->MeasurePedestalFADC(conf);
    }
    cout << "+++++++++++ FADC PEDESTALS ++++++++++++" << endl;
    cout << endl;
  }

  // measure GADC pedestal
  nadc = fConfigs->GetNADC(ADC::GADCT, true);
  if (nadc > 0) {
    fLog->Info("CupGeneralTCB::StartTrigger", "Measuring GADC pedestals");
    cout << endl;
    cout << "+++++++++++ GADC PEDESTALS ++++++++++++" << endl;
    for (int i = 0; i < nadc; i++) {
      auto * conf = (GADCTConf *)fConfigs->GetConfig(ADC::GADCT, i);
      if (conf->IsLinked() && conf->IsEnabled())
        fTCB->MeasurePedestalGADC(conf);
    }
    cout << "+++++++++++ GADC PEDESTALS ++++++++++++" << endl;
    cout << endl;
  }

  // measure SADC pedestal
  nadc = fConfigs->GetNADC(ADC::SADCT, true);
  if (nadc > 0) {
    fLog->Info("CupGeneralTCB::StartTrigger", "Measuring SADC pedestals");
    cout << endl;
    cout << "+++++++++++ SADC PEDESTALS ++++++++++++" << endl;
    for (int i = 0; i < nadc; i++) {
      auto * conf = (SADCTConf *)fConfigs->GetConfig(ADC::SADCT, i);
      if (conf->IsLinked() && conf->IsEnabled())
        fTCB->MeasurePedestalSADC(conf);
    }
    cout << "+++++++++++ SADC PEDESTALS ++++++++++++" << endl;
    cout << endl;
  }

  // measure IADC pedestal
  nadc = fConfigs->GetNADC(ADC::IADCT, true);
  if (nadc > 0) {
    fLog->Info("CupGeneralTCB::StartTrigger", "Measuring IADC pedestals");
    cout << endl;
    cout << "+++++++++++ IADC PEDESTALS ++++++++++++" << endl;
    for (int i = 0; i < nadc; i++) {
      auto * conf = (IADCTConf *)fConfigs->GetConfig(ADC::IADCT, i);
      if (conf->IsLinked() && conf->IsEnabled())
        fTCB->MeasurePedestalIADC(conf);
    }
    cout << "+++++++++++ IADC PEDESTALS ++++++++++++" << endl;
    cout << endl;
  }

  fLog->Info("CupGeneralTCB::StartTrigger", "all ADCs initialized");

  gSystem->Sleep(1000);

  fTCB->TriggerStart();
  fLog->Info("CupGeneralTCB::StartTrigger", "trigger started");

  return true;
}

void CupGeneralTCB::StopTrigger()
{
  fTCB->TriggerStop();
  fLog->Info("CupGeneralTCB::StopTrigger", "trigger stopped");
}

bool CupGeneralTCB::ConfigFADC(FADCTConf * conf)
{
  unsigned long mid = conf->MID();

  if (!conf->IsEnabled()) {
    fLog->Info("CupGeneralTCB::ConfigFADC", "%s[mid=%2d] is disabled, passed",
               conf->GetName(), mid);
    return true;
  }

  if (!conf->IsLinked()) {
    fLog->Error("CupGeneralTCB::ConfigFADC",
                "%s[mid=%2d] is enabled but not linked", conf->GetName(), mid);
    return false;
  }

  conf->PrintConf();
  fTCB->WriteRegisterFADC(conf);
  fTCB->PrintRegisterFADC(conf);

  fLog->Info("CupGeneralTCB::ConfigFADC", "%s[mid=%2d] was configured",
             conf->GetName(), mid);
  return true;
}

bool CupGeneralTCB::ConfigGADC(GADCTConf * conf)
{
  unsigned long mid = conf->MID();

  if (!conf->IsEnabled()) {
    fLog->Info("CupGeneralTCB::ConfigGADC", "%s[mid=%2d] is disabled, passed",
               conf->GetName(), mid);
    return true;
  }

  if (!conf->IsLinked()) {
    fLog->Error("CupGeneralTCB::ConfigGADC",
                "%s[mid=%2d] is enabled but not linked", conf->GetName(), mid);
    return false;
  }

  conf->PrintConf();
  fTCB->WriteRegisterGADC(conf);
  fTCB->PrintRegisterGADC(conf);

  fLog->Info("CupGeneralTCB::ConfigGADC", "%s[mid=%2d] was configured",
             conf->GetName(), mid);
  return true;
}

bool CupGeneralTCB::ConfigSADC(SADCTConf * conf)
{
  unsigned long mid = conf->MID();

  if (!conf->IsEnabled()) {
    fLog->Info("CupGeneralTCB::ConfigSADC", "%s[mid=%2d] is disabled, passed",
               conf->GetName(), mid);
    return true;
  }

  if (!conf->IsLinked()) {
    fLog->Error("CupGeneralTCB::ConfigSADC",
                "%s[mid=%2d] is enabled but not linked", conf->GetName(), mid);
    return false;
  }

  conf->PrintConf();
  fTCB->WriteRegisterSADC(conf);
  fTCB->PrintRegisterSADC(conf);

  fLog->Info("CupGeneralTCB::ConfigSADC", "%s[mid=%2d] was configured",
             conf->GetName(), mid);
  return true;
}

bool CupGeneralTCB::ConfigIADC(IADCTConf * conf)
{
  unsigned long mid = conf->MID();

  if (!conf->IsEnabled()) {
    fLog->Info("CupGeneralTCB::ConfigIADC", "%s[mid=%2d] is disabled, passed",
               conf->GetName(), mid);
    return true;
  }

  if (!conf->IsLinked()) {
    fLog->Error("CupGeneralTCB::ConfigIADC",
                "%s[mid=%2d] is enabled but not linked", conf->GetName(), mid);
    return false;
  }

  conf->PrintConf();
  fTCB->WriteRegisterIADC(conf);
  fTCB->PrintRegisterIADC(conf);

  fLog->Info("CupGeneralTCB::ConfigIADC", "%s[mid=%2d] was configured",
             conf->GetName(), mid);
  return true;
}

bool CupGeneralTCB::ConfigTCB(TCBConf * conf)
{
  conf->PrintConf();
  fTCB->WriteRegisterTCB(conf);
  fTCB->PrintRegisterTCB(conf);

  fLog->Info("CupGeneralTCB::ConfigTCB", "TCB[sid=%d] was configured",
             conf->SID());

  return true;
}

int CupGeneralTCB::CheckLinkStatus()
{
  int nlinked = 0;
  int nport = 0;

  unsigned long linkedMID[40];
  int linked[40];

  unsigned long data[2];
  fTCB->ReadLNSTAT(data);

  if (fTCBType == TCB::MINI) {
    nport = 4;
    for (int i = 0; i < nport; i++) {
      linked[i] = (data[0] >> i) & 0x01;
    }
  }
  else {
    nport = 40;
    for (int i = 0; i < 32; i++) {
      linked[i] = (data[0] >> i) & 0x01;
    }
    for (int i = 0; i < 8; i++) {
      linked[i + 32] = (data[1] >> i) & 0x01;
    }
  }

  // get mid
  fTCB->ReadMIDS(linkedMID);

  for (int i = 0; i < nport; i++) {
    AbsConf * conf = nullptr;
    if (linked[i]) {
      // Cosine uses only FADC and SADC
      if (linkedMID[i] <= 40) {
        conf = fConfigs->FindConfig(ADC::FADCT, linkedMID[i]);
      }
      else {
        conf = fConfigs->FindConfig(ADC::SADCT, linkedMID[i]);
      }

      if (conf) {
        conf->SetLink();
        if (conf->IsEnabled()) {
          fLog->Info("CupGeneralTCB::CheckLinkStatus", "%s[mid=%2d] found @ %d",
                     conf->GetName(), linkedMID[i], i + 1);
          nlinked += 1;
        }
        else {
          fLog->Warning("CupGeneralTCB::CheckLinkStatus",
                        "%s[mid=%2d] found @ %d, but disabled", conf->GetName(),
                        linkedMID[i], i + 1);
        }
      }
      else {
        fLog->Info("CupGeneralTCB::CheckLinkStatus",
                   "unknown adc found @ %d, ignore", i + 1);
      }
    }
  }

  return nlinked;
}
