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

void WCMDTrigger::InitTrigger()
{
  fTotalInputEvent = 0;
  fNTriggeredEvent = 0;
}

bool WCMDTrigger::DoTrigger(BuiltEvent * event)
{
  fTotalInputEvent++;
  fNTriggeredEvent++;
  return true;
}
