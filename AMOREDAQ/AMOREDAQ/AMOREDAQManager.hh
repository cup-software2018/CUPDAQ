#pragma once

#include <string>
#include <vector>

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

  virtual void Run();

private:
  void RC_AMORETCB();
  void RC_AMOREDAQ();

  bool ReadConfig();
  bool ParseConfig(std::ifstream & input);

  bool PrepareDAQ() override;

  void TF_ReadData_AMORE();
  void TF_StreamData();

private:
  AMORETCB & fTCB;

  std::vector<std::unique_ptr<AMOREChunkFIFO>> fFIFOs;

  PROCSTATE fStreamStatus;

  ClassDef(AMOREDAQManager, 0)
};
