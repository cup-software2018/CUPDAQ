#pragma once

#include "AbsSoftTrigger.hh"

// Software trigger for the WCMD door PMT (single FADC channel with the hardware peak sum trigger).
// Keeps events with a large trigger pulse (A) or with enough summed pulse charge after the
// trigger pulse (B), so small first pulses followed by reflected light are recovered. Integer
// arithmetic on v[j] = adc[j] - header pedestal, s4(j) = v[j] + v[j+1] + v[j+2] + v[j+3]:
//   1) trigger sample tr: the hardware PST is emulated over the whole waveform; tr is the first j
//      where the sum of v over PSW/2 samples from j exceeds THR (PSW in ns, 2 ns per sample;
//      THR and PSW of the door channel in its FADC config)
//   2) A: max s4(tr+k), k = 0..PW-1, above ThrA
//   3) B: sum of s4(j-1) over the pulse peaks j in [tr+W0, tr+W1), above ThrB. A peak is
//      v[j] >= HMin && v[j] >= v[j-1] && v[j] > v[j+1].
// Random trigger events (trigger type 1) are always kept. Events without the door channel or
// without a PST crossing are kept as a fail-safe; so are all events when the door channel or its
// PSW is missing from the FADC config.
// Spec: WCMD/door_trig/SWTRIG_DOOR.md
//
// Config (all keys but Enabled are optional, defaults below; samples are 2 ns, sums in ADC):
//   WCMDDoorTrigger:
//     Enabled: 1  # 1: on, 0 or omitted: off
//     PID: 48     # PID of the door channel
//     PW: 6       # number of start samples from tr searched for the trigger-pulse size
//     ThrA: 60    # A: threshold on the trigger-pulse size (max s4)
//     W0: -6      # charge window [tr+W0, tr+W1) relative to tr, in samples ([-12, +188) ns)
//     W1: 94
//     HMin: 8     # minimum height of a pulse peak v[j], in ADC
//     ThrB: 100   # B: threshold on the summed pulse charge in the window (s4 units, ~3 p.e.)
class WCMDDoorTrigger : public AbsSoftTrigger {
public:
  enum Decision
  {
    kReject = 0,
    kPassA = 1,       // large trigger pulse
    kPassB = 2,       // summed pulse charge in the window
    kPeriodic = 3,    // random trigger, bypass
    kNoTrgSample = 4, // no PST crossing in the waveform, kept
    kNoDoorCh = 5,    // door channel not in the event or not configured, kept
    kNDecision
  };

  struct Result {
    int tr{-1}; // trigger sample, -1 if not found
    int P{0};   // trigger-pulse size (max 4-sum)
    int Q{0};   // summed pulse charge in the window
  };

  WCMDDoorTrigger();
  ~WCMDDoorTrigger() override = default;

  void DoConfig(AbsConfList * configs) override;
  void InitTrigger() override;
  bool DoTrigger(BuiltEvent * event) override;
  void PrintReport() const override;

  // decision for one door waveform with the PST of the configured door channel; fills tr, P, Q
  // when result is given
  Decision Evaluate(const unsigned short * adc, int ped, int ns, unsigned int ttype,
                    Result * result = nullptr) const;

private:
  int fPID{48};
  int fPW{6};
  int fThrA{60};
  int fW0{-6};
  int fW1{94};
  int fHMin{8};
  int fThrB{100};

  // door channel and its hardware PST, from the FADC config
  int fDoorMID{-1}; // -1: door channel not configured
  int fDoorCh{-1};
  int fPSTThr{0};
  int fPSTWidth{0}; // in samples

  long fNDecision[kNDecision]{};
};
