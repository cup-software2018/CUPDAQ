#ifndef AbsSoftTrigger_hh
#define AbsSoftTrigger_hh

#include <iostream>

#include "TString.h"

#include "DAQConfig/AbsConf.hh"
#include "DAQUtils/ELogger.hh"
#include "OnlConsts/adcconsts.hh"
#include "OnlObjs/BuiltEvent.hh"

class AbsSoftTrigger {
public:
  AbsSoftTrigger();
  AbsSoftTrigger(AbsConf * config);
  virtual ~AbsSoftTrigger();

  virtual void SetConfig(AbsConf * config);
  virtual void SetMode(ADC::MODE mode);
  virtual void SetVerboseLevel(int verbose);

  virtual void InitTrigger() = 0;
  virtual bool DoTrigger(BuiltEvent * event) = 0;

  virtual bool IsEnabled() const;
  virtual ADC::TYPE GetADCType() const;
  virtual double GetEfficiency() const;
  virtual const char * GetReport() const;
  virtual void PrintReport() const;

protected:
  ELogger * fLog;
  bool fIsEnabled;
  int fVerboseLevel;

  ADC::MODE fMode;
  ADC::TYPE fADCType;
  AbsConf * fConfig;

  int fTotalInputEvent;
  int fNTriggeredEvent;
};

inline void AbsSoftTrigger::SetConfig(AbsConf * config)
{
  if (!config) return;
  fConfig = config;
  fIsEnabled = config->IsEnabled();
  fADCType = config->GetADCType();
}

inline void AbsSoftTrigger::SetMode(ADC::MODE mode) { fMode = mode; }

inline void AbsSoftTrigger::SetVerboseLevel(int verbose)
{
  fVerboseLevel = verbose;
}

inline bool AbsSoftTrigger::IsEnabled() const { return fIsEnabled; }

inline ADC::TYPE AbsSoftTrigger::GetADCType() const { return fADCType; }

inline double AbsSoftTrigger::GetEfficiency() const
{
  return fIsEnabled ? 100. * fNTriggeredEvent / (double)fTotalInputEvent : 100.;
}

inline const char * AbsSoftTrigger::GetReport() const
{
  double eff = GetEfficiency();
  return Form("%6.2f (%d/%d)", eff, fNTriggeredEvent, fTotalInputEvent);
}

inline void AbsSoftTrigger::PrintReport() const
{
  std::cout << Form("%28s", "Software Trigger : ") << GetReport() << std::endl;
}

#endif