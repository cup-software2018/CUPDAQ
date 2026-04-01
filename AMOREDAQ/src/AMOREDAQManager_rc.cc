#include <filesystem>

#include "AMOREDAQ/AMOREDAQManager.hh"

void AMOREDAQManager::Run()
{
  if (std::filesystem::exists(kFORCEDENDRUNFILE)) { std::filesystem::remove(kFORCEDENDRUNFILE); }

  if (!ReadConfig()) return;

  RC_AMOREDAQ();
}

void AMOREDAQManager::RC_AMORETCB()
{
  int state = 0;
  INFO("AMORETCB controller now starting [run=%d]", fRunNumber);
}

void AMOREDAQManager::RC_AMOREDAQ()
{
  INFO("amoredaq now starting [run=%d]", fRunNumber);

  std::thread th1;
  std::thread th2;
  std::thread th3;
  std::thread th_swt[8];

  fTCB.SetConfig(fConfigList);

  if (fTCB.Open() != 0) return;
  if (!fTCB.Config()) return;
  if (!AddADC(fConfigList)) return;
  if (!PrepareDAQ()) return;
  if (!OpenDAQ()) return;

  th1 = std::thread(&AMOREDAQManager::TF_ReadData_AMORE, this);
  th2 = std::thread(&AMOREDAQManager::TF_StreamData, this);
  th3 = std::thread(&AMOREDAQManager::TF_WriteEvent_AMORE, this);

  int nadc = GetEntries();
  INFO("RC_AMOREDAQ: nadc=%d", nadc);
  for (int i = 0; i < nadc; ++i) {
    th_swt[i] = std::thread(&AMOREDAQManager::TF_SWTrigger, this, i);
  }

  // sleep for 1 secs
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  fTCB.TriggerStart();
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);

  // sleep for set DAQ time
  std::this_thread::sleep_for(std::chrono::milliseconds(fSetDAQTime*1000));
  fTCB.TriggerStop();
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);

  th1.join();
  th2.join();
  for (int i = 0; i < nadc; ++i) {
    th_swt[i].join();
  }
  th3.join();

  CloseDAQ();
  fTCB.Close();

  INFO("amoredaq ended");
}
