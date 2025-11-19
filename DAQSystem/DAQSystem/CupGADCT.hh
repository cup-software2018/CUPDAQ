#pragma once

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"

class CupGADCT : public AbsADC {
public:
  CupGADCT();
  CupGADCT(int sid);
  CupGADCT(AbsConf * conf);
  ~CupGADCT() override = default;

  int Open() override;
  void Close() override;

  bool Configure() override { return true; }
  bool Initialize() override { return true; }
  void StartTrigger() override {}
  void StopTrigger() override {}

  int ReadBCount() override;
  int ReadData(int count, unsigned char * data) override;
  int ReadData(int count) override;

private:
  void UpdateTriggerAndTime(const unsigned char * tempdata);

  ClassDef(CupGADCT, 0)
};
