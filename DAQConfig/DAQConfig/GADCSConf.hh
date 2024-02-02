/*
 *
 *  Module:  GADCSConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: Standalone GADC 125MHz configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2022/12/19 01:01:11 $
 *  CVS/RCS Revision: $Revision: 1.3 $
 *  Status:           $State: Exp $
 *
 */

#ifndef GADCSConf_hh
#define GADCSConf_hh

#include "DAQConfig/GADCTConf.hh"
#include "OnlConsts/adcconsts.hh"

class GADCSConf : public GADCTConf {
public:
  GADCSConf();
  GADCSConf(int sid);
  virtual ~GADCSConf();

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

  ClassDef(GADCSConf, 1)
};

inline void GADCSConf::SetTRGON(int val) { fTRGON = val; }

inline void GADCSConf::SetPTRG(int val) { fPTRG = val; }

inline void GADCSConf::SetPSC(int val) { fPSC = val; }

inline int GADCSConf::TRGON() const { return fTRGON; }

inline int GADCSConf::PTRG() const { return fPTRG; }

inline int GADCSConf::PSC() const { return fPSC; }
#endif

/**
$Log: GADCSConf.hh,v $
Revision 1.3  2022/12/19 01:01:11  cupsoft
add IADC things

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
