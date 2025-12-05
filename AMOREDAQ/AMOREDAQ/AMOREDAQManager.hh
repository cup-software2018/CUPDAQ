#pragma once

#include <string>

#include "TObjArray.h"

#include "DAQConfig/AbsConf.hh"
#include "DAQConfig/AbsConfList.hh"
#include "OnlConsts/onlconsts.hh"

class AMOREDAQManager : public TObjArray {
public:
  AMOREDAQManager();
  ~AMOREDAQManager();

  void SetRunNumber(int run);
  void SetDAQID(int id);
  void SetDAQType(DAQ::TYPE type);
  void SetConfigFile(const char * fname);

  void Run();
  void RC_TCB();
  void RC_DAQ();

private:
  bool AddADC(AbsConf * conf);
  bool AddADC(AbsConfList * conflist);
  bool OpenDAQ();
  void CloseDAQ();

private:
  int fRunNumber{0};
  int fDAQID{0};
  DAQ::TYPE fDAQType{DAQ::NULLDAQ};

  std::string fConfigFilename{};
  AbsConfList * fConfigList;

  ClassDef(AMOREDAQManager, 0)
};

inline void AMOREDAQManager::SetConfigFile(const char * fname) { fConfigFilename = fname; }

inline void AMOREDAQManager::SetRunNumber(int run) { fRunNumber = run; }

inline void AMOREDAQManager::SetDAQID(int id) { fDAQID = id; }

inline void AMOREDAQManager::SetDAQType(DAQ::TYPE type) { fDAQType = type; }