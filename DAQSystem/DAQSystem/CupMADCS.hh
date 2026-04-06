#pragma once

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"
#include "Notice/NKFADC125S.hh"

class CupMADCS : public AbsADC {
public:
  CupMADCS() = default;
  CupMADCS(int sid);
  CupMADCS(AbsConf * config);
  ~CupMADCS() override = default;

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

  NKFADC125S fFADC{};

  ClassDef(CupMADCS, 0)
};
