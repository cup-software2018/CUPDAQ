#include <iostream>

#include "DAQConfig/IADCTConf.hh"

ClassImp(IADCTConf)

IADCTConf::IADCTConf()
  : AbsConf()
{
}

IADCTConf::IADCTConf(int sid)
  : AbsConf(sid, ADC::IADCT)
{
  SetNameTitle("IADCT", "TCB controlled SiPM adc");

  fNCH = kNCHIADC;
  fMODE = 0;
  fRL = 8;
  fCW = 1028;
  fGW = 256;
  fPSW = 256;

  for (int i = 0; i < kNCHIADC; i++) {
    fCID[i] = i + 1;
    fPID[i] = 0;
    fDLY[i] = 6150;
    fTHR[i] = 100;
  }

  for (int i = 0; i < 10; i++) {
    fTLT[i] = 0x8000;
  }

  for (int i = 0; i < 5; i++) {
    fHV[i] = 0.0f;
  }
}

void IADCTConf::PrintConf() const
{
  using std::cout;
  using std::endl;

  cout << Form(" ++ %s config: SID(%d) MID(%1d) NCH(%1d) MODE(%d) RL(%d) CW(%d) GW(%d) PSW(%d)", GetName(), fSID, fMID,
               fNCH, fMODE, fRL, fCW, fGW, fPSW)
       << endl;

  cout << Form("                  TLT1(%X) TLT2(%X) TLT3(%X) TLT4(%X) TLT5(%X)", fTLT[0], fTLT[1], fTLT[2], fTLT[3],
               fTLT[4])
       << endl;

  cout << Form("                  TLT6(%X) TLT7(%X) TLT8(%X) TLT9(%X) TLT10(%X)", fTLT[5], fTLT[6], fTLT[7], fTLT[8],
               fTLT[9])
       << endl;

  cout << Form("                  HV1(%.1f) HV2(%.1f) HV3(%.1f) HV4(%.1f) HV5(%.1f)", fHV[0], fHV[1], fHV[2], fHV[3],
               fHV[4])
       << endl;

  if (fIsEnabled) cout << Form(" ++ This %s is enabled", GetName()) << endl;
  else cout << Form(" ++ This %s is disabled", GetName()) << endl;

  cout << " -----------------------------------------------" << endl;

  for (int j = 0; j < 4; j++) {
    if (j > 0) cout << endl;

    cout << "        CID : ";
    for (int i = 10 * j; i < 10 * (j + 1); i++) {
      cout << Form("%8d", fCID[i]);
    }
    cout << endl;

    cout << "        PID : ";
    for (int i = 10 * j; i < 10 * (j + 1); i++) {
      cout << Form("%8d", fPID[i]);
    }
    cout << endl;

    cout << "        THR : ";
    for (int i = 10 * j; i < 10 * (j + 1); i++) {
      cout << Form("%8d", fTHR[i]);
    }
    cout << endl;

    cout << "        DLY : ";
    for (int i = 10 * j; i < 10 * (j + 1); i++) {
      cout << Form("%8d", fDLY[i]);
    }
    cout << endl;
  }

  cout << " -----------------------------------------------" << endl;
  cout << endl;
}
