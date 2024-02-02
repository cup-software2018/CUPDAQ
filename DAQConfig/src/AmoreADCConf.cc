/*
 *
 *  Module:  AmoreADCConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: AMoRE ADC configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/07/18 12:25:11 $
 *  CVS/RCS Revision: $Revision: 1.13 $
 *  Status:           $State: Exp $
 *
 */

#include <iostream>

#include "DAQConfig/AmoreADCConf.hh"

using namespace std;

ClassImp(AmoreADCConf)

AmoreADCConf::AmoreADCConf()
    : AbsConf()
{
}

AmoreADCConf::AmoreADCConf(int sid)
    : AbsConf(sid, ADC::AMOREADC)
{
  SetNameTitle("AMOREADC", "ADC for AMoRE Experiment");

  fNCH = kNCHAMOREADC;
  fSR = 10;
  fRL = 0;
  fDLY = 1000;
  fCW = 0;
  fSKBIN = 0;
  fZEROSUP = 0;

  for (int i = 0; i < kNCHAMOREADC; i++) {
    fCID[i] = i + 1;
    fPID[i] = 0;
    fTRGON[i] = 0;
    fORDER[i] = 1;
    fLOWER[i] = 100;
    fUPPER[i] = 1000;
    fTHR[i] = 10;
    fDT[i] = 0;
  }
}

AmoreADCConf::~AmoreADCConf() {}

void AmoreADCConf::PrintConf() const
{
  cout << Form(" ++ AMOREADC config: SID(%d) MID(%d) NCH(%d) SR(%d) RL(%d) "
               "DLY(%d) CW(%d) SKBIN(%d) ZEORESUP(%d)",
               fSID, fMID, fNCH, fSR, fRL, fDLY, fCW, fSKBIN, fZEROSUP)
       << endl;
  if (fIsEnabled) cout << " ++ This AMOREADC is enabled" << endl;
  else cout << " ++ This AMOREADC is disabled" << endl;

  cout << " -----------------------------------------------" << endl;
  for (int j = 0; j < 2; j++) {
    if (j > 0) cout << endl;
    cout << "        CID : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      cout << Form("%6d", fCID[i]);
    cout << endl;
    cout << "        PID : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      cout << Form("%6d", fPID[i]);
    cout << endl;
    cout << "      TRGON : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      cout << Form("%6d", fTRGON[i]);
    cout << endl;
    cout << "      ORDER : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      cout << Form("%6d", fORDER[i]);
    cout << endl;
    cout << "      LOWER : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      cout << Form("%6d", fLOWER[i]);
    cout << endl;
    cout << "      UPPER : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      cout << Form("%6d", fUPPER[i]);
    cout << endl;
    cout << "        THR : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      cout << Form("%6d", fTHR[i]);
    cout << endl;
    cout << "         DT : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++)
      cout << Form("%6d", fDT[i]);
    cout << endl;
  }
  cout << " -----------------------------------------------" << endl;
  cout << endl;
}

/**
$Log: AmoreADCConf.cc,v $
Revision 1.13  2023/07/18 12:25:11  cupsoft
*** empty log message ***

Revision 1.12  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.11  2020/01/09 04:57:26  cupsoft
*** empty log message ***

Revision 1.10  2019/11/25 02:04:07  cupsoft
*** empty log message ***

Revision 1.9  2019/07/24 07:04:23  cupsoft
*** empty log message ***

Revision 1.8  2019/07/12 07:18:42  cupsoft
*** empty log message ***

Revision 1.7  2018/11/01 01:40:44  cupsoft
*** empty log message ***

Revision 1.6  2017/04/06 23:52:36  cupsoft
*** empty log message ***

Revision 1.5  2017/04/06 12:11:56  cupsoft
*** empty log message ***

Revision 1.4  2017/01/25 02:14:18  cupsoft
*** empty log message ***

Revision 1.3  2016/12/14 04:54:27  cupsoft
*** empty log message ***

Revision 1.2  2016/12/05 09:52:51  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/09/30 09:22:24  cupsoft
DAQConfig

Revision 1.3  2016/09/27 03:41:55  cupsoft
*** empty log message ***

Revision 1.2  2016/09/05 05:26:51  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/07/14 07:42:30  cupsoft
DAQSys

Revision 1.1.1.1  2016/03/08 04:37:46  amore
DAQSts

**/
