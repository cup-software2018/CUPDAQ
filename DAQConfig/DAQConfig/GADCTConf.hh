#ifndef GADCTConf_hh
#define GADCTConf_hh

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class GADCTConf : public AbsConf {
public:
  GADCTConf();
  explicit GADCTConf(int sid);
  ~GADCTConf() override = default;

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

  void PrintConf() const override;

protected:
  int fNCH{};
  int fRL{};
  int fTLT{};
  int fDSR{};

  int fCID[kNCHGADC]{};
  int fPID[kNCHGADC]{};
  int fPOL[kNCHGADC]{};
  int fDACOFF[kNCHGADC]{};
  int fAMD[kNCHGADC]{};
  int fDLY[kNCHGADC]{};
  int fDT[kNCHGADC]{};
  int fCW[kNCHGADC]{};
  int fTM[kNCHGADC]{};
  int fTHR[kNCHGADC]{};
  int fPCT[kNCHGADC]{};
  int fPCI[kNCHGADC]{};
  int fPWT[kNCHGADC]{};
  int fPSW[kNCHGADC]{};

  ClassDef(GADCTConf, 1)
};

inline void GADCTConf::SetNCH(int val) { fNCH = val; }
inline void GADCTConf::SetRL(int val) { fRL = val; }
inline void GADCTConf::SetTLT(int val) { fTLT = val; }
inline void GADCTConf::SetDSR(int val) { fDSR = val; }

inline void GADCTConf::SetCID(int ch, int val) { fCID[ch] = val; }
inline void GADCTConf::SetPID(int ch, int val) { fPID[ch] = val; }
inline void GADCTConf::SetPOL(int ch, int val) { fPOL[ch] = val; }
inline void GADCTConf::SetDACOFF(int ch, int val) { fDACOFF[ch] = val; }
inline void GADCTConf::SetAMD(int ch, int val) { fAMD[ch] = val; }
inline void GADCTConf::SetDLY(int ch, int val) { fDLY[ch] = val; }
inline void GADCTConf::SetDT(int ch, int val) { fDT[ch] = val; }
inline void GADCTConf::SetCW(int ch, int val) { fCW[ch] = val; }
inline void GADCTConf::SetTM(int ch, int val) { fTM[ch] = val; }
inline void GADCTConf::SetTHR(int ch, int val) { fTHR[ch] = val; }
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
inline int GADCTConf::POL(int ch) const { return fPOL[ch]; }
inline int GADCTConf::DACOFF(int ch) const { return fDACOFF[ch]; }
inline int GADCTConf::AMD(int ch) const { return fAMD[ch]; }
inline int GADCTConf::DLY(int ch) const { return fDLY[ch]; }
inline int GADCTConf::DT(int ch) const { return fDT[ch]; }
inline int GADCTConf::CW(int ch) const { return fCW[ch]; }
inline int GADCTConf::TM(int ch) const { return fTM[ch]; }
inline int GADCTConf::THR(int ch) const { return fTHR[ch]; }
inline int GADCTConf::PCT(int ch) const { return fPCT[ch]; }
inline int GADCTConf::PCI(int ch) const { return fPCI[ch]; }
inline int GADCTConf::PWT(int ch) const { return fPWT[ch]; }
inline int GADCTConf::PSW(int ch) const { return fPSW[ch]; }

#endif
