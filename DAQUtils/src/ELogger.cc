#include <cstdarg>
#include <iostream>

#include "TMessage.h"

#include "DAQUtils/ELog.hh"
#include "DAQUtils/ELogger.hh"
#include "OnlConsts/onlconsts.hh"

using namespace std;

ClassImp(ELogger)

ELogger * ELogger::sfLog = nullptr;

ELogger::ELogger(bool send)
{
  fEnded = false;
  fSend = send;
  fSocket = nullptr;
  fConnected = false;

  if (fSend) {
    fSocket = new TSocket(kELOGIPADDR, kELOGPORT);
    if (!fSocket->IsValid()) { fConnected = false; }
    else {
      fConnected = true;
    }
    fThread = new std::thread(&ELogger::ConnectionMonitor, this);
  }

  fTag = 0;
  fRunnum = 0;
}

ELogger::~ELogger()
{
  fEnded = true;

  if (fSend) {
    fThread->join();

    if (fSocket) {
      fSocket->Close();
      delete fSocket;
    }

    if (fLogStream.is_open()) { fLogStream.close(); }
  }
}

ELogger * ELogger::Instance(bool send)
{
  if (!sfLog) { sfLog = new ELogger(send); }
  return sfLog;
}

void ELogger::DeleteInstance()
{
  if (sfLog) { delete sfLog; }

  sfLog = nullptr;
}

void ELogger::Stats(const char * log, ...)
{
  va_list argList;
  char buffer[1024];

  va_start(argList, log);
  vsnprintf(buffer, 1024, log, argList);
  va_end(argList);

  //TString mess = Form("%s: %s", where, buffer);
  ELog * elog = new ELog(ELog::STATS, buffer, fTag, fRunnum);

  std::lock_guard<std::mutex> lock(fMutex);
  if (fLogStream.is_open()) { fLogStream << elog->GetELog() << endl; }
  elog->Print();

  SendLog(elog);
  delete elog;
}

void ELogger::Info(const char * where, const char * log, ...)
{
  va_list argList;
  char buffer[1024];

  va_start(argList, log);
  vsnprintf(buffer, 1024, log, argList);
  va_end(argList);

  TString mess = Form("%s: %s", where, buffer);
  ELog * elog = new ELog(ELog::INFO, mess.Data(), fTag, fRunnum);

  std::lock_guard<std::mutex> lock(fMutex);
  if (fLogStream.is_open()) { fLogStream << elog->GetELog() << endl; }
  elog->Print();

  SendLog(elog);
  delete elog;
}

void ELogger::Debug(const char * where, const char * log, ...)
{
  va_list argList;
  char buffer[1024];

  va_start(argList, log);
  vsnprintf(buffer, 1024, log, argList);
  va_end(argList);

  TString mess = Form("%s: %s", where, buffer);
  ELog * elog = new ELog(ELog::DEBUG, mess.Data(), fTag, fRunnum);

  std::lock_guard<std::mutex> lock(fMutex);
  if (fLogStream.is_open()) { fLogStream << elog->GetELog() << endl; }
  elog->Print();

  SendLog(elog);
  delete elog;
}

void ELogger::Warning(const char * where, const char * log, ...)
{
  va_list argList;
  char buffer[1024];

  va_start(argList, log);
  vsnprintf(buffer, 1024, log, argList);
  va_end(argList);

  TString mess = Form("%s: %s", where, buffer);
  ELog * elog = new ELog(ELog::WARNING, mess.Data(), fTag, fRunnum);

  std::lock_guard<std::mutex> lock(fMutex);
  if (fLogStream.is_open()) { fLogStream << elog->GetELog() << endl; }
  elog->Print();

  SendLog(elog);
  delete elog;
}

void ELogger::Error(const char * where, const char * log, ...)
{
  va_list argList;
  char buffer[1024];

  va_start(argList, log);
  vsnprintf(buffer, 1024, log, argList);
  va_end(argList);

  TString mess = Form("%s: %s", where, buffer);
  ELog * elog = new ELog(ELog::ERROR, mess.Data(), fTag, fRunnum);

  std::lock_guard<std::mutex> lock(fMutex);
  if (fLogStream.is_open()) { fLogStream << elog->GetELog() << endl; }
  elog->Print();

  SendLog(elog);
  delete elog;
}

void ELogger::PrintLog(const char * log, ...)
{
  va_list argList;
  char buffer[1024];

  va_start(argList, log);
  vsnprintf(buffer, 1024, log, argList);
  va_end(argList);

  TString printout(buffer);

  std::lock_guard<std::mutex> lock(fMutex);
  if (fLogStream.is_open()) { fLogStream << printout << endl; }

  cout << printout << endl;
}

void ELogger::Log(const char * where, const char * log, ...)
{
  va_list argList;
  char buffer[1024];

  va_start(argList, log);
  vsnprintf(buffer, 1024, log, argList);
  va_end(argList);

  cout << Form("[ELogger:%s] %s", where, buffer) << endl;
}

void ELogger::SendLog(ELog * elog)
{
  if (!fSend) return;
  if (fConnected) {
    TMessage mess(kMESS_OBJECT);
    mess.WriteObject(elog);
    fSocket->Send(mess);
    char str[8];
    if (fSocket->Recv(str, 8) <= 0) {
      fConnected = false;
      Log("SendLog", "server connection down");
    }
  }
}

void ELogger::ConnectionMonitor()
{
  while (true) {
    if (fEnded) break;
    if (!fConnected) {
      fSocket->Close();
      delete fSocket;

      std::unique_lock<std::mutex> lock{fMutex};
      fSocket = new TSocket(kELOGIPADDR, kELOGPORT);
      lock.unlock();

      if (!fSocket->IsValid()) { fConnected = false; }
      else {
        fConnected = true;
        Log("ConnectionMonitor", "server connection succeeded");
      }
    }
    gSystem->Sleep(1);
  }
}