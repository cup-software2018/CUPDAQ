#include <iostream>

#include "AMOREDAQ/AMOREDAQManager.hh"

int main(int argc, char ** argv)
{
  AMOREDAQManager manager;
  manager.SetConfigFilename(argv[1]);

  manager.Run();

  return 0;
}