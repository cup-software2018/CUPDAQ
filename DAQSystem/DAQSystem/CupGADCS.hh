#ifndef CupGADCS_HH
#define CupGADCS_HH

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"
#include "Notice/NKFADC125S.hh"

class CupGADCS : public AbsADC {
public:
  CupGADCS() = default;
  CupGADCS(int sid);
  CupGADCS(AbsConf * config);
  ~CupGADCS() override = default;

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

  ClassDef(CupGADCS, 0)
};

#endif
