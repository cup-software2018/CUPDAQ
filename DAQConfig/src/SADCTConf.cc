/*
 *
 *  Module:  SADCTConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: TCB controlled SADC 500MHz configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/07/18 12:25:11 $
 *  CVS/RCS Revision: $Revision: 1.13 $
 *  Status:       $State: Exp $
 *
 */

#include "DAQConfig/SADCTConf.hh"
#include <iostream>

using namespace std;

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

SADCTConf::~SADCTConf() {}

void SADCTConf::PrintConf() const
{
  cout << Form(" ++ %s config: SID(%d) MID(%d) NCH(%d) CW(%d) GW(%d) PSW(%d) "
               "SUBPED(%d)",
               GetName(), fSID, fMID, fNCH, fCW, fGW, fPSW, fSUBPED)
       << endl;
  cout << Form("                  TLT1(%X) TLT2(%X) TLT3(%X) TLT4(%X)", fTLT[0],
               fTLT[1], fTLT[2], fTLT[3])
       << endl;
  cout << Form("                  TLT5(%X) TLT6(%X) TLT7(%X) TLT8(%X)", fTLT[4],
               fTLT[5], fTLT[6], fTLT[7])
       << endl;
  if (fIsEnabled) cout << Form(" ++ This %s is enabled", GetName()) << endl;
  else cout << Form(" ++ This %s is disabled", GetName()) << endl;

  cout << " -----------------------------------------------" << endl;
  for (int j = 0; j < 4; j++) {
    if (j > 0) cout << endl;
    cout << "    CID : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      cout << Form("%6d", fCID[i]);
    cout << endl;
    cout << "    PID : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      cout << Form("%6d", fPID[i]);
    cout << endl;
    cout << "    THR : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      cout << Form("%6d", fTHR[i]);
    cout << endl;
    cout << "    DLY : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      cout << Form("%6d", fDLY[i]);
    cout << endl;
  }
  cout << " -----------------------------------------------" << endl;
  cout << endl;
}

/**
$Log: SADCTConf.cc,v $
Revision 1.13  2023/07/18 12:25:11  cupsoft
*** empty log message ***

Revision 1.12  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.11  2019/11/25 02:04:07  cupsoft
*** empty log message ***

Revision 1.10  2018/11/01 01:40:44  cupsoft
*** empty log message ***

Revision 1.9  2017/06/12 02:12:22  cupsoft
*** empty log message ***

Revision 1.8  2017/05/19 01:58:58  cupsoft
*** empty log message ***

Revision 1.7  2017/04/06 23:52:37  cupsoft
*** empty log message ***

Revision 1.6  2016/12/14 04:54:27  cupsoft
*** empty log message ***

Revision 1.5  2016/12/05 09:52:51  cupsoft
*** empty log message ***

Revision 1.4  2016/10/02 10:55:14  cupsoft
*** empty log message ***

Revision 1.3  2016/10/03 06:57:04  cupsoft
*** empty log message ***

Revision 1.2  2016/10/01 17:10:54  cupsoft
clean up

Revision 1.1.1.1  2016/09/30 09:22:24  cupsoft
DAQConfig

Revision 1.3  2016/09/28 16:54:49  cupsoft
*** empty log message ***

Revision 1.2  2016/09/27 03:41:55  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/07/14 07:42:30  cupsoft
DAQSys

Revision 1.1.1.1  2016/03/08 04:37:46  amore
DAQSts

Revision 1.1.1.1  2016/02/29 08:26:44  cupsoft
DAQSys

**/
