#ifndef ELogger_hh
#define ELogger_hh

#include <fstream>
#include <mutex>
#include <thread>

#include "TSocket.h"
#include "TSystem.h"

#include "DAQUtils/ELog.hh"

class ELogger {
private:
  ELogger(bool send = false);
  virtual ~ELogger();

public:
  static ELogger * Instance(bool send = false);
  static void DeleteInstance();

  void Stats(const char * log, ...);
  void Info(const char * where, const char * log, ...);
  void Debug(const char * where, const char * log, ...);
  void Warning(const char * where, const char * log, ...);
  void Error(const char * where, const char * log, ...);
  void PrintLog(const char * log, ...);

  void OpenLogFile(const char * fname);
  void SetTag(unsigned short tag);
  void SetRunNumber(unsigned int runnum);

private:
  void Log(const char * where, const char * log, ...);
  void SendLog(ELog * elog);
  void ConnectionMonitor();

private:
  static ELogger * sfLog;
  unsigned short fTag;
  unsigned int fRunnum;
  std::ofstream fLogStream;

  std::mutex fMutex;
  std::thread * fThread;

  bool fEnded;
  bool fSend;
  bool fConnected;
  TSocket * fSocket;

  ClassDef(ELogger, 0)
};

inline void ELogger::OpenLogFile(const char * fname)
{
  const char * cmd = Form("rm -f %s", fname);
  gSystem->Exec(cmd);

  fLogStream.open(fname, std::ofstream::app);
  if (!fLogStream.is_open()) {
    Warning("ELogger::OpenLogFile", "error log file %s is not opened", fname);
  }

  Info("ELogger::OpenLogFile", "log file %s is opened", fname);
}

inline void ELogger::SetTag(unsigned short tag) { fTag = tag; }
inline void ELogger::SetRunNumber(unsigned int runnum) { fRunnum = runnum; }

#endif
