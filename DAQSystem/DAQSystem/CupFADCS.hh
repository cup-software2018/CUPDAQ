#pragma once

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"
#include "Notice/NKFADC500S.hh"

class CupFADCS : public AbsADC {
public:
  CupFADCS() = default;
  CupFADCS(int sid);
  CupFADCS(AbsConf * config);
  ~CupFADCS() override = default;

  int Open() override;
  void Close() override;

  bool Configure() override;
  bool Initialize() override;
  void StartTrigger() override;
  void StopTrigger() override;

  int ReadBCount() override;
  int ReadData(int count, unsigned char * data) override;
  int ReadData(int count) override;

private:
  void UpdateTriggerAndTime(const unsigned char * tempdata);

  NKFADC500S fFADC{};

  ClassDef(CupFADCS, 0)
};
