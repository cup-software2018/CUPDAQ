#ifndef AmoreADC_hh
#define AmoreADC_hh

#include "TObject.h"

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"
#include "DAQSystem/CupTCB.hh"

class AmoreADC : public AbsADC {
public:
  AmoreADC();
  AmoreADC(int sid);
  AmoreADC(AbsConf * config);
  virtual ~AmoreADC();

  virtual int Open() { return 0; }
  virtual void Close() {}

  virtual bool Configure() { return true; }
  virtual bool Initialize() { return true; }
  virtual void StartTrigger() {}
  virtual void StopTrigger() {}

  virtual int ReadBCount();
  virtual int ReadData(int bcount, unsigned char * data);
  virtual int ReadData(int bcount);

private:
  int fMID;
  CupTCB * fTCB;

  ClassDef(AmoreADC, 0) // AmoreADC wrapper class for root
};

#endif
