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

#endif
