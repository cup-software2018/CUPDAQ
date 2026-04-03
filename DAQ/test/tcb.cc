#include "DAQ/CupDAQManager.hh"
#include "DAQ/daqopt.hh"

int main(int argc, char ** argv)
{
  if (argc < 2) return 0;

  daqopt option;
  option.init();
  optparse(option, argc, argv);

  // for TCB controlled ADC
  auto * DAQ = new CupDAQManager();
  DAQ->SetDAQType(DAQ::TCB);
  DAQ->SetRunNumber(option.runnum);
  DAQ->SetConfigFilename(option.config);
  DAQ->SetOutputSplitTime(option.sptime);
  DAQ->Run();

  delete DAQ;
  return 0;
}