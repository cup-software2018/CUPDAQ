#pragma once

#include "AMOREDAQ/AMORETCB.hh"
#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"

class AMOREADC : public AbsADC {
public:
  AMOREADC();
  AMOREADC(int sid);
  AMOREADC(AbsConf * config);
  ~AMOREADC() override = default;

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
  AMORETCB & fTCB;

  ClassDef(AMOREADC, 0)
};
