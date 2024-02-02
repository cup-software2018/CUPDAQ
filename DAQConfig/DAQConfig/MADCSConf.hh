/*
 *
 *  Module:  MADCSConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: Standalone GADC 125MHz configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2022/12/19 01:01:11 $
 *  CVS/RCS Revision: $Revision: 1.2 $
 *  Status:           $State: Exp $
 *
 */

#ifndef MADCSConf_hh
#define MADCSConf_hh

#include "DAQConfig/FADCTConf.hh"
#include "OnlConsts/adcconsts.hh"

class MADCSConf : public FADCTConf {
public:
  MADCSConf();
  MADCSConf(int sid);
  virtual ~MADCSConf();

  virtual void SetTRGON(int val);
  virtual void SetPTRG(int val);
  virtual void SetPSC(int val);

  virtual int TRGON() const;
  virtual int PTRG() const;
  virtual int PSC() const;

  virtual void PrintConf() const;

protected:
  int fTRGON;
  int fPTRG;
  int fPSC;

  ClassDef(MADCSConf, 1)
};

inline void MADCSConf::SetTRGON(int val) { fTRGON = val; }

inline void MADCSConf::SetPTRG(int val) { fPTRG = val; }

inline void MADCSConf::SetPSC(int val) { fPSC = val; }

inline int MADCSConf::TRGON() const { return fTRGON; }

inline int MADCSConf::PTRG() const { return fPTRG; }

inline int MADCSConf::PSC() const { return fPSC; }
#endif

/**
$Log: MADCSConf.hh,v $
Revision 1.2  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.2  2020/07/10 02:38:36  cupsoft
*** empty log message ***

Revision 1.1  2019/07/24 07:04:23  cupsoft
*** empty log message ***

Revision 1.2  2019/07/09 07:39:06  cupsoft
*** empty log message ***

Revision 1.1  2019/05/30 23:59:55  cupsoft
*** empty log message ***

Revision 1.5  2018/01/08 05:14:45  cupsoft
*** empty log message ***

Revision 1.4  2016/12/05 09:52:50  cupsoft
*** empty log message ***

Revision 1.3  2016/10/02 10:55:13  cupsoft
*** empty log message ***

Revision 1.2  2016/10/01 17:10:53  cupsoft
clean up

Revision 1.1.1.1  2016/09/30 09:22:24  cupsoft
DAQConfig

Revision 1.3  2016/09/27 03:41:52  cupsoft
*** empty log message ***

Revision 1.2  2016/09/05 05:26:48  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/07/14 07:42:30  cupsoft
DAQSys

Revision 1.2  2016/03/29 10:12:16  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/03/08 04:37:46  amore
DAQSts

Revision 1.1.1.1  2016/02/29 08:26:44  cupsoft
DAQSys

**/
