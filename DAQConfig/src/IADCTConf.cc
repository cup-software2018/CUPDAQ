#include <iostream>

#include "DAQConfig/IADCTConf.hh"

ClassImp(IADCTConf)

IADCTConf::IADCTConf()
  : AbsConf()
{
  SetNameTitle("IADCT", "TCB controlled flash ADC reading SiPM");
  fADCType = ADC::IADCT;  
}

IADCTConf::IADCTConf(int sid)
  : AbsConf(sid, ADC::IADCT)
{
  SetNameTitle("IADCT", "TCB controlled flash ADC reading SiPM");
}

void IADCTConf::PrintConf() const
{
  using std::cout;
  using std::endl;

  cout << Form(" ++ %s config: SID(%d) MID(%1d) NCH(%1d) MODE(%d) RL(%d) CW(%d) GW(%d) PSW(%d) DAQID(%d)",
               GetName(), fSID, fMID, fNCH, fMODE, fRL, fCW, fGW, fPSW, fDAQID)
       << endl;

  cout << Form("                  TLT1(%X) TLT2(%X) TLT3(%X) TLT4(%X) TLT5(%X)", fTLT[0], fTLT[1],
               fTLT[2], fTLT[3], fTLT[4])
       << endl;

  cout << Form("                  TLT6(%X) TLT7(%X) TLT8(%X) TLT9(%X) TLT10(%X)", fTLT[5], fTLT[6],
               fTLT[7], fTLT[8], fTLT[9])
       << endl;

  cout << Form("                  HV1(%.1f) HV2(%.1f) HV3(%.1f) HV4(%.1f) HV5(%.1f)", fHV[0],
               fHV[1], fHV[2], fHV[3], fHV[4])
       << endl;

  if (fIsEnabled) cout << Form(" ++ This %s is enabled", GetName()) << endl;
  else cout << Form(" ++ This %s is disabled", GetName()) << endl;

  cout << " -----------------------------------------------" << endl;

  for (int step = 0; step < fNCH; step += 8) {
    int end = (step + 8 > fNCH) ? fNCH : step + 8;

    auto print = [&](const char * label, const int * arr) {
      cout << Form("%8s :", label);
      for (int i = step; i < end; i++) {
        cout << Form("%6d", arr[i]);
      }
      cout << endl;
    };

    print("CID", fCID);
    print("PID", fPID);
    print("THR", fTHR);
    print("DLY", fDLY);

    if (step + 8 < fNCH) cout << endl;
  }

  cout << " -----------------------------------------------" << endl;
  cout << endl;
}
