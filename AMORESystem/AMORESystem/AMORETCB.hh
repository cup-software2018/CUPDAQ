#pragma once

#include <mutex>

#include "TObject.h"

#include "AMORESystem/AMORETCBConf.hh"
#include "AMORESystem/AMOREADCConf.hh"
#include "DAQConfig/AbsConfList.hh"
#include "Notice/NKTCB.hh"

class AMORETCB : public TObject {
public:
  static AMORETCB & Instance();
  ~AMORETCB() override = default;

  int Open();
  void Close();

  bool Config();

  void Reset();
  void ResetTimer();
  void TriggerStart();
  void TriggerStop();

  unsigned long ReadBCount(unsigned long mid);
  int ReadData(unsigned long mid, unsigned long bcount, unsigned char * data);

  void WriteSR(unsigned long mid, unsigned long data);
  unsigned long ReadSR(unsigned long mid);

  void SetConfig(AbsConfList * config);
  int CheckLinkStatus();

private:
  bool ConfigAMORETCB(AMORETCBConf * conf);
  bool ConfigAMOREADC(AMOREADCConf * conf);

private:
  AMORETCB() = default;

  std::mutex fMutex;
  NKTCB fNKTCB;

  AbsConfList * fConfigs = nullptr;

  ClassDef(AMORETCB, 0)
};

inline void AMORETCB::SetConfig(AbsConfList * config) { fConfigs = config; }