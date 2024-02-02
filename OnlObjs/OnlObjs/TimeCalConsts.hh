#ifndef TimeCalConsts_hh
#define TimeCalConsts_hh

#include <map>

#include "TObject.h"

class TimeCalConsts : public TObject {
private:
  std::map<int, int> calconsts;

public:
  TimeCalConsts();
  virtual ~TimeCalConsts();

  void Add(int mid, int value);
  int GetConst(int mid) const;

  virtual void Print(Option_t * opt = "") const;

  ClassDef(TimeCalConsts, 1)
};

#endif