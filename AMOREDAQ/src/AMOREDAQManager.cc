#include <filesystem>

#include "AMOREDAQ/AMOREADC.hh"
#include "AMOREDAQ/AMOREDAQManager.hh"
#include "DAQConfig/RunConfig.hh"
#include "DAQUtils/ELog.hh"

ClassImp(AMOREDAQManager)

AMOREDAQManager::AMOREDAQManager() {}

AMOREDAQManager::~AMOREDAQManager() {}

void AMOREDAQManager::Run()
{
  if (std::filesystem::exists(kFORCEDENDRUNFILE)) { std::filesystem::remove(kFORCEDENDRUNFILE); }

  if (fConfigFilename.empty()) {
    ERROR("no config filename");
    return;
  }

  auto runConfig = std::make_unique<RunConfig>();
  if (!runConfig->ReadConfig(fConfigFilename.c_str())) {
    ERROR("error in reading config file %s", fConfigFilename.c_str());
    return;
  }
  fConfigList = runConfig->GetConfigs();
}

void AMOREDAQManager::RC_TCB() {}
void AMOREDAQManager::RC_DAQ() {}

bool AMOREDAQManager::AddADC(AbsConf * conf)
{
  if (!conf->IsEnabled()) { return true; }
  if (!conf->IsLinked()) {
    ERROR("AMOREADC[sid=%2d] enabled but not linked", conf->SID());
    return false;
  }

  auto * adc = new AMOREADC(conf);
  Add(adc);

  INFO("AMOREADC[sid=%2d] added to DAQ manager", adc->GetSID());

  return true;
}

bool AMOREDAQManager::AddADC(AbsConfList * conflist)
{
  int nadc = conflist->GetNADC(ADC::AMOREADC);
  if (nadc == 0) {
    ERROR("there is no AMOREADC");
    return false;
  }

  for (int i = 0; i < nadc; i++) {
    auto * conf = conflist->GetConfig(ADC::AMOREADC, i);
    if (conf->GetDAQID() == fDAQID) {
      if (!AddADC(conf)) { return false; }
    }
  }

  return true;
}

bool AMOREDAQManager::OpenDAQ()
{
  int nadc = GetEntries();
  for (int i = 0; i < nadc; i++) {
    auto * adc = static_cast<AbsADC *>(fCont[i]);
    if (adc->Open() != 0) { return false; }
  }

  INFO("all ADCs are opened");
  return true;
}

void AMOREDAQManager::CloseDAQ()
{
  int nadc = GetEntries();
  for (int i = 0; i < nadc; i++) {
    auto * adc = static_cast<AbsADC *>(fCont[i]);
    adc->Close();
  }

  INFO("all ADCs are closed");
}
