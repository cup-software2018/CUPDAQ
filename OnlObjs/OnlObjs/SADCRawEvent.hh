#ifndef SADCRawEvent_hh
#define SADCRawEvent_hh

#include "OnlObjs/AbsADCRaw.hh"

class SADCRawEvent : public AbsADCRaw {
private:
  int fNCH;
  unsigned int * fADC;  //[fNCH]
  unsigned int * fTime; //[fNCH]

public:
  SADCRawEvent();
  SADCRawEvent(int s, ADC::TYPE type);
  SADCRawEvent(const SADCRawEvent & raw);
  virtual ~SADCRawEvent();

  virtual void Unpack(AbsConf * config, int verbose = 0);

  int GetNCH() const;
  unsigned int GetADC(int n) const;
  unsigned int * GetADCs() const;
  unsigned int GetTime(int n) const;
  unsigned int * GetTimes() const;

private:
  void Unpack_SADC(AbsConf * config, int verbose = 0);
  void Unpack_IADC(AbsConf * config, int verbose = 0);
  void UnpackHeader_SADC(ADCHeader * header);
  void UnpackHeader_IADC(ADCHeader * header);

  ClassDef(SADCRawEvent, 1)
};

inline int SADCRawEvent::GetNCH() const { return fNCH; }

inline unsigned int SADCRawEvent::GetADC(int n) const { return fADC[n]; }

inline unsigned int * SADCRawEvent::GetADCs() const { return fADC; }

inline unsigned int SADCRawEvent::GetTime(int n) const { return fTime[n]; }

inline unsigned int * SADCRawEvent::GetTimes() const { return fTime; }

#endif
