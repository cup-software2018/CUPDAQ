#include "RawObjs/EventInfo.hh"

ClassImp(EventInfo)

EventInfo::EventInfo()
  : TObject()
{
}

EventInfo::EventInfo(const EventInfo & info)
  : TObject(info)
{
  fTrgType = info.GetTriggerType();
  fNHit = info.GetNHit();
  fTrgNum = info.GetTriggerNumber();
  fEvtNum = info.GetEventNumber();
  fTrgTime = info.GetTriggerTime();
}
