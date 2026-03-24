#ifndef FADCRawChannel_hh
#define FADCRawChannel_hh

#include <cstring>

#include "TObject.h"

class FADCRawChannel : public TObject {
private:
  int fNDP;
  unsigned short * fADC;

public:
  FADCRawChannel();
  explicit FADCRawChannel(int ndp);
  FADCRawChannel(const FADCRawChannel & ch);
  ~FADCRawChannel() override;

  void SetADC(int i, unsigned short adc);

  int GetNDP() const;
  unsigned short * GetADC() const;

  void Print(Option_t * opt = "") const override;

  ClassDef(FADCRawChannel, 1)
};

inline void FADCRawChannel::SetADC(int i, unsigned short adc) { fADC[i] = adc; }

inline int FADCRawChannel::GetNDP() const { return fNDP; }

inline unsigned short * FADCRawChannel::GetADC() const { return fADC; }

#endif
