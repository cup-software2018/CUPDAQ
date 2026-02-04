#include "AMOREDAQ/AMOREDAQManager.hh"

int main(int argc, char ** argv)
{
  AMOREDAQManager manager;
  manager.SetDAQType(DAQ::AMORETCB);


  manager.SetRunNumber(1234);
  manager.SetDAQID(0);
  manager.SetConfigFilename("amore.config");

  manager.Run();

  return 0;
}