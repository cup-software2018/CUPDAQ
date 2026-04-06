#pragma once

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"
#include "DAQSystem/CupTCB.hh"

class AmoreADC : public AbsADC {
public:
  AmoreADC();
  AmoreADC(int sid);
  AmoreADC(AbsConf * config);
  ~AmoreADC() override = default;

  int Open() override { return 0; }
  void Close() override {}

  bool Configure() override { return true; }
  bool Initialize() override { return true; }
  void StartTrigger() override {}
  void StopTrigger() override {}

  int ReadBCount() override;
  int ReadData(int bcount, unsigned char * data) override;
  int ReadData(int bcount) override;

private:
  void UpdateCurrentTime(const unsigned char * data, int bcount);

  int fMID;
  CupTCB * fTCB;

  ClassDef(AmoreADC, 0)
};
