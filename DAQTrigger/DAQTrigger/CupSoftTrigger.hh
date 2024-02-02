#ifndef CupSoftTrigger_hh
#define CupsoftTrigger_hh

#include <iostream>

#include "TObject.h"
#include "TObjArray.h"
#include "DAQTrigger/AbsSoftTrigger.hh"

class Qsum : public TObject {
public:
  int fMID;
  int fRow;
  int fCol;
  int fNCH;
  double * fCut;

  Qsum(int mid, int row, int col) {
    fMID = mid;
    fRow = row;
    fCol = col;
    fNCH = row*col;
    fCut = new double[fNCH];
  }
  virtual ~Qsum() {
    delete[] fCut;
  }
  virtual void Print(Option_t * opt = "") const {
    std::cout << Form("mid %d %d %d", fMID, fRow, fCol) << std::endl;
    for (int i = 0; i < fRow; i++) {
      for (int j = 0; j < fCol; j++) {
        std::cout << fCut[fCol*i+j] << " ";
      }
      std::cout << std::endl;
    }
    std::cout << std::endl;
  }
};

class CupSoftTrigger : public AbsSoftTrigger {
public:
  CupSoftTrigger();
  CupSoftTrigger(AbsConf * config);
  virtual ~CupSoftTrigger();

  virtual void InitTrigger();
  virtual bool DoTrigger(BuiltEvent * bevent);

private:
  bool DoTriggerFADC(BuiltEvent * bevent);
  bool DoTriggerSADC(BuiltEvent * bevent);
  Qsum * FindQsum(int mid) const;

private:
  bool fDoZSUTBit;
  bool fDoZSUQSum;
  bool fDoPrescale;
  int fPrescale;
  TObjArray * fQsumArray;
};

#endif