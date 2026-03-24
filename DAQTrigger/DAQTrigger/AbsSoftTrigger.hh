#pragma once

#include <cstdio>
#include <iomanip>
#include <iostream>
#include <string>

#include "DAQConfig/AbsConf.hh"
#include "DAQUtils/ELog.hh"
#include "OnlConsts/adcconsts.hh"
#include "OnlObjs/BuiltEvent.hh"

class AbsSoftTrigger {
public:
  AbsSoftTrigger();
  explicit AbsSoftTrigger(AbsConf * config);
  virtual ~AbsSoftTrigger() = default;

  virtual void SetConfig(AbsConf * config);
  virtual void SetMode(ADC::MODE mode);
  virtual void SetVerboseLevel(int verbose);

  virtual void InitTrigger() = 0;
  virtual bool DoTrigger(BuiltEvent * event) = 0;

  virtual bool IsEnabled() const;
  virtual ADC::TYPE GetADCType() const;
  virtual double GetEfficiency() const;
  virtual std::string GetReport() const;
  virtual void PrintReport() const;

protected:
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

inline void AbsSoftTrigger::SetVerboseLevel(int verbose) { fVerboseLevel = verbose; }

inline bool AbsSoftTrigger::IsEnabled() const { return fIsEnabled; }

inline ADC::TYPE AbsSoftTrigger::GetADCType() const { return fADCType; }

inline double AbsSoftTrigger::GetEfficiency() const
{
  return fIsEnabled ? 100.0 * fNTriggeredEvent / static_cast<double>(fTotalInputEvent) : 100.0;
}

inline std::string AbsSoftTrigger::GetReport() const
{
  char buf[128];
  std::snprintf(buf, sizeof(buf), "%6.2f (%d/%d)", GetEfficiency(), fNTriggeredEvent,
                fTotalInputEvent);
  return std::string(buf);
}

inline void AbsSoftTrigger::PrintReport() const
{
  std::cout << std::setw(28) << "Software Trigger : " << GetReport() << '\n';
}
