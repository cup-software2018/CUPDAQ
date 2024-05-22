#ifndef AbsADCRaw_hh
#define AbsADCRaw_hh

#include "TObject.h"

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"
#include "OnlObjs/ADCHeader.hh"
#include "OnlObjs/TimeCalConsts.hh"

class AbsADCRaw : public TObject {
protected:
  ADC::TYPE fType;
  ADC::MODE fMode;
  ADCHeader * fHeader;
  int fSize; //
  unsigned char * fData; //[fSize]

public:
  AbsADCRaw();
  AbsADCRaw(int s, ADC::TYPE);
  AbsADCRaw(const AbsADCRaw & raw);
  virtual ~AbsADCRaw();

  virtual void SetRawDataSize(int size);
  virtual void SetADCType(ADC::TYPE type);
  virtual void CopyDataFrom(unsigned char * raw);
  virtual void SetTimeCalConsts(TimeCalConsts * c);

  virtual void Unpack(AbsConf * conf, int verbose) = 0;

  virtual int GetRawDataSize() const;
  virtual unsigned char * GetRawData() const;
  virtual ADC::TYPE GetADCType() const;
  virtual ADC::MODE GetADCMode() const;
  virtual ADCHeader * GetADCHeader() const;
  virtual unsigned int GetTriggerType() const;
  virtual unsigned int GetTriggerNumber() const;
  virtual unsigned long GetTriggerTime() const;

  virtual void PrintHeader() const;

  ClassDef(AbsADCRaw, 1)
};

inline void AbsADCRaw::SetRawDataSize(int size)
{
  fSize = size;
  if (fData) delete[] fData;
  fData = new unsigned char[fSize];
  memset(fData, 0, fSize * sizeof(unsigned char));
}
inline void AbsADCRaw::SetADCType(ADC::TYPE type) { fType = type; }
inline void AbsADCRaw::SetTimeCalConsts(TimeCalConsts * c)
{
  fHeader->SetTimeCalConsts(c);
}
inline ADC::TYPE AbsADCRaw::GetADCType() const { return fType; }
inline ADC::MODE AbsADCRaw::GetADCMode() const { return fMode; }
inline int AbsADCRaw::GetRawDataSize() const { return fSize; }
inline unsigned char * AbsADCRaw::GetRawData() const { return fData; }
inline ADCHeader * AbsADCRaw::GetADCHeader() const { return fHeader; }
inline unsigned int AbsADCRaw::GetTriggerType() const
{
  return fHeader->GetTriggerType();
}
inline unsigned int AbsADCRaw::GetTriggerNumber() const
{
  return fHeader->GetLocalTriggerNumber();
}
inline unsigned long AbsADCRaw::GetTriggerTime() const
{
  return fHeader->GetLocalTriggerTime();
}

#endif
