#include <yaml-cpp/yaml.h>

#include "DAQTrigger/PSMDTrigger.hh"

PSMDTrigger::PSMDTrigger()
  : AbsSoftTrigger("PSMDTrigger")
{
}

void PSMDTrigger::DoConfig(AbsConfList * configs) 
{
  YAML::Node node = configs->GetYAMLNode("PSMDTrigger");
}

void PSMDTrigger::InitTrigger() {}

bool PSMDTrigger::DoTrigger(BuiltEvent * event)
{
  fTotalInputEvent++;
  fNTriggeredEvent++;
  return true;
}
