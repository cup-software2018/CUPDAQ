#ifndef STRGConf_hh
#define STRGConf_hh

#include "TString.h"

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class STRGConf : public AbsConf {
public:
  STRGConf();
  ~STRGConf() override = default;

  void SetZSUMode(int val);
  void SetPrescale(int val);
  void SetInputCard(const char * fname);

  int GetZSUMode() const;
  int GetPrescale() const;
  const char * GetInputCard() const;

  void SetCID(int, int) override {}
  void SetPID(int, int) override {}
  int CID(int) const override { return 0; }
  int PID(int) const override { return 0; }
  void PrintConf() const override {}

private:
  int fZSUMode{};
  int fPrescale{};
  TString fFilename;

  ClassDef(STRGConf, 1)
};

inline void STRGConf::SetZSUMode(int val) { fZSUMode = val; }
inline void STRGConf::SetPrescale(int val) { fPrescale = val; }
inline void STRGConf::SetInputCard(const char * fname) { fFilename = fname; }
inline int STRGConf::GetZSUMode() const { return fZSUMode; }
inline int STRGConf::GetPrescale() const { return fPrescale; }
inline const char * STRGConf::GetInputCard() const { return fFilename.Data(); }

#endif
