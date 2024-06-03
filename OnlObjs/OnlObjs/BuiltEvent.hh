#ifndef BuiltEvent_hh
#define BuiltEvent_hh

#include "TObjArray.h"
#include "OnlObjs/AbsADCRaw.hh"

class BuiltEvent : public TObjArray {
private:
  int fDAQID;
  unsigned int fEventNumber;

public:
  BuiltEvent();
  BuiltEvent(const BuiltEvent & event);
  virtual ~BuiltEvent();

  void SetDAQID(int id);
  void SetEventNumber(unsigned int n);
  void AddADCEvent(AbsADCRaw * event);

  int GetDAQID() const;
  unsigned int GetEventNumber() const;
  unsigned int GetTriggerNumber() const;
  unsigned int GetTriggerType() const;  
  unsigned long GetTriggerTime() const;

  virtual bool IsSortable() const { return true; }
  virtual int Compare(const TObject * object) const;

  ClassDef(BuiltEvent, 1)
};

inline void BuiltEvent::SetDAQID(int id) { fDAQID = id; }

inline void BuiltEvent::SetEventNumber(unsigned int n) { fEventNumber = n; }

inline void BuiltEvent::AddADCEvent(AbsADCRaw * event)
{
  Add(event);
}

inline int BuiltEvent::GetDAQID() const { return fDAQID; }

inline unsigned int BuiltEvent::GetEventNumber() const { return fEventNumber; }

#endif