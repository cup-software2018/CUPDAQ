#include <algorithm>
#include <chrono>
#include <iostream>
#include <thread>

#include "DAQ/CupGeneralTCB.hh"
#include "DAQUtils/ELog.hh"
#include "OnlConsts/onlconsts.hh"

using namespace std;

ClassImp(CupGeneralTCB)

CupGeneralTCB::CupGeneralTCB()
  : TObject(),
    fTCB(nullptr),
    fTCBConfig(nullptr),
    fConfigs(nullptr),
    fTCBType(TCB::V1),
    fADCType(ADC::NONE),
    fExperiment(DAQ::EXPERIMENT::NONE)
{
}

int CupGeneralTCB::Open()
{
  fTCBConfig = static_cast<TCBConf *>(fConfigs->GetTCBConfig());
  fTCBType = fTCBConfig->TCBTYPE();

  if (fTCBType == TCB::MINI) {
    fTCB = new CupMiniTCB();
    static_cast<CupMiniTCB *>(fTCB)->SetIPAddress("192.168.0.2");
    INFO("open with MiniTCB");
  }
  else {
    fTCB = CupTCB::Instance();
    INFO("open with TCB");
  }

  int state = fTCB->Open();
  if (state != 0) { ERROR("tcb open failed, check power or link status"); }
  else {
    INFO("tcb opened");
  }
  return state;
}

