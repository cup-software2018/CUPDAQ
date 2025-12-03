#ifndef CupIADCT_HH
#define CupIADCT_HH

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"
#include "NewNotice/NKIADC64.hh"

class CupIADCT : public AbsADC {
public:
  CupIADCT() = default;
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
  void UpdateTriggerAndTime(const unsigned char * tempdata);

  int fMode{0};
  NKIADC64 fFADC{};

  ClassDef(CupIADCT, 0)
};

#endif
