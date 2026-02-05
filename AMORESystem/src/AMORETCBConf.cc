#include <iostream>

#include "AMORESystem/AMORETCBConf.hh"

ClassImp(AMORETCBConf)

AMORETCBConf::AMORETCBConf(int sid)
  : AbsConf(sid, ADC::TCB)
{
  SetNameTitle("AMORETCB", "Trigger Control Board for AMOREADC");
  SetEnable();
}

void AMORETCBConf::PrintConf() const
{
  std::cout << Form(" ++ AMORETCB config: SID(%d) CW(%d) DT(%d) SCALE(%d)", fSID, fCW, fDT, fSCALE) << std::endl;
}
