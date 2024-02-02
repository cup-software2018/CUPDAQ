/*
 *
 *  Module:  FADCTConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: TCB controlled FADC 500MHz configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2022/12/19 01:01:11 $
 *  CVS/RCS Revision: $Revision: 1.10 $
 *  Status:           $State: Exp $
 *
 */

#ifndef FADCTConf_hh
#define FADCTConf_hh

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class FADCTConf : public AbsConf {
public:
  FADCTConf();
  FADCTConf(int sid);
  virtual ~FADCTConf();

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

  void SetPedRMS(int ch, double val);
  double GetPedRMS(int ch) const;

  void SetPMTGain(int ch, double val);
  double GetPMTGain(int ch) const;

  void SetPMTGainScale(int ch, double val);
  double GetPMTGainScale(int ch) const;

  void SetPMTFallTime(int ch, double val);
  double GetPMTFallTime(int ch) const;

  void SetPMTTransitTime(int ch, double val);
  double GetPMTTransitTime(int ch) const;

  void SetPMTTTS(int ch, double val);
  double GetPMTTTS(int ch) const;

  void SetPMTQEff(int ch, double val);
  double GetPMTQEff(int ch) const;

  void SetTRGON(int val) { fTRGON = val; }
  int TRGON() const { return fTRGON; }

protected:
  int fNCH;
  int fRL;
  int fTLT;
  int fDSR;
  int fCID[kNCHFADC];
  int fPID[kNCHFADC];
  int fPOL[kNCHFADC];
  int fDACOFF[kNCHFADC];
  int fAMD[kNCHFADC];
  int fDLY[kNCHFADC];
  int fDT[kNCHFADC];
  int fCW[kNCHFADC];
  int fTM[kNCHFADC];
  int fTHR[kNCHFADC];
  int fPCT[kNCHFADC];
  int fPCI[kNCHFADC];
  int fPWT[kNCHFADC];
  int fPSW[kNCHFADC];

  // for digitization & trigger sim
  double fPedRMS[kNCHFADC];
  double fPMTGain[kNCHFADC];
  double fPMTGainScale[kNCHFADC];
  double fPMTFallTime[kNCHFADC];
  double fPMTTransitTime[kNCHFADC];
  double fPMTTTS[kNCHFADC];
  double fPMTQEff[kNCHFADC];

  int fTRGON;

  ClassDef(FADCTConf, 1)
};

inline void FADCTConf::SetNCH(int val) { fNCH = val; }

inline void FADCTConf::SetRL(int val) { fRL = val; }

inline void FADCTConf::SetTLT(int val) { fTLT = val; }

inline void FADCTConf::SetDSR(int val) { fDSR = val; }

inline void FADCTConf::SetCID(int ch, int val) { fCID[ch] = val; }

inline void FADCTConf::SetPID(int ch, int val) { fPID[ch] = val; }

inline void FADCTConf::SetAMD(int ch, int val) { fAMD[ch] = val; }

inline void FADCTConf::SetPOL(int ch, int val) { fPOL[ch] = val; }

inline void FADCTConf::SetTHR(int ch, int val) { fTHR[ch] = val; }

inline void FADCTConf::SetDLY(int ch, int val) { fDLY[ch] = val; }

inline void FADCTConf::SetDACOFF(int ch, int val) { fDACOFF[ch] = val; }

inline void FADCTConf::SetDT(int ch, int val) { fDT[ch] = val; }

inline void FADCTConf::SetCW(int ch, int val) { fCW[ch] = val; }

inline void FADCTConf::SetTM(int ch, int val) { fTM[ch] = val; }

inline void FADCTConf::SetPCT(int ch, int val) { fPCT[ch] = val; }

inline void FADCTConf::SetPCI(int ch, int val) { fPCI[ch] = val; }

inline void FADCTConf::SetPWT(int ch, int val) { fPWT[ch] = val; }

inline void FADCTConf::SetPSW(int ch, int val) { fPSW[ch] = val; }

inline int FADCTConf::NCH() const { return fNCH; }

inline int FADCTConf::RL() const { return fRL; }

inline int FADCTConf::TLT() const { return fTLT; }

inline int FADCTConf::DSR() const { return fDSR; }

inline int FADCTConf::CID(int ch) const { return fCID[ch]; }

inline int FADCTConf::PID(int ch) const { return fPID[ch]; }

inline int FADCTConf::AMD(int ch) const { return fAMD[ch]; }

inline int FADCTConf::POL(int ch) const { return fPOL[ch]; }

inline int FADCTConf::THR(int ch) const { return fTHR[ch]; }

inline int FADCTConf::DLY(int ch) const { return fDLY[ch]; }

inline int FADCTConf::DACOFF(int ch) const { return fDACOFF[ch]; }

inline int FADCTConf::DT(int ch) const { return fDT[ch]; }

inline int FADCTConf::CW(int ch) const { return fCW[ch]; }

inline int FADCTConf::TM(int ch) const { return fTM[ch]; }

inline int FADCTConf::PCT(int ch) const { return fPCT[ch]; }

inline int FADCTConf::PCI(int ch) const { return fPCI[ch]; }

inline int FADCTConf::PWT(int ch) const { return fPWT[ch]; }

inline int FADCTConf::PSW(int ch) const { return fPSW[ch]; }

inline void FADCTConf::SetPedRMS(int ch, double val) { fPedRMS[ch] = val; }

inline double FADCTConf::GetPedRMS(int ch) const { return fPedRMS[ch]; }

inline void FADCTConf::SetPMTGain(int ch, double val) { fPMTGain[ch] = val; }

inline double FADCTConf::GetPMTGain(int ch) const { return fPMTGain[ch]; }

inline void FADCTConf::SetPMTGainScale(int ch, double val)
{
  fPMTGainScale[ch] = val;
}

inline double FADCTConf::GetPMTGainScale(int ch) const
{
  return fPMTGainScale[ch];
}

inline void FADCTConf::SetPMTFallTime(int ch, double val)
{
  fPMTFallTime[ch] = val;
}

inline double FADCTConf::GetPMTFallTime(int ch) const
{
  return fPMTFallTime[ch];
}

inline void FADCTConf::SetPMTTransitTime(int ch, double val)
{
  fPMTTransitTime[ch] = val;
}

inline double FADCTConf::GetPMTTransitTime(int ch) const
{
  return fPMTTransitTime[ch];
}

inline void FADCTConf::SetPMTTTS(int ch, double val) { fPMTTTS[ch] = val; }

inline double FADCTConf::GetPMTTTS(int ch) const { return fPMTTTS[ch]; }

inline void FADCTConf::SetPMTQEff(int ch, double val) { fPMTQEff[ch] = val; }

inline double FADCTConf::GetPMTQEff(int ch) const { return fPMTQEff[ch]; }

#endif

/**
$Log: FADCTConf.hh,v $
Revision 1.10  2022/12/19 01:01:11  cupsoft
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
