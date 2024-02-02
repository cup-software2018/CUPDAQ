#ifndef CupFADCT_HH
#define CupFADCT_HH

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"

class CupFADCT : public AbsADC {
public:
  CupFADCT();
  CupFADCT(int sid);
  CupFADCT(AbsConf * conf);
  virtual ~CupFADCT();

  virtual int Open();
  virtual void Close();

  virtual bool Configure() { return true; }
  virtual bool Initialize() { return true; }
  virtual void StartTrigger() {}
  virtual void StopTrigger() {}

  virtual int ReadBCount();
  virtual int ReadData(int count, unsigned char * data);
  virtual int ReadData(int count);

  ClassDef(CupFADCT, 0)
};

#endif
