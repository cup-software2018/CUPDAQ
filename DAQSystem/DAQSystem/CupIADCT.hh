#ifndef CupIADCT_HH
#define CupIADCT_HH

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"

class CupIADCT : public AbsADC {
public:
  CupIADCT();
  CupIADCT(int sid);
  CupIADCT(AbsConf * conf);
  ~CupIADCT() override = default;

  int Open() override;
  void Close() override;

  bool Configure() override { return true; }
  bool Initialize() override { return true; }
  void StartTrigger() override {}
  void StopTrigger() override {}

  int ReadBCount() override;
  int ReadData(int bcount, unsigned char * data) override;
  int ReadData(int bcount) override;

private:
  int fMode{0};

  void UpdateTriggerAndTime(const unsigned char * tempdata);

  ClassDef(CupIADCT, 0)
};

#endif
