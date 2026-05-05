#pragma once

#include <string>

#include "DAQConfig/AbsConfList.hh"
#include "OnlObjs/BuiltEvent.hh"

class AbsSoftTrigger {
public:
  AbsSoftTrigger(const char * name);
  virtual ~AbsSoftTrigger() = default;

  virtual void SetDAQID(int id);
  virtual void SetEnable();
  virtual void SetVerboseLevel(int verbose);

  virtual void DoConfig(AbsConfList * configs) = 0;
  virtual void InitTrigger() = 0;
  virtual bool DoTrigger(BuiltEvent * event) = 0;

  virtual int GetDAQID() const;
  virtual bool IsEnabled() const;
  virtual const char * GetName() const;
  virtual void PrintReport() const;

protected:
  int fDAQID{0};
  std::string fName;

  bool fIsEnabled{false};
  int fVerboseLevel{0};

  int fTotalInputEvent{0};
  int fNTriggeredEvent{0};
};

inline void AbsSoftTrigger::SetDAQID(int id) { fDAQID = id; }

inline void AbsSoftTrigger::SetEnable() { fIsEnabled = true; }

inline void AbsSoftTrigger::SetVerboseLevel(int verbose) { fVerboseLevel = verbose; }

inline int AbsSoftTrigger::GetDAQID() const { return fDAQID; }

inline bool AbsSoftTrigger::IsEnabled() const { return fIsEnabled; }

inline const char * AbsSoftTrigger::GetName() const { return fName.c_str(); }
