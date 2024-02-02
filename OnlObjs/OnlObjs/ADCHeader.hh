#ifndef ADCHeader_hh
#define ADCHeader_hh

#include "OnlObjs/TimeCalConsts.hh"
#include "TObject.h"

class ADCHeader : public TObject {
public:
  ADCHeader();
  ADCHeader(const ADCHeader & header);
  virtual ~ADCHeader();

  void SetMID(int n);
  void SetCID(int n);
  void SetError();
  void SetZero(int n);
  void SetTriggerBit(int n);
  void SetPedestal(int n, int p);
  void SetDataLength(unsigned int n);
  void SetTriggerType(unsigned int n);
  void SetTriggerNumber(unsigned int n);
  void SetTriggerTime(unsigned long n);
  void SetLocalTriggerPattern(unsigned int n);
  void SetLocalTriggerNumber(unsigned int n);
  void SetLocalTriggerTime(unsigned long n);
  void SetTimeCalConsts(TimeCalConsts * consts);

  int GetMID() const;
  int GetCID() const;
  bool GetError() const;
  bool GetZero(int n) const;
  bool * GetZero() const;
  bool GetTriggerBit(int n) const;
  bool * GetTriggerBit() const;
  unsigned int GetPedestal(int n) const;
  unsigned int * GetPedestal() const;
  unsigned int GetDataLength() const;
  unsigned int GetTriggerType() const;
  unsigned int GetTriggerNumber() const;
  unsigned long GetTriggerTime() const;
  unsigned int GetLocalTriggerPattern() const;
  unsigned int GetLocalTriggerNumber() const;
  unsigned long GetLocalTriggerTime() const;
  TimeCalConsts * GetTimeCalConsts() const;

  virtual void Print(Option_t * opt = "") const;

private:
  int kNMAXADCCH;

  bool error;
  bool * zero; //[kNMAXADCCH]
  bool * tbit; //[kNMAXADCCH]

  int mid;
  int cid;

  unsigned int ttype;
  unsigned int dlen;
  unsigned int * ped; //[kNMAXADCCH]
  unsigned int tnum;
  unsigned int loctnum;
  unsigned int loctptn;
  unsigned long ttime;
  unsigned long locttime;

  TimeCalConsts * fCalConsts; //!

  ClassDef(ADCHeader, 1)
};

inline void ADCHeader::SetMID(int n) { mid = n; }
inline void ADCHeader::SetCID(int n) { cid = n; }
inline void ADCHeader::SetError()
{
  if (!error) error = true;
}
inline void ADCHeader::SetZero(int n) { zero[n] = true; }
inline void ADCHeader::SetTriggerBit(int n) { tbit[n] = true; }
inline void ADCHeader::SetPedestal(int n, int p) { ped[n] = p; }
inline void ADCHeader::SetDataLength(unsigned int n) { dlen = n; }
inline void ADCHeader::SetTriggerType(unsigned int n) { ttype = n; }
inline void ADCHeader::SetTriggerNumber(unsigned int n) { tnum = n; }
inline void ADCHeader::SetTriggerTime(unsigned long n) { ttime = n; }
inline void ADCHeader::SetLocalTriggerPattern(unsigned int n) { loctptn = n; }
inline void ADCHeader::SetLocalTriggerNumber(unsigned int n) { loctnum = n; }
inline void ADCHeader::SetLocalTriggerTime(unsigned long n)
{
  locttime = n;
  if (fCalConsts) { locttime += fCalConsts->GetConst(mid); }
}
inline void ADCHeader::SetTimeCalConsts(TimeCalConsts * consts)
{
  fCalConsts = consts;
}
inline int ADCHeader::GetMID() const { return mid; }
inline int ADCHeader::GetCID() const { return cid; }
inline bool ADCHeader::GetError() const { return error; }
inline bool ADCHeader::GetZero(int n) const { return zero[n]; }
inline bool * ADCHeader::GetZero() const { return zero; }
inline bool ADCHeader::GetTriggerBit(int n) const { return tbit[n]; }
inline bool * ADCHeader::GetTriggerBit() const { return tbit; }
inline unsigned int ADCHeader::GetPedestal(int n) const { return ped[n]; }
inline unsigned int * ADCHeader::GetPedestal() const { return ped; }
inline unsigned int ADCHeader::GetDataLength() const { return dlen; }
inline unsigned int ADCHeader::GetTriggerType() const { return ttype; }
inline unsigned int ADCHeader::GetTriggerNumber() const { return tnum; }
inline unsigned long ADCHeader::GetTriggerTime() const { return ttime; }
inline unsigned int ADCHeader::GetLocalTriggerPattern() const { return loctptn; }
inline unsigned int ADCHeader::GetLocalTriggerNumber() const { return loctnum; }
inline unsigned long ADCHeader::GetLocalTriggerTime() const { return locttime; }
inline TimeCalConsts * ADCHeader::GetTimeCalConsts() const
{
  return fCalConsts;
}

#endif