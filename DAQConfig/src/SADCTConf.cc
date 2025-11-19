#include <iostream>

#include "DAQConfig/SADCTConf.hh"

ClassImp(SADCTConf)

SADCTConf::SADCTConf()
  : AbsConf()
{
}

SADCTConf::SADCTConf(int sid)
  : AbsConf(sid, ADC::SADCT)
{
  SetNameTitle("SADCT", "TCB controlled sadc 64MHz");

  fNCH = kNCHSADC;
  fCW = 1024;
  fGW = 256;
  fPSW = 256;
  fSUBPED = 1;

  for (int i = 0; i < kNCHSADC; i++) {
    fCID[i] = i + 1;
    fPID[i] = 0;
    fDLY[i] = 6150;
    fTHR[i] = 50;
  }

  for (int i = 0; i < 8; i++) {
    fTLT[i] = 0xFFFE;
  }
}

void SADCTConf::PrintConf() const
{
  using std::cout;
  using std::endl;

  cout << Form(" ++ %s config: SID(%d) MID(%d) NCH(%d) CW(%d) GW(%d) PSW(%d) "
               "SUBPED(%d)",
               GetName(), fSID, fMID, fNCH, fCW, fGW, fPSW, fSUBPED)
       << endl;

  cout << Form("                  TLT1(%X) TLT2(%X) TLT3(%X) TLT4(%X)", fTLT[0], fTLT[1], fTLT[2], fTLT[3]) << endl;
  cout << Form("                  TLT5(%X) TLT6(%X) TLT7(%X) TLT8(%X)", fTLT[4], fTLT[5], fTLT[6], fTLT[7]) << endl;

  if (fIsEnabled) cout << Form(" ++ This %s is enabled", GetName()) << endl;
  else cout << Form(" ++ This %s is disabled", GetName()) << endl;

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
