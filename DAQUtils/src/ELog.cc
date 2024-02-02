#include <iostream>

#include "TDatime.h"

#include "DAQUtils/ELog.hh"

ClassImp(ELog)

ELog::ELog()
    : TObject()
{
  fLevel = ELog::INFO;
  fLog = "null log";
  fTag = 0;
  fRunnum = 0;
  time(&fTime);
}

ELog::ELog(SEVERITY level, const char * log, unsigned short tag,
           unsigned int runnum)
    : TObject()
{
  fLevel = level;
  fLog = log;
  fTag = tag;
  fRunnum = runnum;
  time(&fTime);
}

ELog::ELog(const ELog & log)
    : TObject()
{
  fLevel = log.GetSeverity();
  fLog = log.GetLog();
  fTag = log.GetTag();
  fRunnum = log.GetRunNumber();
  fTime = log.GetTime();
}

ELog::~ELog() {}

TString ELog::GetELog() const
{
  TString timestr = TDatime(fTime).AsSQLString();
  TString runstr = fRunnum > 0 ? Form(":%06d", fRunnum) : "";
  TString severitystr;
  switch (fLevel) {
    case ELog::INFO: severitystr = " [INFO] "; break;
    case ELog::WARNING: severitystr = " [WARNING] "; break;
    case ELog::ERROR: severitystr = " [ERROR] "; break;
    case ELog::DEBUG: severitystr = " [DEBUG] "; break;
    default: break;
  }

  TString elog = timestr + runstr + severitystr + fLog;
  return elog;
}

void ELog::Print(Option_t * option) const
{
  std::cout << GetELog() << std::endl;
}

void ELog::PrintWithTag(Option_t * option) const
{
  std::cout << Form("->%03d ", fTag) << GetELog() << std::endl;
}