/*
 *
 *  Module:  IADCTConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: TCB controlled SiPM 64MHz ADC configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/07/18 12:25:11 $
 *  CVS/RCS Revision: $Revision: 1.3 $
 *  Status:           $State: Exp $
 *
 */

#include <iostream>

#include "DAQConfig/IADCTConf.hh"

using namespace std;

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
    fHV[i] = 0;
  }
}

IADCTConf::~IADCTConf() {}

void IADCTConf::PrintConf() const
{
  cout << Form(" ++ %s config: SID(%d) MID(%1d) NCH(%1d) MODE(%d) RL(%d) "
               "CW(%d) GW(%d) PSW(%d)",
               GetName(), fSID, fMID, fNCH, fMODE, fRL, fCW, fGW, fPSW)
       << endl;
  cout << Form("                  TLT1(%X) TLT2(%X) TLT3(%X) TLT4(%X) TLT5(%X)",
               fTLT[0], fTLT[1], fTLT[2], fTLT[3], fTLT[4])
       << endl;
  cout << Form(
              "                  TLT6(%X) TLT7(%X) TLT8(%X) TLT9(%X) TLT10(%X)",
              fTLT[5], fTLT[6], fTLT[7], fTLT[8], fTLT[9])
       << endl;
  cout << Form("                  HV1(%.1f) HV2(%.1f) HV3(%.1f) HV4(%.1f) "
               "HV5(%.1f)",
               fHV[0], fHV[1], fHV[2], fHV[3], fHV[4])
       << endl;
  if (fIsEnabled) cout << Form(" ++ This %s is enabled", GetName()) << endl;
  else cout << Form(" ++ This %s is disabled", GetName()) << endl;

  cout << " -----------------------------------------------" << endl;
  for (int j = 0; j < 4; j++) {
    if (j > 0) cout << endl;
    cout << "        CID : ";
    for (int i = 10 * j; i < 10 * (j + 1); i++)
      cout << Form("%8d", fCID[i]);
    cout << endl;
    cout << "        PID : ";
    for (int i = 10 * j; i < 10 * (j + 1); i++)
      cout << Form("%8d", fPID[i]);
    cout << endl;
    cout << "        THR : ";
    for (int i = 10 * j; i < 10 * (j + 1); i++)
      cout << Form("%8d", fTHR[i]);
    cout << endl;
    cout << "        DLY : ";
    for (int i = 10 * j; i < 10 * (j + 1); i++)
      cout << Form("%8d", fDLY[i]);
    cout << endl;
  }
  cout << " -----------------------------------------------" << endl;
  cout << endl;
}

/**
$Log: IADCTConf.cc,v $
Revision 1.3  2023/07/18 12:25:11  cupsoft
*** empty log message ***

Revision 1.2  2023/03/29 23:05:27  cupsoft
*** empty log message ***

Revision 1.1  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.14  2020/07/10 02:38:36  cupsoft
*** empty log message ***

Revision 1.13  2019/11/25 02:04:07  cupsoft
*** empty log message ***

Revision 1.12  2019/05/20 03:00:30  cupsoft
*** empty log message ***

Revision 1.11  2018/11/01 01:40:44  cupsoft
*** empty log message ***

Revision 1.10  2018/09/20 01:01:10  cupsoft
*** empty log message ***

Revision 1.9  2018/04/30 05:42:41  cupsoft
*** empty log message ***

Revision 1.8  2018/02/21 02:32:37  cupsoft
*** empty log message ***

Revision 1.7  2017/05/19 01:58:58  cupsoft
*** empty log message ***

Revision 1.6  2017/04/06 23:52:37  cupsoft
*** empty log message ***

Revision 1.5  2016/12/05 09:52:51  cupsoft
*** empty log message ***

Revision 1.4  2016/10/02 10:55:13  cupsoft
*** empty log message ***

Revision 1.3  2016/10/03 06:57:04  cupsoft
*** empty log message ***

Revision 1.2  2016/10/01 17:10:54  cupsoft
clean up

Revision 1.1.1.1  2016/09/30 09:22:24  cupsoft
DAQConfig

Revision 1.4  2016/09/27 03:41:55  cupsoft
*** empty log message ***

Revision 1.3  2016/09/06 02:15:11  cupsoft
*** empty log message ***

Revision 1.2  2016/09/05 05:26:51  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/07/14 07:42:30  cupsoft
DAQSys

Revision 1.1.1.1  2016/03/08 04:37:46  amore
DAQSts

Revision 1.1.1.1  2016/02/29 08:26:44  cupsoft
DAQSys

**/