void CupGeneralTCB::Close()
{
  int nadc = fConfigs->GetNADC(ADC::IADCT, true);
  if (nadc > 0) {
    for (int i = 0; i < nadc; i++) {
      auto * config = fConfigs->GetConfig(ADC::IADCT, i);
      for (int j = 0; j < 5; j++) {
        fTCB->WriteHV(config->MID(), j + 1, 0.0);
      }
    }
    INFO("turned off all SIPM HVs");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  fTCB->Close();
  INFO("tcb closed");
}

bool CupGeneralTCB::Config()
{
  INFO("module configuration start");

  int nconf = fConfigs->GetEntries();

  if (nconf == 0) {
    ERROR("no module in config list");
    return false;
  }

  int nlinked = CheckLinkStatus();
  if (nlinked == 0) {
    ERROR("no module linked");
    return false;
  }

  for (int i = 0; i < nconf; i++) {
    auto * conf = static_cast<AbsConf *>(fConfigs->At(i));
    if (!conf->IsEnabled() || !conf->IsLinked()) continue;

    const char * name_cstr = conf->GetName();
    std::string_view name(name_cstr);

    if (name.find("TCB") != std::string_view::npos) continue;

    uint32_t mid = conf->MID();

    fTCB->WriteDRAMON(mid, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    uint32_t dramon = fTCB->ReadDRAMON(mid);

    if (dramon) { INFO("%s[mid=%2u] DRAM on", name_cstr, mid); }
    else {
      ERROR("%s[mid=%2u] error occurred during turning DRAM on", name_cstr, mid);
      return false;
    }

    if (name.find("AMOREADC") != std::string_view::npos) {
      int bcount = fTCB->ReadBCount(mid);
      if (bcount) {
        std::unique_ptr<unsigned char[]> data(new unsigned char[bcount * kKILOBYTES]);
        fTCB->ReadData(mid, bcount, data.get());
      }
    }

    if (name.find("SADC") == std::string_view::npos) { fTCB->AlignDRAM(mid); }
  }

  fTCB->Reset();

  bool retval = true;
  for (int i = 0; i < nconf; i++) {
    auto * conf = static_cast<AbsConf *>(fConfigs->At(i));
    std::string_view name = conf->GetName();

    if (name == "STRG" || name == "DAQ") continue;

    if (name == "TCB") { retval &= ConfigTCB(static_cast<TCBConf *>(conf)); }
    else if (name == "FADCT") {
      retval &= ConfigFADC(static_cast<FADCTConf *>(conf));
    }
    else if (name == "SADCT") {
      retval &= ConfigSADC(static_cast<SADCTConf *>(conf));
    }
    else if (name == "IADCT") {
      retval &= ConfigIADC(static_cast<IADCTConf *>(conf));
    }
    else {
      WARNING("unknown kind of module : %s", conf->GetName());
    }
  }

  if (retval) { INFO("all modules configuration done"); }

  return retval;
}

bool CupGeneralTCB::StartTrigger()
{
  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  int nadc = fConfigs->GetNADC(ADC::FADCT, true);
  if (nadc > 0) {
    INFO("Measuring FADC pedestals");
    cout << endl;
    cout << "+++++++++++ FADC PEDESTALS ++++++++++++" << endl;
    for (int i = 0; i < nadc; i++) {
      auto * conf = static_cast<FADCTConf *>(fConfigs->GetConfig(ADC::FADCT, i));
      if (conf->IsLinked() && conf->IsEnabled()) { fTCB->MeasurePedestalFADC(conf); }
    }
    cout << "+++++++++++ FADC PEDESTALS ++++++++++++" << endl;
    cout << endl;
  }

  nadc = fConfigs->GetNADC(ADC::SADCT, true);
  if (nadc > 0) {
    INFO("Measuring SADC pedestals");
    cout << endl;
    cout << "+++++++++++ SADC PEDESTALS ++++++++++++" << endl;
    for (int i = 0; i < nadc; i++) {
      auto * conf = static_cast<SADCTConf *>(fConfigs->GetConfig(ADC::SADCT, i));
      if (conf->IsLinked() && conf->IsEnabled()) { fTCB->MeasurePedestalSADC(conf); }
    }
    cout << "+++++++++++ SADC PEDESTALS ++++++++++++" << endl;
    cout << endl;
  }

  nadc = fConfigs->GetNADC(ADC::IADCT, true);
  if (nadc > 0) {
    INFO("Measuring IADC pedestals");
    cout << endl;
    cout << "+++++++++++ IADC PEDESTALS ++++++++++++" << endl;
    for (int i = 0; i < nadc; i++) {
      auto * conf = static_cast<IADCTConf *>(fConfigs->GetConfig(ADC::IADCT, i));
      if (conf->IsLinked() && conf->IsEnabled()) { fTCB->MeasurePedestalIADC(conf); }
    }
    cout << "+++++++++++ IADC PEDESTALS ++++++++++++" << endl;
    cout << endl;
  }

  INFO("all ADCs initialized");

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  fTCB->TriggerStart();
  INFO("trigger started");

  return true;
}

void CupGeneralTCB::StopTrigger()
{
  fTCB->TriggerStop();
  INFO("trigger stopped");
}

bool CupGeneralTCB::ConfigFADC(FADCTConf * conf)
{
  uint32_t mid = conf->MID();

  if (!conf->IsEnabled()) {
    INFO("%s[mid=%2lu] is disabled, passed", conf->GetName(), mid);
    return true;
  }

  if (!conf->IsLinked()) {
    ERROR("%s[mid=%2lu] is enabled but not linked", conf->GetName(), mid);
    return false;
  }

  conf->PrintConf();
  fTCB->WriteRegisterFADC(conf);
  fTCB->PrintRegisterFADC(conf);

  INFO("%s[mid=%2lu] was configured", conf->GetName(), mid);
  return true;
}

bool CupGeneralTCB::ConfigSADC(SADCTConf * conf)
{
  uint32_t mid = conf->MID();

  if (!conf->IsEnabled()) {
    INFO("%s[mid=%2lu] is disabled, passed", conf->GetName(), mid);
    return true;
  }

  if (!conf->IsLinked()) {
    ERROR("%s[mid=%2lu] is enabled but not linked", conf->GetName(), mid);
    return false;
  }

  conf->PrintConf();
  fTCB->WriteRegisterSADC(conf);
  fTCB->PrintRegisterSADC(conf);

  INFO("%s[mid=%2lu] was configured", conf->GetName(), mid);
  return true;
}

bool CupGeneralTCB::ConfigIADC(IADCTConf * conf)
{
  uint32_t mid = conf->MID();

  if (!conf->IsEnabled()) {
    INFO("%s[mid=%2lu] is disabled, passed", conf->GetName(), mid);
    return true;
  }

  if (!conf->IsLinked()) {
    ERROR("%s[mid=%2lu] is enabled but not linked", conf->GetName(), mid);
    return false;
  }

  conf->PrintConf();
  fTCB->WriteRegisterIADC(conf);
  fTCB->PrintRegisterIADC(conf);

  INFO("%s[mid=%2lu] was configured", conf->GetName(), mid);
  return true;
}

bool CupGeneralTCB::ConfigTCB(TCBConf * conf)
{
  conf->PrintConf();
  fTCB->WriteRegisterTCB(conf);
  fTCB->PrintRegisterTCB(conf);

  INFO("TCB[sid=%d] was configured", conf->SID());

  return true;
}

int CupGeneralTCB::CheckLinkStatus()
{
  int nlinked = 0;
  int nport = 0;

  uint32_t linkedMID[40];
  int linked[40];

  uint32_t data[2];
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

  fTCB->ReadMIDS(linkedMID);

  for (int i = 0; i < nport; i++) {
    if (linked[i]) {
      AbsConf * conf = nullptr;
      if (fADCType == ADC::NONE) { conf = fConfigs->FindConfig(linkedMID[i]); }
      else {
        conf = fConfigs->FindConfig(fADCType, linkedMID[i]);
      }

      if (conf) {
        conf->SetLink();
        if (conf->IsEnabled()) {
          INFO("%s[mid=%2lu] found @ %d", conf->GetName(), linkedMID[i], i + 1);
          nlinked += 1;
        }
        else {
          WARNING("%s[mid=%2lu] found @ %d, but disabled", conf->GetName(), linkedMID[i], i + 1);
        }
      }
    }
  }

  return nlinked;
}