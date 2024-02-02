#ifndef CupIADCT_HH
#define CupIADCT_HH

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"

class CupIADCT : public AbsADC {
public:
  CupIADCT();
  CupIADCT(int sid);
  CupIADCT(AbsConf * conf);
  virtual ~CupIADCT();

  virtual int Open();
  virtual void Close();

  virtual bool Configure() { return true; }
  virtual bool Initialize() { return true; }
  virtual void StartTrigger() {}
  virtual void StopTrigger() {}

  virtual int ReadBCount();
  virtual int ReadData(int count, unsigned char * data);
  virtual int ReadData(int count);

private:
  int fMode;

  ClassDef(CupIADCT, 0)
};

#endif
