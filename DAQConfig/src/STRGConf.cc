#include "DAQConfig/STRGConf.hh"

ClassImp(STRGConf)

STRGConf::STRGConf()
  : AbsConf()
{
  SetNameTitle("STRG", "Software Trigger");

  fZSUMode = 0;
  fPrescale = 0;
  fFilename = "";
}
