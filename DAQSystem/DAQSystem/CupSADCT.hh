#ifndef CupSADCT_HH
#define CupSADCT_HH

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"

class CupSADCT : public AbsADC {
public:
  CupSADCT();
  CupSADCT(int sid);
  CupSADCT(AbsConf * conf);
  ~CupSADCT() override = default;

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

  ClassDef(CupSADCT, 0)
};

#endif
