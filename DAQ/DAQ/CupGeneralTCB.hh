#ifndef CupGeneralTCB_hh
#define CupGeneralTCB_hh

#include "TObject.h"

#include "DAQConfig/AbsConfList.hh"
//#include "DAQConfig/AmoreADCConf.hh"
#include "DAQConfig/FADCTConf.hh"
#include "DAQConfig/GADCTConf.hh"
#include "DAQConfig/IADCTConf.hh"
#include "DAQConfig/SADCTConf.hh"
#include "DAQConfig/TCBConf.hh"
#include "DAQSystem/AbsTCB.hh"
#include "DAQSystem/CupMiniTCB.hh"
#include "DAQSystem/CupTCB.hh"
#include "DAQUtils/ELogger.hh"
#include "OnlConsts/onlconsts.hh"

class CupGeneralTCB : public TObject {
public:
  CupGeneralTCB();
  virtual ~CupGeneralTCB();

  int Open();
  void Close();

  bool Config();
  bool StartTrigger();
  void StopTrigger();

  void SetConfig(AbsConfList * configs);
  void SetIPAddress(const char * ipaddr);
  void SetADCType(ADC::TYPE type);

private:
  int CheckLinkStatus();
  bool ConfigTCB(TCBConf * conf);
  bool ConfigFADC(FADCTConf * conf);
  bool ConfigGADC(GADCTConf * conf);
  bool ConfigSADC(SADCTConf * conf);
  bool ConfigIADC(IADCTConf * conf);
  //bool ConfigAmoreADC(AmoreADCConf * conf);

private:
  ELogger * fLog;

  AbsTCB * fTCB;
  TCBConf * fTCBConfig;
  AbsConfList * fConfigs;

  TCB::TYPE fTCBType;
  ADC::TYPE fADCType;
  DAQ::EXPERIMENT fExperiment;

  ClassDef(CupGeneralTCB, 0)
};

inline void CupGeneralTCB::SetConfig(AbsConfList * configs)
{
  fConfigs = configs;
}

inline void CupGeneralTCB::SetIPAddress(const char * ipaddr)
{
  if (fTCBType == TCB::MINI) { ((CupMiniTCB *)fTCB)->SetIPAddress(ipaddr); }
}

inline void CupGeneralTCB::SetADCType(ADC::TYPE type) { fADCType = type; }

#endif
