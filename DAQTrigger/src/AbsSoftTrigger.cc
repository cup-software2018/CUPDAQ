#include "DAQTrigger/AbsSoftTrigger.hh"

AbsSoftTrigger::AbsSoftTrigger()
  : fIsEnabled(false),
    fVerboseLevel(0),
    fMode(),
    fADCType(ADC::NONE),
    fConfig(nullptr),
    fTotalInputEvent(0),
    fNTriggeredEvent(0)
{
}

AbsSoftTrigger::AbsSoftTrigger(AbsConf * config)
  : fIsEnabled(config->IsEnabled()),
    fVerboseLevel(0),
    fMode(),
    fADCType(config->GetADCType()),
    fConfig(config),
    fTotalInputEvent(0),
    fNTriggeredEvent(0)
{
}

