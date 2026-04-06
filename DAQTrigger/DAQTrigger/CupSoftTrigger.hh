#pragma once

#include "TObjArray.h"
#include "TObject.h"

#include "DAQTrigger/AbsSoftTrigger.hh"

class Qsum : public TObject {
public:
  int fMID;
  int fRow;
  int fCol;
  int fNCH;
  double * fCut;

  Qsum(int mid, int row, int col);
  ~Qsum() override;
  void Print(Option_t * opt = "") const override;
};

class CupSoftTrigger : public AbsSoftTrigger {
public:
  CupSoftTrigger();
  explicit CupSoftTrigger(AbsConf * config);
  ~CupSoftTrigger() override;

  void InitTrigger() override;
  bool DoTrigger(BuiltEvent * bevent) override;

private:
  bool DoTriggerFADC(BuiltEvent * bevent);
  bool DoTriggerSADC(BuiltEvent * bevent);
  Qsum * FindQsum(int mid) const;

private:
  bool fDoZSUTBit;
  bool fDoZSUQSum;
  bool fDoZSUPHeight;
  bool fDoPrescale;
  int fPHeight;
  int fPrescale;
  TObjArray * fQsumArray;
};
