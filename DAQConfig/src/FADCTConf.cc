#include <iostream>

#include "DAQConfig/FADCTConf.hh"

ClassImp(FADCTConf)

FADCTConf::FADCTConf()
  : AbsConf(0, ADC::FADCT)
{
  SetNameTitle("FADCT", "TCB controlled flash ADC 500MHz");
}

FADCTConf::FADCTConf(int sid)
  : AbsConf(sid, ADC::FADCT)
{
  SetNameTitle("FADCT", "TCB controlled flash ADC 500MHz");
}

void FADCTConf::PrintConf() const
{
  using std::cout;
  using std::endl;

  cout << Form(" ++ FADC config: SID(%d) MID(%1d) NCH(%1d) RL(%d) "
               "TLT(%X) DSR(%d) DAQID(%d)",
               fSID, fMID, fNCH, fRL, fTLT, fDSR, fDAQID)
       << endl;

  if (fIsEnabled) cout << Form(" ++ This %s is enabled", GetName()) << endl;
  else cout << Form(" ++ This %s is disabled", GetName()) << endl;

  cout << " -----------------------------------------------" << endl;

  auto print = [&](const char * label, const int * arr) {
    cout << Form("%11s :", label);
    for (int i = 0; i < fNCH; i++) {
      cout << Form("%8d", arr[i]);
    }
    cout << endl;
  };

  print("CID", fCID);
  print("PID", fPID);
  print("POL", fPOL);
  print("DACOFF", fDACOFF);
  print("DLY", fDLY);
  print("DTIME", fDT);
  print("CW", fCW);
  print("TM", fTM);
  print("THR", fTHR);
  print("PCT", fPCT);
  print("PCI", fPCI);
  print("PWT", fPWT);
  print("PSW", fPSW);

  cout << " -----------------------------------------------" << endl;
  cout << endl;
}
