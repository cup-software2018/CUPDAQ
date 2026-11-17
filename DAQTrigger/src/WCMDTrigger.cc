#include <yaml-cpp/yaml.h>

#include "WCMDTrigger.hh"

WCMDTrigger::WCMDTrigger()
  : AbsSoftTrigger("WCMDTrigger")
{
}

void WCMDTrigger::DoConfig(AbsConfList * configs)
{
  YAML::Node node = configs->GetYAMLNode("WCMDTrigger");
  if (!node) return;
}

void WCMDTrigger::InitTrigger() { ResetCounters(); }

bool WCMDTrigger::DoTrigger(BuiltEvent * event)
{
  CountInput(event);
  fNTriggeredEvent++;
  return true;
}
