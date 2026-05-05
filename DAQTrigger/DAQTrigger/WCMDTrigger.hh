#pragma once

#include "DAQTrigger/AbsSoftTrigger.hh"

class WCMDTrigger : public AbsSoftTrigger {
public:
  WCMDTrigger();
  ~WCMDTrigger() = default;

  void DoConfig(AbsConfList * configs) override;
  void InitTrigger() override;
  bool DoTrigger(BuiltEvent * event) override;
};
