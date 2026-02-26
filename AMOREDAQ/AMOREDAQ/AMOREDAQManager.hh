#pragma once

#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "AMOREAlgs/AMOREChunkFIFO.hh"
#include "AMORESystem/AMORETCB.hh"
#include "DAQ/CupDAQManager.hh"
#include "DAQConfig/AbsConf.hh"
#include "DAQConfig/AbsConfList.hh"
#include "OnlConsts/onlconsts.hh"

class AMOREDAQManager : public CupDAQManager {
public:
  AMOREDAQManager();
  ~AMOREDAQManager() override;

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

  void TF_ReadData_AMORE();
  void TF_StreamData();
  void TF_SWTrigger(int n);

private:
  AMORETCB & fTCB;

  std::vector<std::unique_ptr<AMOREChunkFIFO>> fFIFOs;

  PROCSTATE fStreamStatus;

  unsigned long fTimeDelta;

  ClassDef(AMOREDAQManager, 0)
};
