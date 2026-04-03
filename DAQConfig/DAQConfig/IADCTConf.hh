#pragma once
#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class IADCTConf : public AbsConf {
public:
  IADCTConf();
  explicit IADCTConf(int sid);
  ~IADCTConf() override = default;

  void SetNCH(int val);
  void SetMODE(int val);
  void SetRL(int val);
  void SetCW(int val);
  void SetGW(int val);
  void SetPSW(int val);

  void SetCID(int ch, int val) override;
  void SetPID(int ch, int val) override;
  void SetTHR(int ch, int val);
  void SetDLY(int ch, int val);
  void SetTLT(int group, int val);
  void SetHV(int group, float val);

  int NCH() const;
  int MODE() const;

  int RL() const;
  int CW() const;
  int GW() const;
  int PSW() const;

  int CID(int ch) const override;
  int PID(int ch) const override;
  int THR(int ch) const;
  int DLY(int ch) const;
  int TLT(int group) const;
  float HV(int group) const;

  void PrintConf() const override;

protected:
  int fNCH{};
  int fMODE{};
  int fRL{};
  int fCW{};
  int fGW{};
  int fPSW{};

  int fCID[kNCHIADC]{};
  int fPID[kNCHIADC]{};
  int fTHR[kNCHIADC]{};
  int fDLY[kNCHIADC]{};

  int fTLT[10]{};
  float fHV[5]{};

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
