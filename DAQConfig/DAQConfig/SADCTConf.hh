#ifndef SADCTConf_hh
#define SADCTConf_hh

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class SADCTConf : public AbsConf {
public:
  SADCTConf();
  explicit SADCTConf(int sid);
  ~SADCTConf() override = default;

  void SetNCH(int val);
  void SetCW(int val);
  void SetGW(int val);
  void SetPSW(int val);
  void SetCID(int ch, int val);
  void SetPID(int ch, int val);
  void SetTHR(int ch, int val);
  void SetDLY(int ch, int val);
  void SetTLT(int group, int val);
  void SetSUBPED(int val);

  int NCH() const;
  int CW() const;
  int GW() const;
  int PSW() const;
  int CID(int ch) const;
  int PID(int ch) const;
  int THR(int ch) const;
  int DLY(int ch) const;
  int TLT(int group) const;
  int SUBPED() const;

  void PrintConf() const override;

protected:
  int fNCH{};
  int fCW{};
  int fGW{};
  int fPSW{};
  int fCID[kNCHSADC]{};
  int fPID[kNCHSADC]{};
  int fTHR[kNCHSADC]{};
  int fDLY[kNCHSADC]{};
  int fTLT[8]{};
  int fSUBPED{};

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
inline int SADCTConf::CW() const { return fCW; }
inline int SADCTConf::GW() const { return fGW; }
inline int SADCTConf::PSW() const { return fPSW; }
inline int SADCTConf::CID(int ch) const { return fCID[ch]; }
inline int SADCTConf::PID(int ch) const { return fPID[ch]; }
inline int SADCTConf::THR(int ch) const { return fTHR[ch]; }
inline int SADCTConf::DLY(int ch) const { return fDLY[ch]; }
inline int SADCTConf::TLT(int group) const { return fTLT[group]; }
inline int SADCTConf::SUBPED() const { return fSUBPED; }

#endif
