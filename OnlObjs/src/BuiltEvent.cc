#include <limits>

#include "OnlObjs/BuiltEvent.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"

ClassImp(BuiltEvent)

BuiltEvent::BuiltEvent()
  : TObjArray(),
    fDAQID(0),
    fEventNumber(0)
{
  SetOwner(kTRUE);
}

BuiltEvent::BuiltEvent(const BuiltEvent & builtevent)
  : TObjArray(),
    fDAQID(builtevent.GetDAQID()),
    fEventNumber(builtevent.GetEventNumber())
{
  SetOwner(kTRUE);
  const int nent = builtevent.GetLast() + 1;
  for (int i = 0; i < nent; ++i) {
    auto * event = static_cast<AbsADCRaw *>(builtevent.At(i));
    if (!event) continue;

    AbsADCRaw * newadc = nullptr;
    switch (event->GetADCMode()) {
      case ADC::FMODE: {
        newadc = new FADCRawEvent(*static_cast<FADCRawEvent *>(event));
        break;
      }
      case ADC::SMODE: {
        newadc = new SADCRawEvent(*static_cast<SADCRawEvent *>(event));
        break;
      }
      default: break;
    }

    if (newadc) {
      AddAt(newadc, i);
    }
  }
}

BuiltEvent::~BuiltEvent() {}

unsigned int BuiltEvent::GetTriggerType() const
{
  if (GetEntries() == 0) { return 0U; }

  auto * adcevent = static_cast<AbsADCRaw *>(At(0));
  return adcevent ? adcevent->GetTriggerType() : 0U;
}

unsigned int BuiltEvent::GetTriggerNumber() const
{
  if (GetEntries() == 0) { return 0U; }

  auto * adcevent = static_cast<AbsADCRaw *>(At(0));
  return adcevent ? adcevent->GetTriggerNumber() : 0U;
}

unsigned long BuiltEvent::GetTriggerTime() const
{
  const int nevt = GetEntries();
  if (nevt <= 0) { return 0UL; }

  unsigned long fastttime = std::numeric_limits<unsigned long>::max();

  for (int i = 0; i < nevt; ++i) {
    auto * adcevent = static_cast<AbsADCRaw *>(At(i));
    if (!adcevent) continue;

    const unsigned long ttime = adcevent->GetTriggerTime();
    if (ttime < fastttime) { fastttime = ttime; }
  }

  return fastttime;
}

int BuiltEvent::Compare(const TObject * object) const
{
  auto * comp = static_cast<const BuiltEvent *>(object);
  if (!comp) { return 0; }

  const unsigned long t1 = GetTriggerTime();
  const unsigned long t2 = comp->GetTriggerTime();

  if (t1 > t2) { return 1; }
  if (t1 < t2) { return -1; }
  return 0;
}
