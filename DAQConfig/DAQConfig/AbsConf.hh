#ifndef AbsConf_hh
#define AbsConf_hh

#include "TNamed.h"

#include "OnlConsts/adcconsts.hh"

class AbsConf : public TNamed {
public:
  AbsConf();
  AbsConf(int sid, ADC::TYPE type = ADC::TCB);
  ~AbsConf() override = default;

  void SetEnable();
  bool IsEnabled() const;

  void SetLink();
  bool IsLinked() const;

  void SetSID(int id);
  void SetMID(int id);

  int SID() const;
  int MID() const;

  virtual void SetCID(int ch, int val) = 0;
  virtual void SetPID(int ch, int val) = 0;

  virtual int CID(int ch) const = 0;
  virtual int PID(int ch) const = 0;

  void SetADCType(ADC::TYPE type);
  ADC::TYPE GetADCType() const;

  void SetDAQID(int id);
  int GetDAQID() const;

  int Compare(const TObject * object) const override;
  const char * InfoStr() const;

  virtual void PrintConf() const = 0;

protected:
  bool fIsEnabled{false};
  bool fIsLinked{false};

  int fSID{0};
  int fMID{0};

  ADC::TYPE fADCType{ADC::TCB};
  int fDAQID{0};

  ClassDef(AbsConf, 1)
};

inline void AbsConf::SetEnable() { fIsEnabled = true; }
inline bool AbsConf::IsEnabled() const { return fIsEnabled; }
inline void AbsConf::SetLink() { fIsLinked = true; }
inline bool AbsConf::IsLinked() const { return fIsLinked; }

inline void AbsConf::SetSID(int id) { fSID = id; }
inline void AbsConf::SetMID(int id) { fMID = id; }

inline int AbsConf::SID() const { return fSID; }
inline int AbsConf::MID() const { return fMID; }

inline void AbsConf::SetADCType(ADC::TYPE type) { fADCType = type; }
inline ADC::TYPE AbsConf::GetADCType() const { return fADCType; }

inline void AbsConf::SetDAQID(int id) { fDAQID = id; }
inline int AbsConf::GetDAQID() const { return fDAQID; }

#endif
