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
  fTrgNum = info.GetTriggerNumber();
  fEvtNum = info.GetEventNumber();
  fTrgTime = info.GetTriggerTime();
  fNHit = info.GetNHit();
}

EventInfo::~EventInfo() {}
