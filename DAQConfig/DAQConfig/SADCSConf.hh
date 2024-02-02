/*
 *
 *  Module:  SADCSConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: Standalone SADC 64MHz configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2022/12/19 01:01:11 $
 *  CVS/RCS Revision: $Revision: 1.4 $
 *  Status:           $State: Exp $
 *
 */

#ifndef SADCSConf_hh
#define SADCSConf_hh

#include "DAQConfig/SADCTConf.hh"
#include "OnlConsts/adcconsts.hh"

class SADCSConf : public SADCTConf {
public:
  SADCSConf();
  SADCSConf(int sid);
  virtual ~SADCSConf();

  virtual void SetPTRG(int val);
  virtual void SetMTHR(int val);
  virtual void SetPSC(int val);

  virtual int PTRG() const;
  virtual int MTHR() const;
  virtual int PSC() const;

  virtual void PrintConf() const;

protected:
  int fPTRG;
  int fMTHR;
  int fPSC;

  ClassDef(SADCSConf, 1)
};

inline void SADCSConf::SetPTRG(int val) { fPTRG = val; }

inline void SADCSConf::SetMTHR(int val) { fMTHR = val; }

inline void SADCSConf::SetPSC(int val) { fPSC = val; }

inline int SADCSConf::PTRG() const { return fPTRG; }

inline int SADCSConf::MTHR() const { return fMTHR; }

inline int SADCSConf::PSC() const { return fPSC; }

#endif

/**
$Log: SADCSConf.hh,v $
Revision 1.4  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.3  2016/12/05 09:52:50  cupsoft
*** empty log message ***

Revision 1.2  2016/10/01 17:10:54  cupsoft
clean up

Revision 1.1.1.1  2016/09/30 09:22:24  cupsoft
DAQConfig

Revision 1.4  2016/09/28 16:54:49  cupsoft
*** empty log message ***

Revision 1.3  2016/09/27 03:41:52  cupsoft
*** empty log message ***

Revision 1.2  2016/09/05 05:26:49  cupsoft
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
