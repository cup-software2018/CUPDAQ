#include "OnlObjs/BuiltEvent.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"

ClassImp(BuiltEvent)

BuiltEvent::BuiltEvent()
    : TObjArray()
{
  fDAQID = 0;
  fEventNumber = 0;
}

BuiltEvent::BuiltEvent(const BuiltEvent & builtevent)
    : TObjArray(builtevent)
{
  fDAQID = builtevent.GetDAQID();
  fEventNumber = builtevent.GetEventNumber();

  for (int i = 0; i < builtevent.GetEntries(); i++) {
    auto * event = (AbsADCRaw *)builtevent.At(i);
    switch (event->GetADCMode()) {
      case ADC::FMODE: {
        auto * adc = (FADCRawEvent *)event;
        auto * newadc = new FADCRawEvent(*adc);
        Add(newadc);
        break;
      }
      case ADC::SMODE: {
        auto * adc = (SADCRawEvent *)event;
        auto * newadc = new SADCRawEvent(*adc);
        Add(newadc);
        break;
      }
      default: break;
    }
  }
}

BuiltEvent::~BuiltEvent() 
{
  Delete();
}

unsigned int BuiltEvent::GetTriggerType() const
{
  auto * adcevent = (AbsADCRaw *)At(0);
  return adcevent->GetTriggerType();
}


unsigned int BuiltEvent::GetTriggerNumber() const
{
  auto * adcevent = (AbsADCRaw *)At(0);
  return adcevent->GetTriggerNumber();
}


unsigned long BuiltEvent::GetTriggerTime() const
{
  unsigned long fastttime = UINT64_MAX;
  int nevt = GetEntries();
  for (int i = 0; i < nevt; i++) {
    auto * adcevent = (AbsADCRaw *)At(i);
    unsigned long ttime = adcevent->GetTriggerTime();
    if (ttime < fastttime) { fastttime = ttime; }
  }
  return fastttime;
}

int BuiltEvent::Compare(const TObject * object) const
{
  auto * comp = (BuiltEvent *)object;
  if (this->GetTriggerTime() > comp->GetTriggerTime()) { return 1; }
  else if (this->GetTriggerTime() < comp->GetTriggerTime()) {
    return -1;
  }
  return 0;
}