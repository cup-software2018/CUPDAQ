#pragma once

#include "AbsSoftTrigger.hh"

// Software trigger for the plastic scintillator muon detector (IADC waveform
// mode). Events with no trigger-bit channel are rejected; with ZSUTBit on,
// channels whose trigger bit is 0 are also marked zero so the writer drops
// them. Random trigger events (trigger type 1) keep all channels.
class PSMDTrigger : public AbsSoftTrigger {
public:
  PSMDTrigger();
  ~PSMDTrigger() override = default;

  void DoConfig(AbsConfList * configs) override;
  void InitTrigger() override;
  bool DoTrigger(BuiltEvent * event) override;

private:
  bool fDoZSUTBit{false};
};
