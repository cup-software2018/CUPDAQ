#include "RawObjs/EventInfo.hh"

ClassImp(EventInfo)

    EventInfo::EventInfo()
    : TObject()
{
  fTrgType = 0;
  fTrgNum = 0;
  fEvtNum = 0;
  fTrgTime = 0;
  fNHit = 0;
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
