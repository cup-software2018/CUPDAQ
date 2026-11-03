#pragma once

#include <cstdint>
#include <map>

#include "AbsSoftTrigger.hh"

// Software trigger for the plastic scintillator muon detector (IADC waveform
// mode). The hardware sets trigger bits per 4-channel group, so with PeakSum on
// every tbit=1 channel is triggered again on its own: the largest sliding sum of
// (ADC - pedestal) over PSW/16 samples must exceed ThrScale x THR of the IADCT
// config. A detector (4-channel group, DET 1-10 of a board) has two slabs of two
// PMTs (ch1-2, ch3-4); a slab fires when all its channels not listed in DeadCh
// fired. With MinSlab > 0, events with fewer than MinSlab fired slabs are
// rejected, otherwise events with no channel that fired are rejected. With
// ZSUTBit on, channels that did not fire are also marked zero so the writer
// drops them.
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
  int fMinSlab{0};
  std::map<int, uint64_t> fDeadMask; // SID -> dead channels, bit i = channel i
};
