#pragma once

#include "DAQTrigger/AbsSoftTrigger.hh"

class PSMDTrigger : public AbsSoftTrigger {
public:
  PSMDTrigger();
  ~PSMDTrigger() = default;

  void DoConfig(AbsConfList * configs) override;
  void InitTrigger() override;
  bool DoTrigger(BuiltEvent * event) override;
};
