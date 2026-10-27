#include <yaml-cpp/yaml.h>

#include "ELog.hh"
#include "FADCRawEvent.hh"
#include "PSMDTrigger.hh"

PSMDTrigger::PSMDTrigger()
  : AbsSoftTrigger("PSMDTrigger")
{
}

void PSMDTrigger::DoConfig(AbsConfList * configs)
{
  YAML::Node node = configs->GetYAMLNode("PSMDTrigger");
  if (!node) return;

  if (node["Enabled"] && node["Enabled"].as<int>()) SetEnable();
  if (node["ZSUTBit"]) fDoZSUTBit = node["ZSUTBit"].as<int>() != 0;

  INFO("PSMDTrigger: enabled=%d, ZSUTBit=%d", IsEnabled(), fDoZSUTBit);
}

void PSMDTrigger::InitTrigger()
{
  fTotalInputEvent = 0;
  fNTriggeredEvent = 0;
}

bool PSMDTrigger::DoTrigger(BuiltEvent * event)
{
  fTotalInputEvent++;

  // random trigger: keep all channels
  if (event->GetTriggerType() == 1) {
    fNTriggeredEvent++;
    return true;
  }

  int nalive = 0;
  int nadc = event->GetEntries();
  for (int k = 0; k < nadc; k++) {
    auto * adcraw = static_cast<FADCRawEvent *>(event->At(k));
    auto * header = adcraw->GetADCHeader();
    int nch = adcraw->GetNCH();
    for (int i = 0; i < nch; i++) {
      if (header->GetZero(i)) continue;

      if (header->GetTriggerBit(i)) { nalive++; }
      else if (fDoZSUTBit) {
        header->SetZero(i);
      }
    }
  }

  if (nalive == 0) return false;

  fNTriggeredEvent++;
  return true;
}
