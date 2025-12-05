#pragma once

#include <mutex>

#include "TObject.h"

#include "Notice/NKTCB.hh"

class AMORETCB : public TObject {
public:
  static AMORETCB & Instance();

  ~AMORETCB() override = default;

  int Open();
  void Close();

  void Reset();
  void ResetTimer();
  void TriggerStart();
  void TriggerStop();

  unsigned long ReadBCount(unsigned long mid);
  int ReadData(unsigned long mid, unsigned long bcount, unsigned char * data);

  void WriteSR(unsigned long mid, unsigned long data);
  unsigned long ReadSR(unsigned long mid);

private:
  AMORETCB() = default;

  std::mutex fMutex;
  NKTCB fNKTCB;

  ClassDef(AMORETCB, 0)
};
