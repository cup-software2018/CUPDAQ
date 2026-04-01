#pragma once

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "AMOREAlgs/AMOREChunkFIFO.hh"
#include "AMORESystem/AMORETCB.hh"
#include "DAQ/CupDAQManager.hh"
#include "DAQConfig/AbsConf.hh"
#include "DAQConfig/AbsConfList.hh"
#include "HDF5Utils/EDM.hh"
#include "OnlConsts/onlconsts.hh"
#include "DAQUtils/ConcurrentDeque.hh"

class AMOREDAQManager : public CupDAQManager {
public:
  AMOREDAQManager();
  ~AMOREDAQManager() override;

  void SetRunNumber(int run);
  void SetDAQID(int id);
  void SetConfigFilename(const char * name);
  void SetOutputFilename(const char * fname);

  void SetDAQTime(int t);
  void SetNEvent(int n);
  void SetCompressionLevel(int level);
  void SetOutputSplitTime(int time);

  void SetVerboseLevel(int level);
  void EnableHistograming();

  bool AddADC(AbsConfList * conflist) override;
  bool PrepareDAQ() override;

  virtual void Run();

private:
  void RC_AMORETCB();
  void RC_AMOREDAQ();

  bool ReadConfig();

  template <typename T>
  void FillConfigArray(YAML::Node node, int nch, std::function<void(int, T)> setter,
                       bool inc = false);

  void ReadConfigTCB(YAML::Node ymlnode);
  void ReadConfigADC(YAML::Node ymlnode);

  bool HasRunningTrigger() const;
  void TF_ReadData_AMORE();
  void TF_StreamData();
  void TF_SWTrigger(int n);
  void TF_WriteEvent_AMORE();


private:
  AMORETCB & fTCB;

  std::vector<std::unique_ptr<AMOREChunkFIFO>> fFIFOs;
  ConcurrentDeque<Crystal_t> fTriggeredCrystals;

  PROCSTATE fStreamStatus;
  PROCSTATE fTrigStatus[kNADCAMOREADC];

  unsigned long fTimeDelta;

  ClassDef(AMOREDAQManager, 0)
};

inline void AMOREDAQManager::SetRunNumber(int run) { fRunNumber = run; }

inline void AMOREDAQManager::SetDAQID(int id) { fDAQID = id; }

inline void AMOREDAQManager::SetConfigFilename(const char * name) { fConfigFilename = name; }

inline void AMOREDAQManager::SetOutputFilename(const char * fname) { fOutputFilename = fname; }

inline void AMOREDAQManager::SetDAQTime(int t) { fSetDAQTime = t; }

inline void AMOREDAQManager::SetNEvent(int n) { fSetNEvent = n; }

inline void AMOREDAQManager::SetCompressionLevel(int level) { fCompressionLevel = level; }

inline void AMOREDAQManager::SetOutputSplitTime(int time) { fOutputSplitTime = time; }

inline void AMOREDAQManager::SetVerboseLevel(int level) { fVerboseLevel = level; }

inline void AMOREDAQManager::EnableHistograming() { fDoHistograming = true; }
