#include <algorithm>
#include <cstdint>
#include <yaml-cpp/yaml.h>

#include "ELog.hh"
#include "FADCRawChannel.hh"
#include "FADCRawEvent.hh"
#include "IADCTConf.hh"
#include "PSMDTrigger.hh"

namespace
{
constexpr int kIADCSamplePeriod = 16; // ns per waveform sample, PSW is in ns
constexpr int kNCHPerDet = 4;         // channels per detector (TLT group)
constexpr int kNCHPerSlab = 2;        // PMTs per slab, a detector has two slabs

// true at the first sum of (adc - ped) over w consecutive samples above thr, sliding one sample
// at a time
bool IsPeakSumOver(const unsigned short * adc, int ndp, int ped, int w, int thr)
{
  w = std::min(w, ndp);

  int sum = 0;
  for (int k = 0; k < w; k++) {
    sum += adc[k] - ped;
  }
  if (sum > thr) return true;

  for (int k = w; k < ndp; k++) {
    sum += adc[k] - adc[k - w];
    if (sum > thr) return true;
  }
  return false;
}
} // namespace

PSMDTrigger::PSMDTrigger()
  : AbsSoftTrigger("PSMDTrigger")
{
}

void PSMDTrigger::DoConfig(AbsConfList * configs)
{
  fConfigs = configs;

  YAML::Node node = configs->GetYAMLNode("PSMDTrigger");
  if (!node) return;

  if (node["Enabled"] && node["Enabled"].as<int>()) SetEnable();
  if (node["ZSUTBit"]) fDoZSUTBit = node["ZSUTBit"].as<int>() != 0;
  if (node["PeakSum"]) fDoPeakSum = node["PeakSum"].as<int>() != 0;
  if (node["ThrScale"]) fThrScale = node["ThrScale"].as<int>();
  if (node["MinSlab"]) fMinSlab = node["MinSlab"].as<int>();

  fDeadMask.clear();
  for (const auto & dead : node["DeadCh"]) {
    int sid = dead["SID"].as<int>();
    int det = dead["DET"].as<int>();
    int ch = dead["CH"].as<int>();
    fDeadMask[sid] |= uint64_t{1} << ((det - 1) * kNCHPerDet + ch - 1);
  }

  INFO("PSMDTrigger: enabled=%d, ZSUTBit=%d, PeakSum=%d, ThrScale=%d, MinSlab=%d", IsEnabled(),
       fDoZSUTBit, fDoPeakSum, fThrScale, fMinSlab);
  for (const auto & [sid, mask] : fDeadMask) {
    INFO("PSMDTrigger: dead channel mask 0x%llX for SID=%d", static_cast<unsigned long long>(mask),
         sid);
  }
}

void PSMDTrigger::InitTrigger()
{
  fTotalInputEvent = 0;
  fNTriggeredEvent = 0;
}

bool PSMDTrigger::DoTrigger(BuiltEvent * event)
{
  fTotalInputEvent++;

  // random trigger: keep all channels
  if (event->GetTriggerType() == 1) {
    fNTriggeredEvent++;
    return true;
  }

  int nfired = 0;
  int nslab = 0;
  int nadc = event->GetEntries();
  for (int k = 0; k < nadc; k++) {
    auto * adcraw = static_cast<FADCRawEvent *>(event->At(k));
    auto * header = adcraw->GetADCHeader();

    // without its config the board falls back to the trigger bits alone
    IADCTConf * conf = nullptr;
    if (fDoPeakSum && fConfigs) {
      conf = static_cast<IADCTConf *>(fConfigs->FindConfig(ADC::IADCT, header->GetMID()));
    }
    int width = conf ? std::max(1, conf->PSW() / kIADCSamplePeriod) : 0;

    int nch = adcraw->GetNCH();
    uint64_t fmask = 0; // fired channels, bit i = channel i
    for (int i = 0; i < nch; i++) {
      if (header->GetZero(i)) continue;

      bool fired = header->GetTriggerBit(i);
      if (fired && conf) {
        auto * channel = adcraw->GetChannel(i);
        fired = IsPeakSumOver(channel->GetADC(), channel->GetNDP(),
                              static_cast<int>(header->GetPedestal(i)), width,
                              fThrScale * conf->THR(i));
      }

      if (fired) {
        nfired++;
        fmask |= uint64_t{1} << i;
      }
      else if (fDoZSUTBit) {
        header->SetZero(i);
      }
    }

    // a slab fires when all its live channels fired
    auto it = fDeadMask.find(header->GetMID());
    uint64_t dead = it == fDeadMask.end() ? 0 : it->second;
    for (int i = 0; i + kNCHPerSlab <= nch; i += kNCHPerSlab) {
      uint64_t live = (uint64_t{(1u << kNCHPerSlab) - 1} << i) & ~dead;
      if (live && (fmask & live) == live) nslab++;
    }
  }

  if (fMinSlab > 0 ? nslab < fMinSlab : nfired == 0) return false;

  fNTriggeredEvent++;
  return true;
}
