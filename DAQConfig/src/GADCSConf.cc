/*
 *
 *  Module:  GADCSConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: TCB controlled GADC 125MHz configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2022/12/19 01:01:11 $
 *  CVS/RCS Revision: $Revision: 1.5 $
 *  Status:           $State: Exp $
 *
 */

#include <iostream>

#include "DAQConfig/GADCSConf.hh"

using namespace std;

ClassImp(GADCSConf)

GADCSConf::GADCSConf()
  : GADCTConf()
{}

GADCSConf::GADCSConf(int sid)
  : GADCTConf(sid)
{
  SetNameTitle("GADCS", "Standalone GADC 125MHz");
  fIsLinked = true;
  fTRGON = 1;
  fPTRG = 0;
  fPSC = 1;

  fADCType = ADC::GADCS;
}

GADCSConf::~GADCSConf()
{
}

void GADCSConf::PrintConf() const
{
  cout << Form(" ++ %s config: SID(%d) MID(%1d) NCH(%1d) RL(%d) DSR(%d) TLT(%X) TRGON(%d) PTRG(%d) PSC(%d)",
               GetName(), fSID, fMID, fNCH, fRL, fDSR, fTLT, fTRGON, fPTRG, 0) << endl;
  if (fIsEnabled)
    cout << Form(" ++ This %s is enabled", GetName()) << endl;
  else
    cout << Form(" ++ This %s is disabled", GetName()) << endl;

  cout << " -----------------------------------------------" << endl;
  cout << "        CID : ";
  for (int i = 0; i < fNCH; i++)
    cout << Form("%8d", fCID[i]);
  cout << endl;
  cout << "        PID : ";
  for (int i = 0; i < fNCH; i++)
    cout << Form("%8d", fPID[i]);
  cout << endl;
  cout << "        POL : ";
  for (int i = 0; i < fNCH; i++)
    cout << Form("%8d", fPOL[i]);
  cout << endl;
  cout << "     DACOFF : ";
  for (int i = 0; i < fNCH; i++)
    cout << Form("%8d", fDACOFF[i]);
  cout << endl;
  cout << "        DLY : ";
  for (int i = 0; i < fNCH; i++)
    cout << Form("%8d", fDLY[i]);
  cout << endl;
  cout << "      DTIME : ";
  for (int i = 0; i < fNCH; i++)
    cout << Form("%8d", fDT[i]);
  cout << endl;
  cout << "         CW : ";
  for (int i = 0; i < fNCH; i++)
    cout << Form("%8d", fCW[i]);
  cout << endl;
  cout << "         TM : ";
  for (int i = 0; i < fNCH; i++)
    cout << Form("%8d", fTM[i]);
  cout << endl;
  cout << "        THR : ";
  for (int i = 0; i < fNCH; i++)
    cout << Form("%8d", fTHR[i]);
  cout << endl;
  cout << "        PCT : ";
  for (int i = 0; i < fNCH; i++)
    cout << Form("%8d", fPCT[i]);
  cout << endl;
  cout << "        PCI : ";
  for (int i = 0; i < fNCH; i++)
    cout << Form("%8d", fPCI[i]);
  cout << endl;
  cout << "        PWT : ";
  for (int i = 0; i < fNCH; i++)
    cout << Form("%8d", fPWT[i]);
  cout << endl;
  cout << "        PSW : ";
  for (int i = 0; i < fNCH; i++)
    cout << Form("%8d", fPSW[i]);
  cout << endl;
  cout << " -----------------------------------------------" << endl;
  cout << endl;
}

/**
$Log: GADCSConf.cc,v $
Revision 1.5  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.5  2020/07/10 02:38:36  cupsoft
*** empty log message ***

Revision 1.4  2019/11/25 02:04:07  cupsoft
*** empty log message ***

Revision 1.3  2019/07/11 08:22:11  cupsoft
*** empty log message ***

Revision 1.2  2019/07/09 07:39:06  cupsoft
*** empty log message ***

Revision 1.1  2019/05/30 23:59:55  cupsoft
*** empty log message ***

Revision 1.8  2018/11/01 01:40:44  cupsoft
*** empty log message ***

Revision 1.7  2018/01/08 05:14:45  cupsoft
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
