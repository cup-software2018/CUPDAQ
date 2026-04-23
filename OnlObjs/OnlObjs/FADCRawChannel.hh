#pragma once

#include <cstring>

#include "TObject.h"

class FADCRawChannel : public TObject {
private:
  int fNDP;
  unsigned short * fADC; // [fNDP]

public:
  FADCRawChannel();
  explicit FADCRawChannel(int ndp);
  FADCRawChannel(const FADCRawChannel & ch);
  ~FADCRawChannel() override;

  void SetADC(int i, unsigned short adc);

  int GetNDP() const;
  unsigned short * GetADC() const;
  int GetSize() const;

  void Print(Option_t * opt = "") const override;

  ClassDef(FADCRawChannel, 1)
};

inline void FADCRawChannel::SetADC(int i, unsigned short adc) { fADC[i] = adc; }

inline int FADCRawChannel::GetNDP() const { return fNDP; }

inline unsigned short * FADCRawChannel::GetADC() const { return fADC; }

inline int FADCRawChannel::GetSize() const
{
  // ROOT framing: version(2) + bytecount(4) = 6 bytes.
  // Serialized fields: fNDP(4) + fADC[fNDP](2*fNDP).
  return 6 + 4 + fNDP * 2;
}
