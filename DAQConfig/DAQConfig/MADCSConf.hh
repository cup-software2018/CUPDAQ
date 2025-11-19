#ifndef MADCSConf_hh
#define MADCSConf_hh

#include "DAQConfig/FADCTConf.hh"
#include "OnlConsts/adcconsts.hh"

class MADCSConf : public FADCTConf {
public:
  MADCSConf();
  explicit MADCSConf(int sid);
  ~MADCSConf() override = default;

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

  ClassDef(MADCSConf, 1)
};

inline void MADCSConf::SetTRGON(int val) { fTRGON = val; }
inline void MADCSConf::SetPTRG(int val) { fPTRG = val; }
inline void MADCSConf::SetPSC(int val) { fPSC = val; }

inline int MADCSConf::TRGON() const { return fTRGON; }
inline int MADCSConf::PTRG() const { return fPTRG; }
inline int MADCSConf::PSC() const { return fPSC; }

#endif
