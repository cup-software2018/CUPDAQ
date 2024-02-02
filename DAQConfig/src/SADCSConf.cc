/*
 *
 *  Module:  SADCSConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: Standalone SADC 500MHz configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2022/12/19 01:01:11 $
 *  CVS/RCS Revision: $Revision: 1.9 $
 *  Status:       $State: Exp $
 *
 */

#include <iostream>

#include "DAQConfig/SADCSConf.hh"

using namespace std;

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

SADCSConf::~SADCSConf() {}

void SADCSConf::PrintConf() const
{
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
$Log: SADCSConf.cc,v $
Revision 1.9  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.8  2019/11/25 02:04:07  cupsoft
*** empty log message ***

Revision 1.7  2018/11/01 01:40:44  cupsoft
*** empty log message ***

Revision 1.6  2017/04/06 23:52:37  cupsoft
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
