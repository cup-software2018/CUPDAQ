#ifndef EventInfo_hh
#define EventInfo_hh

#include "TObject.h"

class EventInfo : public TObject {
public:
  EventInfo();
  EventInfo(const EventInfo & info);
  virtual ~EventInfo();

  void SetTriggerType(unsigned short type);
  void SetNHit(unsigned short n);
  void SetTriggerNumber(unsigned int n);
  void SetEventNumber(unsigned int n);
  void SetTriggerTime(unsigned long t);

  unsigned short GetTriggerType() const;
  unsigned short GetNHit() const;
  unsigned int GetTriggerNumber() const;
  unsigned int GetEventNumber() const;
  unsigned long GetTriggerTime() const;

private:
  unsigned short fTrgType;
  unsigned short fNHit;
  unsigned int fTrgNum;
  unsigned int fEvtNum;
  unsigned long fTrgTime;

  ClassDef(EventInfo, 1)
};

inline void EventInfo::SetTriggerType(unsigned short type) { fTrgType = type; }
inline void EventInfo::SetNHit(unsigned short n) { fNHit = n; }
inline void EventInfo::SetTriggerNumber(unsigned int n) { fTrgNum = n; }
inline void EventInfo::SetEventNumber(unsigned int n) { fEvtNum = n; }
inline void EventInfo::SetTriggerTime(unsigned long t) { fTrgTime = t; }
inline unsigned short EventInfo::GetTriggerType() const { return fTrgType; }
inline unsigned short EventInfo::GetNHit() const { return fNHit; }
inline unsigned int EventInfo::GetTriggerNumber() const { return fTrgNum; }
inline unsigned int EventInfo::GetEventNumber() const { return fEvtNum; }
inline unsigned long EventInfo::GetTriggerTime() const { return fTrgTime; }

#endif
