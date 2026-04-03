#include "DAQ/CupDAQManager.hh"
#include "DAQ/daqopt.hh"
#include "DAQTrigger/CupSoftTrigger.hh"

int main(int argc, char ** argv)
{
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

  auto * swtrigger = new CupSoftTrigger();
  swtrigger->SetVerboseLevel(option.vlevel);
  DAQ->SetSoftTrigger(swtrigger);

  DAQ->Run();

  delete swtrigger;
  delete DAQ;

  return 0;
}
