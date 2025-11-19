#include <iostream>

#include "TString.h"

#include "OnlObjs/TimeCalConsts.hh"

ClassImp(TimeCalConsts)

TimeCalConsts::TimeCalConsts()
  : TObject()
{
}

TimeCalConsts::~TimeCalConsts() = default;

void TimeCalConsts::Print(Option_t * opt) const
{
  for (const auto & [key, value] : calconsts) {
    std::cout << Form("mid=%d, dtime=%d", key, value) << std::endl;
  }
}
