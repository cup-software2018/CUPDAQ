#pragma once

#include "DAQConfig/AbsConf.hh"

class FADCTConf : public AbsConf {
public:
  FADCTConf();
  explicit FADCTConf(int sid);
  ~FADCTConf() override = default;

  void SetNCH(int val);
  void SetRL(int val);
  void SetTLT(int val);
  void SetDSR(int val);

  void SetCID(int ch, int val) override;
  void SetPID(int ch, int val) override;
  void SetPOL(int ch, int val);
  void SetDACOFF(int ch, int val);
  void SetAMD(int ch, int val);
  void SetDLY(int ch, int val);
  void SetDT(int ch, int val);
  void SetCW(int ch, int val);
  void SetTM(int ch, int val);
  void SetTHR(int ch, int val);
  void SetPCT(int ch, int val);
  void SetPCI(int ch, int val);
  void SetPWT(int ch, int val);
  void SetPSW(int ch, int val);

  int NCH() const;
  int RL() const;
  int TLT() const;
  int DSR() const;
  int CID(int ch) const override;
  int PID(int ch) const override;
  int POL(int ch) const;
  int DACOFF(int ch) const;
  int AMD(int ch) const;
  int DLY(int ch) const;
  int DT(int ch) const;
  int CW(int ch) const;
  int TM(int ch) const;
  int THR(int ch) const;
  int PCT(int ch) const;
  int PCI(int ch) const;
  int PWT(int ch) const;
  int PSW(int ch) const;

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

  void SetTRGON(int val);
  int TRGON() const;

  void PrintConf() const override;

protected:
  int fNCH{};
  int fRL{};
  int fTLT{};
  int fDSR{1};

  int fCID[kNCHFADC]{};
  int fPID[kNCHFADC]{};
  int fPOL[kNCHFADC]{};
  int fDACOFF[kNCHFADC]{};
  int fAMD[kNCHFADC]{};
  int fDLY[kNCHFADC]{};
  int fDT[kNCHFADC]{};
  int fCW[kNCHFADC]{};
  int fTM[kNCHFADC]{};
  int fTHR[kNCHFADC]{};
  int fPCT[kNCHFADC]{};
  int fPCI[kNCHFADC]{};
  int fPWT[kNCHFADC]{};
  int fPSW[kNCHFADC]{};

  double fPedRMS[kNCHFADC]{};
  double fPMTGain[kNCHFADC]{};
  double fPMTGainScale[kNCHFADC]{};
  double fPMTFallTime[kNCHFADC]{};
  double fPMTTransitTime[kNCHFADC]{};
  double fPMTTTS[kNCHFADC]{};
  double fPMTQEff[kNCHFADC]{};

  int fTRGON{};

  ClassDef(FADCTConf, 1)
};

inline void FADCTConf::SetNCH(int val) { fNCH = val; }
inline void FADCTConf::SetRL(int val) { fRL = val; }
inline void FADCTConf::SetTLT(int val) { fTLT = val; }
inline void FADCTConf::SetDSR(int val) { fDSR = val; }

inline void FADCTConf::SetCID(int ch, int val) { fCID[ch] = val; }
inline void FADCTConf::SetPID(int ch, int val) { fPID[ch] = val; }
inline void FADCTConf::SetPOL(int ch, int val) { fPOL[ch] = val; }
inline void FADCTConf::SetDACOFF(int ch, int val) { fDACOFF[ch] = val; }
inline void FADCTConf::SetAMD(int ch, int val) { fAMD[ch] = val; }
inline void FADCTConf::SetDLY(int ch, int val) { fDLY[ch] = val; }
inline void FADCTConf::SetDT(int ch, int val) { fDT[ch] = val; }
inline void FADCTConf::SetCW(int ch, int val) { fCW[ch] = val; }
inline void FADCTConf::SetTM(int ch, int val) { fTM[ch] = val; }
inline void FADCTConf::SetTHR(int ch, int val) { fTHR[ch] = val; }
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
inline int FADCTConf::POL(int ch) const { return fPOL[ch]; }
inline int FADCTConf::DACOFF(int ch) const { return fDACOFF[ch]; }
inline int FADCTConf::AMD(int ch) const { return fAMD[ch]; }
inline int FADCTConf::DLY(int ch) const { return fDLY[ch]; }
inline int FADCTConf::DT(int ch) const { return fDT[ch]; }
inline int FADCTConf::CW(int ch) const { return fCW[ch]; }
inline int FADCTConf::TM(int ch) const { return fTM[ch]; }
inline int FADCTConf::THR(int ch) const { return fTHR[ch]; }
inline int FADCTConf::PCT(int ch) const { return fPCT[ch]; }
inline int FADCTConf::PCI(int ch) const { return fPCI[ch]; }
inline int FADCTConf::PWT(int ch) const { return fPWT[ch]; }
inline int FADCTConf::PSW(int ch) const { return fPSW[ch]; }

inline void FADCTConf::SetPedRMS(int ch, double val) { fPedRMS[ch] = val; }
inline double FADCTConf::GetPedRMS(int ch) const { return fPedRMS[ch]; }

inline void FADCTConf::SetPMTGain(int ch, double val) { fPMTGain[ch] = val; }
inline double FADCTConf::GetPMTGain(int ch) const { return fPMTGain[ch]; }

inline void FADCTConf::SetPMTGainScale(int ch, double val) { fPMTGainScale[ch] = val; }
inline double FADCTConf::GetPMTGainScale(int ch) const { return fPMTGainScale[ch]; }

inline void FADCTConf::SetPMTFallTime(int ch, double val) { fPMTFallTime[ch] = val; }
inline double FADCTConf::GetPMTFallTime(int ch) const { return fPMTFallTime[ch]; }

inline void FADCTConf::SetPMTTransitTime(int ch, double val) { fPMTTransitTime[ch] = val; }
inline double FADCTConf::GetPMTTransitTime(int ch) const { return fPMTTransitTime[ch]; }

inline void FADCTConf::SetPMTTTS(int ch, double val) { fPMTTTS[ch] = val; }
inline double FADCTConf::GetPMTTTS(int ch) const { return fPMTTTS[ch]; }

inline void FADCTConf::SetPMTQEff(int ch, double val) { fPMTQEff[ch] = val; }
inline double FADCTConf::GetPMTQEff(int ch) const { return fPMTQEff[ch]; }

inline void FADCTConf::SetTRGON(int val) { fTRGON = val; }
inline int FADCTConf::TRGON() const { return fTRGON; }
