#pragma once

#include "DAQConfig/AbsConf.hh"
#include "AMORE/amoreconsts.hh"

class AMOREADCConf : public AbsConf {
public:
  AMOREADCConf() = default;
  explicit AMOREADCConf(int sid);
  ~AMOREADCConf() override = default;

  void SetNCH(int val);
  void SetCID(int ch, int val) override;
  void SetPID(int ch, int val) override;
  void SetTRGON(int ch, int val);
  void SetSR(int val);
  void SetRL(int val);
  void SetDLY(int val);
  void SetZSU(int val);

  int NCH() const;
  int CID(int ch) const override;
  int PID(int ch) const override;
  int TRGON(int ch) const;
  int SR() const;
  int RL() const;
  int DLY() const;
  int ZSU() const;

  void PrintConf() const override;

private:
  int fNCH{AMORE::kNCHPERADC};
  int fSR{};
  int fRL{};
  int fDLY{};
  int fZSU{};
  int fCID[AMORE::kNCHPERADC]{};
  int fPID[AMORE::kNCHPERADC]{};
  int fTRGON[AMORE::kNCHPERADC]{};

  ClassDef(AMOREADCConf, 1)
};

inline void AMOREADCConf::SetNCH(int val) { fNCH = val; }

inline void AMOREADCConf::SetSR(int val) { fSR = val; }

inline void AMOREADCConf::SetRL(int val) { fRL = val; }

inline void AMOREADCConf::SetDLY(int val) { fDLY = val; }

inline void AMOREADCConf::SetZSU(int val) { fZSU = val; }

inline void AMOREADCConf::SetCID(int ch, int val) { fCID[ch] = val; }

inline void AMOREADCConf::SetPID(int ch, int val) { fPID[ch] = val; }

inline void AMOREADCConf::SetTRGON(int ch, int val) { fTRGON[ch] = val; }

inline int AMOREADCConf::NCH() const { return fNCH; }

inline int AMOREADCConf::SR() const { return fSR; }

inline int AMOREADCConf::RL() const { return fRL; }

inline int AMOREADCConf::DLY() const { return fDLY; }

inline int AMOREADCConf::ZSU() const { return fZSU; }

inline int AMOREADCConf::CID(int ch) const { return fCID[ch]; }

inline int AMOREADCConf::PID(int ch) const { return fPID[ch]; }

inline int AMOREADCConf::TRGON(int ch) const { return fTRGON[ch]; }

