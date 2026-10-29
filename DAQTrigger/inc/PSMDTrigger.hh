#pragma once

#include "AbsSoftTrigger.hh"

// Software trigger for the plastic scintillator muon detector (IADC waveform
// mode). The hardware sets trigger bits per 4-channel group, so with PeakSum on
// every tbit=1 channel is triggered again on its own: the largest sliding sum of
// (ADC - pedestal) over PSW/16 samples must exceed ThrScale x THR of the IADCT
// config. Events with no channel that fired are rejected; with ZSUTBit on,
// channels that did not fire are also marked zero so the writer drops them.
// Random trigger events (trigger type 1) keep all channels.
class PSMDTrigger : public AbsSoftTrigger {
public:
  PSMDTrigger();
  ~PSMDTrigger() override = default;

  void DoConfig(AbsConfList * configs) override;
  void InitTrigger() override;
  bool DoTrigger(BuiltEvent * event) override;

private:
  AbsConfList * fConfigs{nullptr};

  bool fDoZSUTBit{false};
  bool fDoPeakSum{false};
  int fThrScale{1};
};
