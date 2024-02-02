/*
 *
 *  Module:  TCBConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: Standalone Trigger board configuration class for KIMS
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/07/18 12:25:11 $
 *  CVS/RCS Revision: $Revision: 1.8 $
 *  Status:           $State: Exp $
 *
 */

#ifndef TCBConf_hh
#define TCBConf_hh

#include "DAQConfig/AbsConf.hh"

class TCBConf : public AbsConf {
public:
  TCBConf();
  TCBConf(int sid);
  virtual ~TCBConf();

  virtual void SetTM(int val);
  virtual void SetCW(int val);
  virtual void SetDLY(int val);
  virtual void SetPTRG(int val);
  virtual void SetMTHRF(int val);
  virtual void SetPSCF(int val);
  virtual void SetDTF(int val);
  virtual void SetSWF(int f, int sm, int sl, int i);
  virtual void SetMTHRSM(int val);
  virtual void SetPSCSM(int val);
  virtual void SetDTSM(int val);
  virtual void SetSWSM(int f, int sm, int sl, int i);
  virtual void SetMTHRSL(int val);
  virtual void SetPSCSL(int val);
  virtual void SetDTSL(int val);
  virtual void SetSWSL(int f, int sm, int sl, int i);
  virtual void SetMTHRI(int val);
  virtual void SetPSCI(int val);
  virtual void SetDTI(int val);
  virtual void SetSWI(int f, int sm, int sl, int i);
  virtual void SetTCBTYPE(TCB::TYPE val);

  virtual int TM() const;
  virtual int CW() const;
  virtual int DLY() const;
  virtual int PTRG() const;
  virtual int MTHRF() const;
  virtual int PSCF() const;
  virtual int DTF() const;
  virtual int SWF() const;
  virtual int MTHRSM() const;
  virtual int PSCSM() const;
  virtual int DTSM() const;
  virtual int SWSM() const;
  virtual int MTHRSL() const;
  virtual int PSCSL() const;
  virtual int DTSL() const;
  virtual int SWSL() const;
  virtual int MTHRI() const;
  virtual int PSCI() const;
  virtual int DTI() const;
  virtual int SWI() const;
  virtual TCB::TYPE TCBTYPE() const;

  virtual void SetCID(int ch, int val) {}
  virtual void SetPID(int ch, int val) {}

  virtual int CID(int ch) const { return 0; }
  virtual int PID(int ch) const { return 0; }

  virtual void PrintConf() const;

private:
  int fTM;
  int fCW;
  int fDLY;
  int fPTRG;
  int fMTHRF;
  int fPSCF;
  int fDTF;
  int fSWF;
  int fMTHRSM;
  int fPSCSM;
  int fDTSM;
  int fSWSM;
  int fMTHRSL;
  int fPSCSL;
  int fDTSL;
  int fSWSL;
  int fMTHRI;
  int fPSCI;
  int fDTI;
  int fSWI;
  TCB::TYPE fTCBTYPE;

  ClassDef(TCBConf, 1)
};

inline void TCBConf::SetTM(int val) { fTM = val; }

inline void TCBConf::SetCW(int val) { fCW = val; }

inline void TCBConf::SetDLY(int val) { fDLY = val; }

inline void TCBConf::SetPTRG(int val) { fPTRG = val; }

inline void TCBConf::SetMTHRF(int val) { fMTHRF = val; }

inline void TCBConf::SetPSCF(int val) { fPSCF = val; }

inline void TCBConf::SetDTF(int val) { fDTF = val; }

inline void TCBConf::SetSWF(int f, int sm, int sl, int i)
{
  fSWF = f * 1;
  fSWF += sm * 2;
  fSWF += sl * 4;
  fSWF += i * 8;
}

inline void TCBConf::SetMTHRSM(int val) { fMTHRSM = val; }

inline void TCBConf::SetPSCSM(int val) { fPSCSM = val; }

inline void TCBConf::SetDTSM(int val) { fDTSM = val; }

inline void TCBConf::SetSWSM(int f, int sm, int sl, int i)
{
  fSWSM = f * 1;
  fSWSM += sm * 2;
  fSWSM += sl * 4;
  fSWSM += i * 8;
}

inline void TCBConf::SetMTHRSL(int val) { fMTHRSL = val; }

inline void TCBConf::SetPSCSL(int val) { fPSCSL = val; }

inline void TCBConf::SetDTSL(int val) { fDTSL = val; }

inline void TCBConf::SetSWSL(int f, int sm, int sl, int i)
{
  fSWSL = f * 1;
  fSWSL += sm * 2;
  fSWSL += sl * 4;
  fSWSL += i * 8;
}

inline void TCBConf::SetMTHRI(int val) { fMTHRI = val; }

inline void TCBConf::SetPSCI(int val) { fPSCI = val; }

inline void TCBConf::SetDTI(int val) { fDTI = val; }

inline void TCBConf::SetSWI(int f, int sm, int sl, int i)
{
  fSWI = f * 1;
  fSWI += sm * 2;
  fSWI += sl * 4;
  fSWI += i * 8;
}

inline void TCBConf::SetTCBTYPE(TCB::TYPE val) { fTCBTYPE = val; }

inline int TCBConf::TM() const { return fTM; }

inline int TCBConf::CW() const { return fCW; }

inline int TCBConf::DLY() const { return fDLY; }

inline int TCBConf::PTRG() const { return fPTRG; }

inline int TCBConf::MTHRF() const { return fMTHRF; }

inline int TCBConf::PSCF() const { return fPSCF; }

inline int TCBConf::DTF() const { return fDTF; }

inline int TCBConf::SWF() const { return fSWF; }

inline int TCBConf::MTHRSM() const { return fMTHRSM; }

inline int TCBConf::PSCSM() const { return fPSCSM; }

inline int TCBConf::DTSM() const { return fDTSM; }

inline int TCBConf::SWSM() const { return fSWSM; }

inline int TCBConf::MTHRSL() const { return fMTHRSL; }

inline int TCBConf::PSCSL() const { return fPSCSL; }

inline int TCBConf::DTSL() const { return fDTSL; }

inline int TCBConf::SWSL() const { return fSWSL; }

inline int TCBConf::MTHRI() const { return fMTHRI; }

inline int TCBConf::PSCI() const { return fPSCI; }

inline int TCBConf::DTI() const { return fDTI; }

inline int TCBConf::SWI() const { return fSWI; }

inline TCB::TYPE TCBConf::TCBTYPE() const { return fTCBTYPE; }

#endif

/**
$Log: TCBConf.hh,v $
Revision 1.8  2023/07/18 12:25:11  cupsoft
*** empty log message ***

Revision 1.7  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.6  2019/11/25 02:04:07  cupsoft
*** empty log message ***

Revision 1.5  2018/01/08 05:14:45  cupsoft
*** empty log message ***

Revision 1.4  2017/05/19 01:58:58  cupsoft
*** empty log message ***

Revision 1.3  2017/03/08 05:21:44  cupsoft
*** empty log message ***

Revision 1.2  2016/12/05 09:52:50  cupsoft
*** empty log message ***

Revision 1.1  2016/10/03 06:57:03  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/09/30 09:22:24  cupsoft
DAQConfig

Revision 1.3  2016/09/27 03:41:52  cupsoft
*** empty log message ***

Revision 1.2  2016/09/05 05:26:49  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/07/14 07:42:30  cupsoft
DAQSys

Revision 1.1.1.1  2016/03/08 04:37:46  amore
DAQSts

Revision 1.1.1.1  2016/02/29 08:26:44  cupsoft
DAQSys

**/
