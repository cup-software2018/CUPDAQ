#ifndef SADCSConf_hh
#define SADCSConf_hh

#include "DAQConfig/SADCTConf.hh"
#include "OnlConsts/adcconsts.hh"

class SADCSConf : public SADCTConf {
public:
  SADCSConf();
  explicit SADCSConf(int sid);
  ~SADCSConf() override = default;

  void SetPTRG(int val);
  void SetMTHR(int val);
  void SetPSC(int val);

  int PTRG() const;
  int MTHR() const;
  int PSC() const;

  void PrintConf() const override;

protected:
  int fPTRG{};
  int fMTHR{};
  int fPSC{};

  ClassDef(SADCSConf, 1)
};

inline void SADCSConf::SetPTRG(int val) { fPTRG = val; }
inline void SADCSConf::SetMTHR(int val) { fMTHR = val; }
inline void SADCSConf::SetPSC(int val) { fPSC = val; }

inline int SADCSConf::PTRG() const { return fPTRG; }
inline int SADCSConf::MTHR() const { return fMTHR; }
inline int SADCSConf::PSC() const { return fPSC; }

#endif
