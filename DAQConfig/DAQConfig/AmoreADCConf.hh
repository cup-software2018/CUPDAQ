#ifndef AmoreADCConf_hh
#define AmoreADCConf_hh

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class AmoreADCConf : public AbsConf {
public:
  AmoreADCConf();
  explicit AmoreADCConf(int sid);
  ~AmoreADCConf() override = default;

  void SetNCH(int val);
  void SetSR(int val);
  void SetRL(int val);
  void SetDLY(int val);
  void SetCW(int val);
  void SetSKBIN(int val);
  void SetZEROSUP(int val);
  void SetCID(int ch, int val) override;
  void SetPID(int ch, int val) override;
  void SetTRGON(int ch, int val);
  void SetORDER(int ch, int val);
  void SetLOWER(int ch, int val);
  void SetUPPER(int ch, int val);
  void SetTHR(int ch, int val);
  void SetDT(int ch, int val);

  int NCH() const;
  int SR() const;
  int RL() const;
  int DLY() const;
  int CW() const;
  int SKBIN() const;
  int ZEROSUP() const;
  int CID(int ch) const override;
  int PID(int ch) const override;
  int TRGON(int ch) const;
  int ORDER(int ch) const;
  int LOWER(int ch) const;
  int UPPER(int ch) const;
  int THR(int ch) const;
  int DT(int ch) const;

  void PrintConf() const override;

private:
  int fNCH{};
  int fSR{};
  int fRL{};
  int fDLY{};
  int fCW{};
  int fSKBIN{};
  int fZEROSUP{};
  int fCID[kNCHAMOREADC]{};
  int fPID[kNCHAMOREADC]{};
  int fTRGON[kNCHAMOREADC]{};
  int fORDER[kNCHAMOREADC]{};
  int fLOWER[kNCHAMOREADC]{};
  int fUPPER[kNCHAMOREADC]{};
  int fTHR[kNCHAMOREADC]{};
  int fDT[kNCHAMOREADC]{};

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
