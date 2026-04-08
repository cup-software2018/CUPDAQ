#pragma once

#include <cstring>

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
  int fSize;
  unsigned char * fData;

public:
  AbsADCRaw();
  AbsADCRaw(int s, ADC::TYPE type);
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
  delete[] fData;
  fData = new unsigned char[fSize];
  std::memset(fData, 0, static_cast<std::size_t>(fSize));
}

inline void AbsADCRaw::SetADCType(ADC::TYPE type) { fType = type; }

inline void AbsADCRaw::SetTimeCalConsts(TimeCalConsts * c)
{
  if (fHeader) { fHeader->SetTimeCalConsts(c); }
}

inline ADC::TYPE AbsADCRaw::GetADCType() const { return fType; }

inline ADC::MODE AbsADCRaw::GetADCMode() const { return fMode; }

inline int AbsADCRaw::GetRawDataSize() const { return fSize; }

inline unsigned char * AbsADCRaw::GetRawData() const { return fData; }

inline ADCHeader * AbsADCRaw::GetADCHeader() const { return fHeader; }

inline unsigned int AbsADCRaw::GetTriggerType() const { return fHeader ? fHeader->GetTriggerType() : 0U; }

inline unsigned int AbsADCRaw::GetTriggerNumber() const { return fHeader ? fHeader->GetLocalTriggerNumber() : 0U; }

inline unsigned long AbsADCRaw::GetTriggerTime() const { return fHeader ? fHeader->GetLocalTriggerTime() : 0UL; }
