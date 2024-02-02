#include "TString.h"

#include "HDF5Utils/AbsH5Event.hh"

ClassImp(AbsH5Event)

AbsH5Event::AbsH5Event()
{
  fWriteTag = false;
  fCompressionLevel = 1;
  fMemSize = 0;
}

AbsH5Event::~AbsH5Event() {}
