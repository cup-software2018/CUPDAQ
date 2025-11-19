#ifndef GADCSConf_hh
#define GADCSConf_hh

#include "DAQConfig/GADCTConf.hh"
#include "OnlConsts/adcconsts.hh"

class GADCSConf : public GADCTConf {
public:
  GADCSConf();
  explicit GADCSConf(int sid);
  ~GADCSConf() override = default;

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

  ClassDef(GADCSConf, 1)
};

inline void GADCSConf::SetTRGON(int val) { fTRGON = val; }
inline void GADCSConf::SetPTRG(int val) { fPTRG = val; }
inline void GADCSConf::SetPSC(int val) { fPSC = val; }

inline int GADCSConf::TRGON() const { return fTRGON; }
inline int GADCSConf::PTRG() const { return fPTRG; }
inline int GADCSConf::PSC() const { return fPSC; }

#endif
