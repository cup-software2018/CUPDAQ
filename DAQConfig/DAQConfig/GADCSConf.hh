#ifndef GADCSConf_hh
#define GADCSConf_hh

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class GADCSConf : public AbsConf {
public:
  GADCSConf();
  explicit GADCSConf(int sid);
  ~GADCSConf() override = default;

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

  void SetTRGON(int val);
  void SetPTRG(int val);
  void SetPSC(int val);

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

  int TRGON() const;
  int PTRG() const;
  int PSC() const;

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

  int fTRGON{};
  int fPTRG{};
  int fPSC{};

  ClassDef(GADCSConf, 1)
};

inline void GADCSConf::SetNCH(int val) { fNCH = val; }
inline void GADCSConf::SetRL(int val) { fRL = val; }
inline void GADCSConf::SetTLT(int val) { fTLT = val; }
inline void GADCSConf::SetDSR(int val) { fDSR = val; }
inline void GADCSConf::SetCID(int ch, int val) { fCID[ch] = val; }
inline void GADCSConf::SetPID(int ch, int val) { fPID[ch] = val; }
inline void GADCSConf::SetPOL(int ch, int val) { fPOL[ch] = val; }
inline void GADCSConf::SetDACOFF(int ch, int val) { fDACOFF[ch] = val; }
inline void GADCSConf::SetAMD(int ch, int val) { fAMD[ch] = val; }
inline void GADCSConf::SetDLY(int ch, int val) { fDLY[ch] = val; }
inline void GADCSConf::SetDT(int ch, int val) { fDT[ch] = val; }
inline void GADCSConf::SetCW(int ch, int val) { fCW[ch] = val; }
inline void GADCSConf::SetTM(int ch, int val) { fTM[ch] = val; }
inline void GADCSConf::SetTHR(int ch, int val) { fTHR[ch] = val; }
inline void GADCSConf::SetPCT(int ch, int val) { fPCT[ch] = val; }
inline void GADCSConf::SetPCI(int ch, int val) { fPCI[ch] = val; }
inline void GADCSConf::SetPWT(int ch, int val) { fPWT[ch] = val; }
inline void GADCSConf::SetPSW(int ch, int val) { fPSW[ch] = val; }
inline void GADCSConf::SetTRGON(int val) { fTRGON = val; }
inline void GADCSConf::SetPTRG(int val) { fPTRG = val; }
inline void GADCSConf::SetPSC(int val) { fPSC = val; }

inline int GADCSConf::NCH() const { return fNCH; }
inline int GADCSConf::RL() const { return fRL; }
inline int GADCSConf::TLT() const { return fTLT; }
inline int GADCSConf::DSR() const { return fDSR; }
inline int GADCSConf::CID(int ch) const { return fCID[ch]; }
inline int GADCSConf::PID(int ch) const { return fPID[ch]; }
inline int GADCSConf::POL(int ch) const { return fPOL[ch]; }
inline int GADCSConf::DACOFF(int ch) const { return fDACOFF[ch]; }
inline int GADCSConf::AMD(int ch) const { return fAMD[ch]; }
inline int GADCSConf::DLY(int ch) const { return fDLY[ch]; }
inline int GADCSConf::DT(int ch) const { return fDT[ch]; }
inline int GADCSConf::CW(int ch) const { return fCW[ch]; }
inline int GADCSConf::TM(int ch) const { return fTM[ch]; }
inline int GADCSConf::THR(int ch) const { return fTHR[ch]; }
inline int GADCSConf::PCT(int ch) const { return fPCT[ch]; }
inline int GADCSConf::PCI(int ch) const { return fPCI[ch]; }
inline int GADCSConf::PWT(int ch) const { return fPWT[ch]; }
inline int GADCSConf::PSW(int ch) const { return fPSW[ch]; }
inline int GADCSConf::TRGON() const { return fTRGON; }
inline int GADCSConf::PTRG() const { return fPTRG; }
inline int GADCSConf::PSC() const { return fPSC; }

#endif
