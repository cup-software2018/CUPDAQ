#include "TROOT.h"

#include "DAQ/CupDAQManager.hh"
#include "DAQ/daqopt.hh"
#include "DAQTrigger/CupSoftTrigger.hh"

int main(int argc, char ** argv)
{
  ROOT::EnableThreadSafety();

  if (argc < 2) {
    printusage(argv[0]);
    return 0;
  }

  daqopt option;
  option.init();
  optparse(option, argc, argv);

  // for TCB controlled ADC
  ADC::TYPE adctype = static_cast<ADC::TYPE>(static_cast<int>(option.adctype[0]) + 10);

  auto * DAQ = new CupDAQManager();
  DAQ->SetDAQType(DAQ::TCBDAQ);
  DAQ->SetRunNumber(option.runnum);
  DAQ->SetADCType(adctype);
  DAQ->SetTriggerMode(TRIGGER::GLOBAL);
  DAQ->SetConfigFilename(option.config);
  DAQ->SetOutputFilename(option.output);
  DAQ->SetDAQTime(option.daqtime);
  DAQ->SetNEvent(option.daqevent);
  DAQ->SetOutputSplitTime(option.sptime);
  DAQ->SetTriggerMonTime(option.rfreq);
  DAQ->SetVerboseLevel(option.vlevel);
  if (option.dohist) DAQ->EnableHistograming();

  auto * swtrigger = new CupSoftTrigger();
  swtrigger->SetVerboseLevel(option.vlevel);
  DAQ->SetSoftTrigger(swtrigger);

  DAQ->Run();
  DAQ->ClearBuffers();

  delete swtrigger;
  delete DAQ;

  return 0;
}