#include <iostream>

#include "AMORESystem/AMORETCBConf.hh"

ClassImp(AMORETCBConf)

AMORETCBConf::AMORETCBConf(int sid)
  : AbsConf(sid, ADC::AMORETCB)
{
  SetNameTitle("AMORETCB", "Trigger Control Board for AMOREADC");
  SetEnable();
}

void AMORETCBConf::PrintConf() const
{
  std::cout << Form(" ++ AMORETCB config: SID(%d) CW(%d) DT(%d) PSC(%d)", fSID, fCW, fDT, fPSC) << std::endl;
}
