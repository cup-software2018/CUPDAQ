#ifndef ELog_hh
#define ELog_hh

#include <time.h>

#include "TObject.h"
#include "TString.h"

class ELog : public TObject {
public:
  enum SEVERITY { INFO, WARNING, ERROR, DEBUG };

  ELog();
  ELog(SEVERITY level, const char * log, unsigned short tag = 0,
       unsigned int run = 0);
  ELog(const ELog & elog);
  virtual ~ELog();

  SEVERITY GetSeverity() const;
  const char * GetLog() const;
  time_t GetTime() const;
  unsigned short GetTag() const;
  unsigned int GetRunNumber() const;

  TString GetELog() const;
  virtual void Print(Option_t * option = "") const;
  virtual void PrintWithTag(Option_t * option = "") const;

private:
  SEVERITY fLevel;
  TString fLog;
  time_t fTime;
  unsigned short fTag;
  unsigned int fRunnum;

  ClassDef(ELog, 1)
};

inline ELog::SEVERITY ELog::GetSeverity() const { return fLevel; }

inline const char * ELog::GetLog() const { return fLog; }

inline time_t ELog::GetTime() const { return fTime; }

inline unsigned short ELog::GetTag() const { return fTag; }

inline unsigned int ELog::GetRunNumber() const { return fRunnum; }

#endif