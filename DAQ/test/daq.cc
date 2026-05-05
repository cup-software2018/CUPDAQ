#include "TROOT.h"

#include "DAQ/CupDAQManager.hh"
#include "DAQ/daqopt.hh"

int main(int argc, char ** argv)
{
  if (argc < 2) {
    printusage(argv[0]);
    return 0;
  }

  ROOT::EnableThreadSafety();

  daqopt option;
  option.init();
  optparse(option, argc, argv);

  // for TCB controlled ADC
  ADC::TYPE adctype = static_cast<ADC::TYPE>(static_cast<int>(option.adctype[0]) + 10);

  auto * DAQ = new CupDAQManager();
  DAQ->SetDAQType(DAQ::TCBCTRL);
  DAQ->SetRunNumber(option.runnum);
  DAQ->SetADCType(adctype);
  DAQ->SetDAQID(option.daqid);
  DAQ->SetTriggerMode(TRIGGER::GLOBAL);
  DAQ->SetConfigFilename(option.config);
  DAQ->SetOutputFileFormat(option.format);
  DAQ->SetTriggerMonTime(option.rfreq);
  DAQ->SetVerboseLevel(option.vlevel);
  if (option.dosend) DAQ->UseEventMerger();
  if (option.dohist) DAQ->EnableHistograming();

  // To apply a software trigger, subclass AbsSoftTrigger and implement:
  //   DoConfig(AbsConfList *)  -- read parameters from the config list
  //   InitTrigger()            -- called once before the run starts
  //   DoTrigger(BuiltEvent *)  -- return true to accept, false to reject
  // Then register it here:
  //   auto * swtrigger = new YourTrigger();
  //   DAQ->SetSoftTrigger(swtrigger);

  DAQ->Run();

  delete DAQ;

  return 0;
}
