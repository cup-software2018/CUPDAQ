#include <algorithm>
#include <string>
#include <yaml-cpp/yaml.h>

#include "ELog.hh"
#include "FADCRawChannel.hh"
#include "FADCRawEvent.hh"
#include "FADCTConf.hh"
#include "WCMDDoorTrigger.hh"

namespace
{
constexpr int kFADCSamplePeriod = 2; // ns per waveform sample, PSW is in ns

const char * kDecisionName[WCMDDoorTrigger::kNDecision] = {"reject",           "A (trigger pulse)",
                                                           "B (pulse charge)", "pedestal",
                                                           "no PST crossing",  "no door channel"};
} // namespace

WCMDDoorTrigger::WCMDDoorTrigger()
  : AbsSoftTrigger("WCMDDoorTrigger")
{
}

void WCMDDoorTrigger::DoConfig(AbsConfList * configs)
{
  fIsEnabled = false; // disabled unless WCMDDoorTrigger.Enabled is set

  YAML::Node node = configs->GetYAMLNode("WCMDDoorTrigger");
  if (!node) return;

  if (node["Enabled"] && node["Enabled"].as<int>()) SetEnable();
  if (node["PID"]) fPID = node["PID"].as<int>();
  if (node["PW"]) fPW = node["PW"].as<int>();
  if (node["ThrA"]) fThrA = node["ThrA"].as<int>();
  if (node["W0"]) fW0 = node["W0"].as<int>();
  if (node["W1"]) fW1 = node["W1"].as<int>();
  if (node["HMin"]) fHMin = node["HMin"].as<int>();
  if (node["ThrB"]) fThrB = node["ThrB"].as<int>();

  INFO("WCMDDoorTrigger: enabled=%d, PID=%d, PW=%d, ThrA=%d, W=[%d, %d), HMin=%d, ThrB=%d",
       IsEnabled(), fPID, fPW, fThrA, fW0, fW1, fHMin, fThrB);

  // door channel and its hardware PST
  fDoorMID = -1;
  fDoorCh = -1;
  for (auto * obj : *configs) {
    auto * conf = dynamic_cast<FADCTConf *>(obj);
    if (!conf) continue;
    for (int i = 0; i < conf->NCH(); i++) {
      if (conf->PID(i) != fPID) continue;
      fDoorMID = conf->MID();
      fDoorCh = i;
      fPSTThr = conf->THR(i);
      fPSTWidth = conf->PSW(i) / kFADCSamplePeriod;
      break;
    }
    if (fDoorMID >= 0) break;
  }

  if (fDoorMID < 0) {
    if (IsEnabled()) {
      ERROR("WCMDDoorTrigger: no FADC channel with PID=%d, keeping all events", fPID);
    }
    return;
  }
  if (fPSTWidth < 1) {
    if (IsEnabled()) {
      ERROR("WCMDDoorTrigger: PSW of the door channel (PID=%d) not set, keeping all events", fPID);
    }
    fDoorMID = -1;
    return;
  }
  INFO("WCMDDoorTrigger: door channel MID=%d, ch=%d, PST THR=%d over %d samples", fDoorMID,
       fDoorCh + 1, fPSTThr, fPSTWidth);
}

void WCMDDoorTrigger::InitTrigger()
{
  ResetCounters();
  std::fill(std::begin(fNDecision), std::end(fNDecision), 0L);
}

bool WCMDDoorTrigger::DoTrigger(BuiltEvent * event)
{
  CountInput(event);

  unsigned int ttype = event->GetTriggerType();
  Decision decision = ttype == 1 ? kPedestal : kNoDoorCh;

  int nadc = event->GetEntries();
  for (int k = 0; decision == kNoDoorCh && fDoorMID >= 0 && k < nadc; k++) {
    auto * adcraw = static_cast<FADCRawEvent *>(event->At(k));
    auto * header = adcraw->GetADCHeader();
    if (header->GetMID() != fDoorMID || fDoorCh >= adcraw->GetNCH() || header->GetZero(fDoorCh)) {
      continue;
    }

    auto * channel = adcraw->GetChannel(fDoorCh);
    decision = Evaluate(channel->GetADC(), static_cast<int>(header->GetPedestal(fDoorCh)),
                        channel->GetNDP(), ttype);
  }

  fNDecision[decision]++;
  if (decision == kReject) return false;

  fNTriggeredEvent++;
  return true;
}

WCMDDoorTrigger::Decision WCMDDoorTrigger::Evaluate(const unsigned short * adc, int ped, int ns,
                                                    unsigned int ttype, Result * result) const
{
  if (ttype == 1) return kPedestal;
  if (fDoorMID < 0) return kNoDoorCh;
  if (result) *result = Result{};
  if (ns < 4 || ns < fPSTWidth) return kNoTrgSample;

  auto v = [&](int j) {
    return static_cast<int>(adc[j]) - ped;
  };
  auto s4 = [&](int j) {
    return v(j) + v(j + 1) + v(j + 2) + v(j + 3);
  };

  // 1) trigger sample: PST emulation, first sum over PSTWidth samples above THR, sliding one
  // sample at a time over the whole waveform
  int tr = -1;
  int sum = 0;
  for (int j = 0; j < fPSTWidth; j++) {
    sum += v(j);
  }
  for (int j = 0; j + fPSTWidth <= ns; j++) {
    if (j > 0) sum += v(j + fPSTWidth - 1) - v(j - 1);
    if (sum > fPSTThr) {
      tr = j;
      break;
    }
  }
  if (tr < 0) return kNoTrgSample;
  if (result) result->tr = tr;

  // 2) A: trigger-pulse size, largest 4-sum of the PW start samples from tr
  int P = s4(std::min(tr, ns - 4));
  for (int k = 1; k < fPW; k++) {
    P = std::max(P, s4(std::min(tr + k, ns - 4)));
  }
  if (result) result->P = P;
  if (P > fThrA) return kPassA;

  // 3) B: summed 4-sum charge of the pulse peaks in [tr+W0, tr+W1), trigger pulse included.
  // A peak is a local maximum; on a plateau only its last sample counts.
  int Q = 0;
  const int j0 = std::max(tr + fW0, 1);
  const int j1 = std::min(tr + fW1, ns - 3);
  for (int j = j0; j < j1; j++) {
    int vj = v(j);
    if (vj >= fHMin && vj >= v(j - 1) && vj > v(j + 1)) Q += s4(j - 1);
  }
  if (result) result->Q = Q;
  if (Q > fThrB) return kPassB;

  return kReject;
}

void WCMDDoorTrigger::PrintReport() const
{
  AbsSoftTrigger::PrintReport();

  char buf[128];
  std::string report = "\n";
  report += "========= SoftTrigger Decisions [" + fName + "] =========\n";
  for (int i = 0; i < kNDecision; i++) {
    snprintf(buf, sizeof(buf), "  %25s: %ld\n", kDecisionName[i], fNDecision[i]);
    report += buf;
  }
  report += "=====================================================\n";

  INFO("%s", report.c_str());
}
