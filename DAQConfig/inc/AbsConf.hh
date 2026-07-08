#pragma once

#include "TNamed.h"

#include "adcconsts.hh"

class AbsConf : public TNamed {
public:
  AbsConf() = default;
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

  void SetZSU(bool val);
  bool ZSU() const;

  // Per-channel zero suppression threshold. Not every ADC type supports it
  // yet, so the default is a no-op override rather than a pure virtual.
  virtual int THR(int ch) const;

  void SetADCType(ADC::TYPE type);
  ADC::TYPE GetADCType() const;

  void SetDAQID(int id);
  int GetDAQID() const;

  int Compare(const TObject * object) const override;
  bool IsSortable() const override { return true; }
  const char * InfoStr() const;

  virtual void PrintConf() const = 0;

protected:
  bool fIsEnabled{false};
  bool fIsLinked{false};
  bool fZSU{false};

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

inline void AbsConf::SetZSU(bool val) { fZSU = val; }
inline bool AbsConf::ZSU() const { return fZSU; }

inline int AbsConf::THR(int /*ch*/) const { return 0; }

inline void AbsConf::SetSID(int id) { fSID = id; }
inline void AbsConf::SetMID(int id) { fMID = id; }

inline int AbsConf::SID() const { return fSID; }
inline int AbsConf::MID() const { return fMID; }

inline void AbsConf::SetADCType(ADC::TYPE type) { fADCType = type; }
inline ADC::TYPE AbsConf::GetADCType() const { return fADCType; }

inline void AbsConf::SetDAQID(int id) { fDAQID = id; }
inline int AbsConf::GetDAQID() const { return fDAQID; }
