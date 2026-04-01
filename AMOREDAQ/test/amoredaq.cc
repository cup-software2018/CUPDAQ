#include <iostream>

#include "AMOREDAQ/AMOREDAQManager.hh"
#include "AMOREDAQ/daqopt.hh"

int main(int argc, char ** argv)
{
  if (argc < 2) {
    printusage(argv[0]);
    return 0;
  }

  daqopt option;
  option.init();
  optparse(option, argc, argv);

  AMOREDAQManager manager;
  manager.SetConfigFilename(option.config);
  manager.SetOutputFilename(option.output);
  manager.SetRunNumber(option.runnum);
  manager.SetDAQID(option.daqid);
  manager.SetDAQTime(option.daqtime);
  manager.SetNEvent(option.daqevent);
  manager.SetOutputSplitTime(option.sptime); // split output file every hour
  manager.SetVerboseLevel(option.vlevel);
  manager.SetCompressionLevel(1);
  if(option.dohist) manager.EnableHistograming();

  manager.Run();

  delete &manager;
  return 0;
}