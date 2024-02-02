/*
 *
 *  Module:  GADCTConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: TCB controlled FADC 125MHz configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2022/12/19 01:01:11 $
 *  CVS/RCS Revision: $Revision: 1.3 $
 *  Status:           $State: Exp $
 *
 */

#ifndef GADCTConf_hh
#define GADCTConf_hh

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class GADCTConf : public AbsConf {
public:
  GADCTConf();
  GADCTConf(int sid);
  virtual ~GADCTConf();

  virtual void SetNCH(int val);
  virtual void SetRL(int val);
  virtual void SetTLT(int val);
  virtual void SetDSR(int val);
  virtual void SetCID(int ch, int val);
  virtual void SetPID(int ch, int val);
  virtual void SetPOL(int ch, int val);
  virtual void SetDACOFF(int ch, int val);
  virtual void SetAMD(int ch, int val);
  virtual void SetDLY(int ch, int val);
  virtual void SetDT(int ch, int val);
  virtual void SetCW(int ch, int val);
  virtual void SetTM(int ch, int val);
  virtual void SetTHR(int ch, int val);
  virtual void SetPCT(int ch, int val);
  virtual void SetPCI(int ch, int val);
  virtual void SetPWT(int ch, int val);
  virtual void SetPSW(int ch, int val);

  virtual int NCH() const;
  virtual int RL() const;
  virtual int TLT() const;
  virtual int DSR() const;
  virtual int CID(int ch) const;
  virtual int PID(int ch) const;
  virtual int POL(int ch) const;
  virtual int DACOFF(int ch) const;
  virtual int AMD(int ch) const;
  virtual int DLY(int ch) const;
  virtual int DT(int ch) const;
  virtual int CW(int ch) const;
  virtual int TM(int ch) const;
  virtual int THR(int ch) const;
  virtual int PCT(int ch) const;
  virtual int PCI(int ch) const;
  virtual int PWT(int ch) const;
  virtual int PSW(int ch) const;

  virtual void PrintConf() const;

protected:
  int fNCH;
  int fRL;
  int fTLT;
  int fDSR;
  int fCID[kNCHGADC];
  int fPID[kNCHGADC];
  int fPOL[kNCHGADC];
  int fDACOFF[kNCHGADC];
  int fAMD[kNCHGADC];
  int fDLY[kNCHGADC];
  int fDT[kNCHGADC];
  int fCW[kNCHGADC];
  int fTM[kNCHGADC];
  int fTHR[kNCHGADC];
  int fPCT[kNCHGADC];
  int fPCI[kNCHGADC];
  int fPWT[kNCHGADC];
  int fPSW[kNCHGADC];

  ClassDef(GADCTConf, 1)
};

inline void GADCTConf::SetNCH(int val) { fNCH = val; }

inline void GADCTConf::SetRL(int val) { fRL = val; }

inline void GADCTConf::SetTLT(int val) { fTLT = val; }

inline void GADCTConf::SetDSR(int val) { fDSR = val; }

inline void GADCTConf::SetCID(int ch, int val) { fCID[ch] = val; }

inline void GADCTConf::SetPID(int ch, int val) { fPID[ch] = val; }

inline void GADCTConf::SetAMD(int ch, int val) { fAMD[ch] = val; }

inline void GADCTConf::SetPOL(int ch, int val) { fPOL[ch] = val; }

inline void GADCTConf::SetTHR(int ch, int val) { fTHR[ch] = val; }

inline void GADCTConf::SetDLY(int ch, int val) { fDLY[ch] = val; }

inline void GADCTConf::SetDACOFF(int ch, int val) { fDACOFF[ch] = val; }

inline void GADCTConf::SetDT(int ch, int val) { fDT[ch] = val; }

inline void GADCTConf::SetCW(int ch, int val) { fCW[ch] = val; }

inline void GADCTConf::SetTM(int ch, int val) { fTM[ch] = val; }

inline void GADCTConf::SetPCT(int ch, int val) { fPCT[ch] = val; }

inline void GADCTConf::SetPCI(int ch, int val) { fPCI[ch] = val; }

inline void GADCTConf::SetPWT(int ch, int val) { fPWT[ch] = val; }

inline void GADCTConf::SetPSW(int ch, int val) { fPSW[ch] = val; }

inline int GADCTConf::NCH() const { return fNCH; }

inline int GADCTConf::RL() const { return fRL; }

inline int GADCTConf::TLT() const { return fTLT; }

inline int GADCTConf::DSR() const { return fDSR; }

inline int GADCTConf::CID(int ch) const { return fCID[ch]; }

inline int GADCTConf::PID(int ch) const { return fPID[ch]; }

inline int GADCTConf::AMD(int ch) const { return fAMD[ch]; }

inline int GADCTConf::POL(int ch) const { return fPOL[ch]; }

inline int GADCTConf::THR(int ch) const { return fTHR[ch]; }

inline int GADCTConf::DLY(int ch) const { return fDLY[ch]; }

inline int GADCTConf::DACOFF(int ch) const { return fDACOFF[ch]; }

inline int GADCTConf::DT(int ch) const { return fDT[ch]; }

inline int GADCTConf::CW(int ch) const { return fCW[ch]; }

inline int GADCTConf::TM(int ch) const { return fTM[ch]; }

inline int GADCTConf::PCT(int ch) const { return fPCT[ch]; }

inline int GADCTConf::PCI(int ch) const { return fPCI[ch]; }

inline int GADCTConf::PWT(int ch) const { return fPWT[ch]; }

inline int GADCTConf::PSW(int ch) const { return fPSW[ch]; }

#endif

/**
$Log: GADCTConf.hh,v $
Revision 1.3  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.3  2020/07/10 02:38:36  cupsoft
*** empty log message ***

Revision 1.2  2019/07/09 07:39:06  cupsoft
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
