/*
 *
 *  Module:  SADCTConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: TCB controlled SADC 64MHz configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2022/12/19 01:01:11 $
 *  CVS/RCS Revision: $Revision: 1.9 $
 *  Status:           $State: Exp $
 *
 */

#ifndef SADCTConf_hh
#define SADCTConf_hh

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class SADCTConf : public AbsConf {
public:
  SADCTConf();
  SADCTConf(int sid);
  virtual ~SADCTConf();

  virtual void SetNCH(int val);
  virtual void SetCW(int val);
  virtual void SetGW(int val);
  virtual void SetPSW(int val);
  virtual void SetCID(int ch, int val);
  virtual void SetPID(int ch, int val);
  virtual void SetTHR(int ch, int val);
  virtual void SetDLY(int ch, int val);
  virtual void SetTLT(int group, int val);
  virtual void SetSUBPED(int val);

  virtual int NCH() const;
  virtual int CW() const;
  virtual int GW() const;
  virtual int PSW() const;
  virtual int CID(int ch) const;
  virtual int PID(int ch) const;
  virtual int THR(int ch) const;
  virtual int DLY(int ch) const;
  virtual int TLT(int group) const;
  virtual int SUBPED() const;

  virtual void PrintConf() const;

protected:
  int fNCH;
  int fCW;
  int fGW;
  int fPSW;
  int fCID[kNCHSADC];
  int fPID[kNCHSADC];
  int fTHR[kNCHSADC];
  int fDLY[kNCHSADC];
  int fTLT[8];
  int fSUBPED;

  ClassDef(SADCTConf, 1)
};

inline void SADCTConf::SetNCH(int val) { fNCH = val; }

inline void SADCTConf::SetCW(int val) { fCW = val; }

inline void SADCTConf::SetGW(int val) { fGW = val; }

inline void SADCTConf::SetPSW(int val) { fPSW = val; }

inline void SADCTConf::SetCID(int ch, int val) { fCID[ch] = val; }

inline void SADCTConf::SetPID(int ch, int val) { fPID[ch] = val; }

inline void SADCTConf::SetTHR(int ch, int val) { fTHR[ch] = val; }

inline void SADCTConf::SetDLY(int ch, int val) { fDLY[ch] = val; }

inline void SADCTConf::SetTLT(int group, int val) { fTLT[group] = val; }

inline void SADCTConf::SetSUBPED(int val) { fSUBPED = val; }

inline int SADCTConf::NCH() const { return fNCH; }

inline int SADCTConf::GW() const { return fGW; }

inline int SADCTConf::CW() const { return fCW; }

inline int SADCTConf::PSW() const { return fPSW; }

inline int SADCTConf::CID(int ch) const { return fCID[ch]; }

inline int SADCTConf::PID(int ch) const { return fPID[ch]; }

inline int SADCTConf::THR(int ch) const { return fTHR[ch]; }

inline int SADCTConf::DLY(int ch) const { return fDLY[ch]; }

inline int SADCTConf::TLT(int group) const { return fTLT[group]; }

inline int SADCTConf::SUBPED() const { return fSUBPED; }

#endif

/**
$Log: SADCTConf.hh,v $
Revision 1.9  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.8  2017/06/12 02:12:22  cupsoft
*** empty log message ***

Revision 1.7  2017/05/19 01:58:58  cupsoft
*** empty log message ***

Revision 1.6  2017/04/06 23:52:36  cupsoft
*** empty log message ***

Revision 1.5  2016/12/05 09:52:50  cupsoft
*** empty log message ***

Revision 1.4  2016/10/02 10:55:13  cupsoft
*** empty log message ***

Revision 1.3  2016/10/03 06:57:03  cupsoft
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
