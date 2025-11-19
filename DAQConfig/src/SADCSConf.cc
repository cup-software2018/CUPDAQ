#include <iostream>

#include "DAQConfig/SADCSConf.hh"

ClassImp(SADCSConf)

SADCSConf::SADCSConf()
  : SADCTConf()
{
}

SADCSConf::SADCSConf(int sid)
  : SADCTConf(sid)
{
  SetNameTitle("SADCS", "Standalone sadc 64MHz");

  fPTRG = 0;
  fMTHR = 1;
  fPSC = 1;
  fADCType = ADC::SADCS;
}

void SADCSConf::PrintConf() const
{
  using std::cout;
  using std::endl;

  cout << Form(" ++ SADCS config: SID(%d) MID(%1d) NCH(%1d) CW(%d) GW(%d) "
               "PSW(%d) SUBPED(%d) PTRG(%d) MTHR(%d) PSC(%d)",
               fSID, fMID, fNCH, fCW, fGW, fPSW, fSUBPED, fPTRG, fMTHR, fPSC)
       << endl;

  if (fIsEnabled) cout << " ++ This SADCS is enabled" << endl;
  else cout << " ++ This SADCS is disabled" << endl;

  cout << " -----------------------------------------------" << endl;
  for (int j = 0; j < 4; j++) {
    if (j > 0) cout << endl;

    cout << "    CID : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++) {
      cout << Form("%6d", fCID[i]);
    }
    cout << endl;

    cout << "    PID : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++) {
      cout << Form("%6d", fPID[i]);
    }
    cout << endl;

    cout << "    THR : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++) {
      cout << Form("%6d", fTHR[i]);
    }
    cout << endl;

    cout << "    DLY : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++) {
      cout << Form("%6d", fDLY[i]);
    }
    cout << endl;
  }
  cout << " -----------------------------------------------" << endl;
  cout << endl;
}
