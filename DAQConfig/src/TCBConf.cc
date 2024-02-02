/*
 *
 *  Module:  TCBConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: Standalone TCB for KIMS configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/07/18 12:25:11 $
 *  CVS/RCS Revision: $Revision: 1.13 $
 *  Status:           $State: Exp $
 *
 */

#include "DAQConfig/TCBConf.hh"
#include <iostream>

using namespace std;

ClassImp(TCBConf)

    TCBConf::TCBConf()
    : AbsConf(0)
{
}

TCBConf::TCBConf(int sid)
    : AbsConf(sid, ADC::TCB)
{
  SetNameTitle("TCB", "Trigger Control Board");
  SetEnable();

  fTM = 1;
  fCW = 1000;
  fDLY = 0;
  fPTRG = 1000;
  fMTHRF = 1;
  fPSCF = 1;
  fDTF = 0;
  fSWF = 0;
  fMTHRSM = 1;
  fPSCSM = 1;
  fDTSM = 0;
  fSWSM = 0;
  fMTHRSL = 1;
  fPSCSL = 1;
  fDTSL = 0;
  fSWSL = 0;
  fMTHRI = 1;
  fPSCI = 1;
  fDTI = 0;
  fSWI = 0;
  fTCBTYPE = TCB::V1;
}

TCBConf::~TCBConf() {}

void TCBConf::PrintConf() const
{
  if (fTCBTYPE == TCB::V2) {
    int f = (int)TESTBIT(fSWF, 0);
    int sm = (int)TESTBIT(fSWF, 1);
    int sl = (int)TESTBIT(fSWF, 2);
    int i = (int)TESTBIT(fSWF, 3);
    const char * swf = Form("%d %d %d %d", f, sm, sl, i);

    f = (int)TESTBIT(fSWSM, 0);
    sm = (int)TESTBIT(fSWSM, 1);
    sl = (int)TESTBIT(fSWSM, 2);
    i = (int)TESTBIT(fSWSM, 3);
    const char * swsm = Form("%d %d %d %d", f, sm, sl, i);

    f = (int)TESTBIT(fSWSL, 0);
    sm = (int)TESTBIT(fSWSL, 1);
    sl = (int)TESTBIT(fSWSL, 2);
    i = (int)TESTBIT(fSWSL, 3);
    const char * swsl = Form("%d %d %d %d", f, sm, sl, i);

    f = (int)TESTBIT(fSWI, 0);
    sm = (int)TESTBIT(fSWI, 1);
    sl = (int)TESTBIT(fSWI, 2);
    i = (int)TESTBIT(fSWI, 3);
    const char * swi = Form("%d %d %d %d", f, sm, sl, i);
    cout
        << Form(
               " ++   TCB config: SID(%d) TRGON(%d) CW(%d) DLY(%d) PTRIG(%d) \n"
               "                  MTHRF(%d)  PSCF(%d)  DTF(%d)  TSWF(%s) \n"
               "                  MTHRSM(%d) PSCSM(%d) DTSM(%d) TSWSM(%s) \n"
               "                  MTHRSL(%d) PSCSL(%d) DTSL(%d) TSWSL(%s) \n"
               "                  MTHRI(%d)  PSCI(%d)  DTI(%d)  TSWI(%s)",
               fSID, fTM, fCW, fDLY, fPTRG, fMTHRF, fPSCF, fDTF, swf, fMTHRSM,
               fPSCSM, fDTSM, swsm, fMTHRSL, fPSCSL, fDTSL, swsl, fMTHRI, fPSCI,
               fDTI, swi)
        << endl;
  }
  else {
    cout
        << Form(
               " ++   TCB config: SID(%d) TRGON(%d) CW(%d) DLY(%d) PTRIG(%d) \n"
               "                  MTHR(%d) PSC(%d) DT(%d)",
               fSID, fTM, fCW, fDLY, fPTRG, fMTHRF, fPSCF, fDTF)
        << endl;
  }
}

/**
$Log: TCBConf.cc,v $
Revision 1.13  2023/07/18 12:25:11  cupsoft
*** empty log message ***

Revision 1.12  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.11  2019/11/25 02:04:07  cupsoft
*** empty log message ***

Revision 1.10  2019/07/24 07:04:23  cupsoft
*** empty log message ***

Revision 1.9  2018/11/01 01:40:44  cupsoft
*** empty log message ***

Revision 1.8  2018/02/19 03:34:24  cupsoft
*** empty log message ***

Revision 1.7  2018/01/08 05:14:46  cupsoft
*** empty log message ***

Revision 1.6  2017/05/19 01:58:58  cupsoft
*** empty log message ***

Revision 1.5  2017/03/08 05:21:44  cupsoft
*** empty log message ***

Revision 1.4  2016/12/14 04:54:27  cupsoft
*** empty log message ***

Revision 1.3  2016/12/05 09:52:51  cupsoft
*** empty log message ***

Revision 1.2  2016/10/03 09:52:02  cupsoft
*** empty log message ***

Revision 1.1  2016/10/03 06:57:05  cupsoft
*** empty log message ***

Revision 1.2  2016/10/01 17:10:54  cupsoft
clean up

Revision 1.1.1.1  2016/09/30 09:22:24  cupsoft
DAQConfig

Revision 1.2  2016/09/27 03:41:55  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/07/14 07:42:30  cupsoft
DAQSys

Revision 1.1.1.1  2016/03/08 04:37:46  amore
DAQSts

Revision 1.1.1.1  2016/02/29 08:26:44  cupsoft
DAQSys

**/
