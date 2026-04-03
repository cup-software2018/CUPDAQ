#pragma once
#include "TObjArray.h"

#include "OnlObjs/AbsADCRaw.hh"

class BuiltEvent : public TObjArray {
private:
  int fDAQID;
  unsigned int fEventNumber;

public:
  BuiltEvent();
  BuiltEvent(const BuiltEvent & event);
  ~BuiltEvent() override;

  void SetDAQID(int id);
  void SetEventNumber(unsigned int n);
  void AddADCEvent(AbsADCRaw * event);

  int GetDAQID() const;
  unsigned int GetEventNumber() const;
  unsigned int GetTriggerNumber() const;
  unsigned int GetTriggerType() const;
  unsigned long GetTriggerTime() const;

  bool IsSortable() const override;
  int Compare(const TObject * object) const override;

  ClassDef(BuiltEvent, 1)
};

inline void BuiltEvent::SetDAQID(int id) { fDAQID = id; }

inline void BuiltEvent::SetEventNumber(unsigned int n) { fEventNumber = n; }

inline void BuiltEvent::AddADCEvent(AbsADCRaw * event) { Add(event); }

inline int BuiltEvent::GetDAQID() const { return fDAQID; }

inline unsigned int BuiltEvent::GetEventNumber() const { return fEventNumber; }

inline bool BuiltEvent::IsSortable() const { return true; }
