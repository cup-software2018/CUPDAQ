#include "AbsSoftTrigger.hh"
#include "ELog.hh"
#include "adcconsts.hh"

AbsSoftTrigger::AbsSoftTrigger(const char * name)
  : fName(name)
{
}

void AbsSoftTrigger::ResetCounters()
{
  fTotalInputEvent = 0;
  fNTriggeredEvent = 0;
  fFirstTrgTime = 0;
  fLastTrgTime = 0;
}

void AbsSoftTrigger::CountInput(const BuiltEvent * event)
{
  unsigned long ttime = event->GetTriggerTime();
  if (fTotalInputEvent == 0) fFirstTrgTime = ttime;
  fLastTrgTime = ttime;
  fTotalInputEvent++;
}

double AbsSoftTrigger::GetElapsedTime() const
{
  if (fLastTrgTime <= fFirstTrgTime) return 0.0;
  return static_cast<double>(fLastTrgTime - fFirstTrgTime) / kDONESECOND;
}

void AbsSoftTrigger::PrintReport() const
{
  double eff = (fTotalInputEvent > 0) ? 100.0 * fNTriggeredEvent / fTotalInputEvent : 0.0;

  char buf[128];
  std::string report = "\n";
  report += "========= SoftTrigger Report [" + fName + "] =========\n";
  snprintf(buf, sizeof(buf), "  %25s: %d\n", "total input", fTotalInputEvent);
  report += buf;
  snprintf(buf, sizeof(buf), "  %25s: %d\n", "triggered", fNTriggeredEvent);
  report += buf;
  snprintf(buf, sizeof(buf), "  %25s: %.2f%%\n", "efficiency", eff);
  report += buf;

  double elapsed = GetElapsedTime();
  if (elapsed > 0) {
    snprintf(buf, sizeof(buf), "  %25s: %.2f Hz\n", "triggered rate", fNTriggeredEvent / elapsed);
    report += buf;
  }
  report += "=====================================================\n";

  INFO("%s", report.c_str());
}
