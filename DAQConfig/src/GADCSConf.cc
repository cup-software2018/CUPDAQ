#include "DAQConfig/GADCSConf.hh"

ClassImp(GADCSConf)

GADCSConf::GADCSConf()
  : FADCSConf()
{
  SetNameTitle("GADCS", "Standalone flash ADC 125MHz");
  fADCType = ADC::GADCS;

  SetLink();
}

GADCSConf::GADCSConf(int sid)
  : FADCSConf(sid)
{
  SetNameTitle("GADCS", "Standalone flash ADC 125MHz");
  fADCType = ADC::GADCS;

  SetLink();
}