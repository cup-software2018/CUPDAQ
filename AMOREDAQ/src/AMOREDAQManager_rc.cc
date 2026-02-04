#include <filesystem>

#include "AMOREDAQ/AMOREDAQManager.hh"

void AMOREDAQManager::Run()
{
  if (std::filesystem::exists(kFORCEDENDRUNFILE)) { std::filesystem::remove(kFORCEDENDRUNFILE); }

  if (!ReadConfig()) return;

  fTCB.SetConfig(fConfigList);
}

void AMOREDAQManager::RC_AMORETCB()
{
  int state = 0;
  INFO("AMORETCB controller now starting [run=%d]", fRunNumber);
}

void AMOREDAQManager::RC_AMOREDAQ()
{
  INFO("daq now starting [run=%d]", fRunNumber);

  std::thread th1;

  // AMORETCB Open
  if (fTCB.Open() != 0) { goto TERMINATE; }
  if (!fTCB.Config()) { goto TERMINATE; }
  if (!AddADC(fConfigList)) { goto TERMINATE; }
  if (!PrepareDAQ()) { goto TERMINATE; }
  if (!OpenDAQ()) { goto TERMINATE; }

  th1 = std::thread(&AMOREDAQManager::TF_ReadData_AMORE, this);

TERMINATE:

  INFO("daq ended");
  CloseDAQ();
}
