#include "DAQTrigger/AbsSoftTrigger.hh"

#include "DAQUtils/ELog.hh"

AbsSoftTrigger::AbsSoftTrigger(const char * name)
  : fName(name)
{
}

void AbsSoftTrigger::PrintReport() const
{
  double eff = (fTotalInputEvent > 0)
               ? 100.0 * fNTriggeredEvent / fTotalInputEvent
               : 0.0;

  char buf[128];
  std::string report = "\n";
  report += "========= SoftTrigger Report [" + fName + "] =========\n";
  snprintf(buf, sizeof(buf), "  %25s: %d\n",      "total input",  fTotalInputEvent);
  report += buf;
  snprintf(buf, sizeof(buf), "  %25s: %d\n",      "triggered",    fNTriggeredEvent);
  report += buf;
  snprintf(buf, sizeof(buf), "  %25s: %.2f%%\n",  "efficiency",   eff);
  report += buf;
  report += "=====================================================\n";

  INFO("%s", report.c_str());
}
