#ifndef CupGADCT_HH
#define CupGADCT_HH

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"

class CupGADCT : public AbsADC {
public:
  CupGADCT();
  CupGADCT(int sid);
  CupGADCT(AbsConf * conf);
  virtual ~CupGADCT();

  virtual int Open();
  virtual void Close();

  virtual bool Configure() { return true; }
  virtual bool Initialize() { return true; }
  virtual void StartTrigger() {}
  virtual void StopTrigger() {}

  virtual int ReadBCount();
  virtual int ReadData(int count, unsigned char * data);
  virtual int ReadData(int count);

  ClassDef(CupGADCT, 0)
};

#endif
