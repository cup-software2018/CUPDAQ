/*
 *
 *  Module:  AmoreADCConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: AMoRE ADC configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2022/12/19 01:01:11 $
 *  CVS/RCS Revision: $Revision: 1.9 $
 *  Status:           $State: Exp $
 *
 */

#ifndef AmoreADCConf_hh
#define AmoreADCConf_hh

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class AmoreADCConf : public AbsConf {
public:
  AmoreADCConf();
  AmoreADCConf(int sid);
  virtual ~AmoreADCConf();

  virtual void SetNCH(int val);
  virtual void SetSR(int val);
  virtual void SetRL(int val);
  virtual void SetDLY(int val);
  virtual void SetCW(int val);
  virtual void SetSKBIN(int val);
  virtual void SetZEROSUP(int val);
  virtual void SetCID(int ch, int val);
  virtual void SetPID(int ch, int val);
  virtual void SetTRGON(int ch, int val);
  virtual void SetORDER(int ch, int val);
  virtual void SetLOWER(int ch, int val);
  virtual void SetUPPER(int ch, int val);
  virtual void SetTHR(int ch, int val);
  virtual void SetDT(int ch, int val);

  virtual int NCH() const;
  virtual int SR() const;
  virtual int RL() const;
  virtual int DLY() const;
  virtual int CW() const;
  virtual int SKBIN() const;
  virtual int ZEROSUP() const;
  virtual int CID(int ch) const;
  virtual int PID(int ch) const;
  virtual int TRGON(int ch) const;
  virtual int ORDER(int ch) const;
  virtual int LOWER(int ch) const;
  virtual int UPPER(int ch) const;
  virtual int THR(int ch) const;
  virtual int DT(int ch) const;

  virtual void PrintConf() const;

private:
  int fNCH;
  int fSR;
  int fRL;
  int fDLY;
  int fCW;
  int fSKBIN;
  int fZEROSUP;
  int fCID[kNCHAMOREADC];
  int fPID[kNCHAMOREADC];
  int fTRGON[kNCHAMOREADC];
  int fORDER[kNCHAMOREADC];
  int fLOWER[kNCHAMOREADC];
  int fUPPER[kNCHAMOREADC];
  int fTHR[kNCHAMOREADC];
  int fDT[kNCHAMOREADC];

  ClassDef(AmoreADCConf, 1)
};

inline void AmoreADCConf::SetNCH(int val) { fNCH = val; }

inline void AmoreADCConf::SetSR(int val) { fSR = val; }

inline void AmoreADCConf::SetRL(int val) { fRL = val; }

inline void AmoreADCConf::SetDLY(int val) { fDLY = val; }

inline void AmoreADCConf::SetCW(int val) { fCW = val; }

inline void AmoreADCConf::SetSKBIN(int val) { fSKBIN = val; }

inline void AmoreADCConf::SetZEROSUP(int val) { fZEROSUP = val; }

inline void AmoreADCConf::SetCID(int ch, int val) { fCID[ch] = val; }

inline void AmoreADCConf::SetPID(int ch, int val) { fPID[ch] = val; }

inline void AmoreADCConf::SetTRGON(int ch, int val) { fTRGON[ch] = val; }

inline void AmoreADCConf::SetORDER(int ch, int val) { fORDER[ch] = val; }

inline void AmoreADCConf::SetLOWER(int ch, int val) { fLOWER[ch] = val; }

inline void AmoreADCConf::SetUPPER(int ch, int val) { fUPPER[ch] = val; }

inline void AmoreADCConf::SetTHR(int ch, int val) { fTHR[ch] = val; }

inline void AmoreADCConf::SetDT(int ch, int val) { fDT[ch] = val; }

inline int AmoreADCConf::NCH() const { return fNCH; }

inline int AmoreADCConf::SR() const { return fSR; }

inline int AmoreADCConf::RL() const { return fRL; }

inline int AmoreADCConf::DLY() const { return fDLY; }

inline int AmoreADCConf::CW() const { return fCW; }

inline int AmoreADCConf::SKBIN() const { return fSKBIN; }

inline int AmoreADCConf::ZEROSUP() const { return fZEROSUP; }

inline int AmoreADCConf::CID(int ch) const { return fCID[ch]; }

inline int AmoreADCConf::PID(int ch) const { return fPID[ch]; }

inline int AmoreADCConf::TRGON(int ch) const { return fTRGON[ch]; }

inline int AmoreADCConf::ORDER(int ch) const { return fORDER[ch]; }

inline int AmoreADCConf::UPPER(int ch) const { return fUPPER[ch]; }

inline int AmoreADCConf::LOWER(int ch) const { return fLOWER[ch]; }

inline int AmoreADCConf::THR(int ch) const { return fTHR[ch]; }

inline int AmoreADCConf::DT(int ch) const { return fDT[ch]; }

#endif

/**
$Log: AmoreADCConf.hh,v $
Revision 1.9  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.8  2020/01/09 04:57:26  cupsoft
*** empty log message ***

Revision 1.7  2019/11/25 02:04:07  cupsoft
*** empty log message ***

Revision 1.6  2019/07/12 07:18:42  cupsoft
*** empty log message ***

Revision 1.5  2017/04/06 23:52:36  cupsoft
*** empty log message ***

Revision 1.4  2017/01/25 02:14:18  cupsoft
*** empty log message ***

Revision 1.3  2016/12/14 04:54:26  cupsoft
*** empty log message ***

Revision 1.2  2016/12/05 09:52:50  cupsoft
*** empty log message ***

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

**/
