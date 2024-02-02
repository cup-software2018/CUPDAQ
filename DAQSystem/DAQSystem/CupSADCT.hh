#ifndef CupSADCT_HH
#define CupSADCT_HH

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"

class CupSADCT : public AbsADC {
public:
  CupSADCT();
  CupSADCT(int sid);
  CupSADCT(AbsConf * conf);
  virtual ~CupSADCT();

  virtual int Open();
  virtual void Close();

  virtual bool Configure() { return true; }
  virtual bool Initialize() { return true; }
  virtual void StartTrigger() {}
  virtual void StopTrigger() {}

  virtual int ReadBCount();
  virtual int ReadData(int count, unsigned char * data);
  virtual int ReadData(int count);

  ClassDef(CupSADCT, 0)
};

#endif
