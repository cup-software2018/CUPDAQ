#ifndef FADCSConf_hh
#define FADCSConf_hh

#include "DAQConfig/FADCTConf.hh"
#include "OnlConsts/adcconsts.hh"

class FADCSConf : public FADCTConf {
public:
  FADCSConf();
  explicit FADCSConf(int sid);
  ~FADCSConf() override = default;

  void SetTRGON(int val);
  void SetPTRG(int val);
  void SetPSC(int val);

  int TRGON() const;
  int PTRG() const;
  int PSC() const;

  void PrintConf() const override;

protected:
  int fTRGON{};
  int fPTRG{};
  int fPSC{};

  ClassDef(FADCSConf, 1)
};

inline void FADCSConf::SetTRGON(int val) { fTRGON = val; }

inline void FADCSConf::SetPTRG(int val) { fPTRG = val; }

inline void FADCSConf::SetPSC(int val) { fPSC = val; }

inline int FADCSConf::TRGON() const { return fTRGON; }

inline int FADCSConf::PTRG() const { return fPTRG; }

inline int FADCSConf::PSC() const { return fPSC; }

#endif
