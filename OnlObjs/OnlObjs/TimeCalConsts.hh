#ifndef TimeCalConsts_hh
#define TimeCalConsts_hh

#include <map>

#include "TObject.h"

class TimeCalConsts : public TObject {
private:
  std::map<int, int> calconsts;

public:
  TimeCalConsts();
  ~TimeCalConsts() override;

  void Add(int mid, int value);
  int GetConst(int mid) const;

  void Print(Option_t * opt = "") const override;

  ClassDef(TimeCalConsts, 1)
};

inline void TimeCalConsts::Add(int mid, int value) { calconsts.insert({mid, value}); }

inline int TimeCalConsts::GetConst(int mid) const
{
  auto item = calconsts.find(mid);
  return (item != calconsts.end()) ? item->second : -1;
}

#endif
