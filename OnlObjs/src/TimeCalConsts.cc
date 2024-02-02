#include "OnlObjs/TimeCalConsts.hh"
#include "TString.h"
#include <iostream>

using namespace std;

ClassImp(TimeCalConsts)

TimeCalConsts::TimeCalConsts()
    : TObject()
{
}

TimeCalConsts::~TimeCalConsts() {}

void TimeCalConsts::Add(int mid, int value)
{
  calconsts.insert(pair<int, int>(mid, value));
}

int TimeCalConsts::GetConst(int mid) const
{
  auto item = calconsts.find(mid);
  if (item != calconsts.end()) { return item->second; }
  return -1;
}

void TimeCalConsts::Print(Option_t * opt) const
{
  for (const auto & [key, value] : calconsts) {
    cout << Form("mid=%d, dtime=%d", key, value) << endl;
  }
}