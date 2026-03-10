#include "DAQConfig/MADCSConf.hh"

ClassImp(MADCSConf)

MADCSConf::MADCSConf()
  : FADCSConf()
{
  SetNameTitle("MADCS", "Standalone flash ADC 125MHz");
  fADCType = ADC::MADCS;

  SetLink();
}

MADCSConf::MADCSConf(int sid)
  : FADCSConf(sid)
{
  SetNameTitle("MADCS", "Standalone flash ADC 125MHz");
  fADCType = ADC::MADCS;

  SetLink();
}