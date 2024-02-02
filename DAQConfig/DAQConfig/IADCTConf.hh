/*
 *
 *  Module:  IADCTConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: TCB controlled SiPM 64MHz ADC configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2022/12/19 01:01:11 $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 */

#ifndef IADCTConf_hh
#define IADCTConf_hh

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class IADCTConf : public AbsConf {
public:
  IADCTConf();
  IADCTConf(int sid);
  virtual ~IADCTConf();

  virtual void SetNCH(int val);
  virtual void SetMODE(int val);
  virtual void SetRL(int val);
  virtual void SetCW(int val);
  virtual void SetGW(int val);
  virtual void SetPSW(int val);
  virtual void SetCID(int ch, int val);
  virtual void SetPID(int ch, int val);
  virtual void SetTHR(int ch, int val);
  virtual void SetDLY(int ch, int val);
  virtual void SetTLT(int group, int val);
  virtual void SetHV(int group, float val);

  virtual int NCH() const;
  virtual int MODE() const;
  virtual int RL() const;
  virtual int CW() const;
  virtual int GW() const;
  virtual int PSW() const;
  virtual int CID(int ch) const;
  virtual int PID(int ch) const;
  virtual int THR(int ch) const;
  virtual int DLY(int ch) const;
  virtual int TLT(int group) const;
  virtual float HV(int group) const;

  virtual void PrintConf() const;

protected:
  int fNCH;
  int fMODE;
  int fRL;
  int fCW;
  int fGW;
  int fPSW;
  int fCID[kNCHIADC];
  int fPID[kNCHIADC];
  int fTHR[kNCHIADC];
  int fDLY[kNCHIADC];
  int fTLT[10];
  float fHV[5];

  ClassDef(IADCTConf, 1)
};

inline void IADCTConf::SetNCH(int val) { fNCH = val; }

inline void IADCTConf::SetMODE(int val) { fMODE = val; }

inline void IADCTConf::SetRL(int val) { fRL = val; }

inline void IADCTConf::SetCW(int val) { fCW = val; }

inline void IADCTConf::SetGW(int val) { fGW = val; }

inline void IADCTConf::SetPSW(int val) { fPSW = val; }

inline void IADCTConf::SetCID(int ch, int val) { fCID[ch] = val; }

inline void IADCTConf::SetPID(int ch, int val) { fPID[ch] = val; }

inline void IADCTConf::SetTHR(int ch, int val) { fTHR[ch] = val; }

inline void IADCTConf::SetDLY(int ch, int val) { fDLY[ch] = val; }

inline void IADCTConf::SetTLT(int group, int val) { fTLT[group] = val; }

inline void IADCTConf::SetHV(int group, float val) { fHV[group] = val; }

inline int IADCTConf::NCH() const { return fNCH; }

inline int IADCTConf::MODE() const { return fMODE; }

inline int IADCTConf::RL() const
{
  if (fMODE == 0) return 0;
  return fRL;
}

inline int IADCTConf::CW() const { return fCW; }

inline int IADCTConf::GW() const
{
  if (fMODE > 0) return 0;
  return fGW;
}

inline int IADCTConf::PSW() const { return fPSW; }

inline int IADCTConf::CID(int ch) const { return fCID[ch]; }

inline int IADCTConf::PID(int ch) const { return fPID[ch]; }

inline int IADCTConf::THR(int ch) const { return fTHR[ch]; }

inline int IADCTConf::DLY(int ch) const { return fDLY[ch]; }

inline int IADCTConf::TLT(int group) const { return fTLT[group]; }

inline float IADCTConf::HV(int group) const { return fHV[group]; }

#endif

/**
$Log: IADCTConf.hh,v $
Revision 1.1  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.10  2020/07/10 02:38:36  cupsoft
*** empty log message ***

Revision 1.9  2019/05/20 03:00:30  cupsoft
*** empty log message ***

Revision 1.8  2018/09/20 01:01:10  cupsoft
*** empty log message ***

Revision 1.7  2018/04/30 05:42:41  cupsoft
*** empty log message ***

Revision 1.6  2017/04/06 23:52:36  cupsoft
*** empty log message ***

Revision 1.5  2016/12/05 09:52:50  cupsoft
*** empty log message ***

Revision 1.4  2016/11/12 04:05:28  cupsoft
*** empty log message ***

Revision 1.3  2016/10/03 06:57:03  cupsoft
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
