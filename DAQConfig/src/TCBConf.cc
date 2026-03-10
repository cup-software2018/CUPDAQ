#include <iostream>

#include "DAQConfig/TCBConf.hh"

ClassImp(TCBConf)

TCBConf::TCBConf(int sid)
  : AbsConf(sid, ADC::TCB)
{
  SetNameTitle("TCB", "Trigger Control Board");
  SetEnable();
}

void TCBConf::PrintConf() const
{
  using std::cout;
  using std::endl;

  if (fTCBTYPE == TCB::V2) {
    int f = static_cast<int>(TESTBIT(fSWF, 0));
    int sm = static_cast<int>(TESTBIT(fSWF, 1));
    int sl = static_cast<int>(TESTBIT(fSWF, 2));
    int i = static_cast<int>(TESTBIT(fSWF, 3));
    const char * swf = Form("%d %d %d %d", f, sm, sl, i);

    f = static_cast<int>(TESTBIT(fSWSM, 0));
    sm = static_cast<int>(TESTBIT(fSWSM, 1));
    sl = static_cast<int>(TESTBIT(fSWSM, 2));
    i = static_cast<int>(TESTBIT(fSWSM, 3));
    const char * swsm = Form("%d %d %d %d", f, sm, sl, i);

    f = static_cast<int>(TESTBIT(fSWSL, 0));
    sm = static_cast<int>(TESTBIT(fSWSL, 1));
    sl = static_cast<int>(TESTBIT(fSWSL, 2));
    i = static_cast<int>(TESTBIT(fSWSL, 3));
    const char * swsl = Form("%d %d %d %d", f, sm, sl, i);

    f = static_cast<int>(TESTBIT(fSWI, 0));
    sm = static_cast<int>(TESTBIT(fSWI, 1));
    sl = static_cast<int>(TESTBIT(fSWI, 2));
    i = static_cast<int>(TESTBIT(fSWI, 3));
    const char * swi = Form("%d %d %d %d", f, sm, sl, i);

    cout << Form(" ++   TCB config: SID(%d) TRGON(%d) CW(%d) DLY(%d) PTRIG(%d) \n"
                 "                  MTHRF(%d)  PSCF(%d)  DTF(%d)  TSWF(%s) \n"
                 "                  MTHRSM(%d) PSCSM(%d) DTSM(%d) TSWSM(%s) \n"
                 "                  MTHRSL(%d) PSCSL(%d) DTSL(%d) TSWSL(%s) \n"
                 "                  MTHRI(%d)  PSCI(%d)  DTI(%d)  TSWI(%s)",
                 fSID, fTM, fCW, fDLY, fPTRG, fMTHRF, fPSCF, fDTF, swf, fMTHRSM, fPSCSM, fDTSM,
                 swsm, fMTHRSL, fPSCSL, fDTSL, swsl, fMTHRI, fPSCI, fDTI, swi)
         << endl;
  }
  else {
    cout << Form(" ++   TCB config: SID(%d) TRGON(%d) CW(%d) DLY(%d) PTRIG(%d) \n"
                 "                  MTHR(%d) PSC(%d) DT(%d)",
                 fSID, fTM, fCW, fDLY, fPTRG, fMTHRF, fPSCF, fDTF)
         << endl;
  }
}
