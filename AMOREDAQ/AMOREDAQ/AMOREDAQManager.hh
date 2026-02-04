#pragma once

#include <string>

#include "AMOREDAQ/AMORETCB.hh"
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

private:
  AMORETCB & fTCB;

  ClassDef(AMOREDAQManager, 0)
};
