#ifndef FADCRawEvent_hh
#define FADCRawEvent_hh

#include "OnlObjs/AbsADCRaw.hh"

class FADCRawChannel;
class FADCRawEvent : public AbsADCRaw {
private:
  int fNCH;
  int fNDP;
  FADCRawChannel ** fChannel; //[fNCH]

public:
  FADCRawEvent();
  FADCRawEvent(int n, int s, ADC::TYPE type);
  FADCRawEvent(const FADCRawEvent & event);
  virtual ~FADCRawEvent();

  virtual void Unpack(AbsConf * conf, int verbose = 0);

  int GetNCH() const;
  int GetNDP() const;
  FADCRawChannel * GetChannel(int n) const;

private:
  void Unpack_FADC(AbsConf * conf, int verbose);
  void Unpack_MADC(AbsConf * conf, int verbose);
  void Unpack_GADC(AbsConf * conf, int verbose);
  void Unpack_IADC(AbsConf * conf, int verbose);
  void UnpackHeader_FADC(int n, ADCHeader * header);
  void UnpackHeader_MADC(int n, ADCHeader * header);
  void UnpackHeader_GADC(ADCHeader * header);
  void UnpackHeader_IADC(ADCHeader * header);

  ClassDef(FADCRawEvent, 1)
};

inline int FADCRawEvent::GetNCH() const { return fNCH; }

inline int FADCRawEvent::GetNDP() const { return fNDP; }

inline FADCRawChannel * FADCRawEvent::GetChannel(int n) const
{
  return fChannel[n];
}

#endif
