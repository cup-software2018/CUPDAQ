#pragma once

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"
#include "Notice/NKFADC500.hh"

class CupFADCT : public AbsADC {
public:
  CupFADCT() = default;
  CupFADCT(int sid);
  CupFADCT(AbsConf * conf);
  ~CupFADCT() override = default;

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

  NKFADC500 fFADC{};

  ClassDef(CupFADCT, 0)
};
