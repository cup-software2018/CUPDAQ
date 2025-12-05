#include "AMOREDAQ/AMOREDAQManager.hh"

int main(int argc, char ** argv)
{
  AMOREDAQManager manager;

  manager.SetRunNumber(1234);
  manager.SetDAQID(0);
  manager.SetConfigFile("");

  manager.Run();

  return 0;
}