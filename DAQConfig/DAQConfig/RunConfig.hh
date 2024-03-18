/*
 *
 *  Module:  RunConfig
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: Class for configuring DAQ fronend electronics
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/07/18 12:25:11 $
 *  CVS/RCS Revision: $Revision: 1.28 $
 *  Status:           $State: Exp $
 *
 */

#ifndef RunConfig_hh
#define RunConfig_hh

#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "TObjArray.h"
#include "TObject.h"

#include "DAQConfig/AbsConfList.hh"
#include "DAQConfig/DAQConf.hh"
#include "DAQConfig/FADCSConf.hh"
#include "DAQConfig/FADCTConf.hh"
#include "DAQConfig/GADCSConf.hh"
#include "DAQConfig/GADCTConf.hh"
#include "DAQConfig/IADCTConf.hh"
#include "DAQConfig/MADCSConf.hh"
#include "DAQConfig/SADCSConf.hh"
#include "DAQConfig/SADCTConf.hh"
#include "DAQConfig/STRGConf.hh"
#include "DAQConfig/TCBConf.hh"
#include "DAQConfig/AmoreADCConf.hh"
#include "DAQConfig/TriggerLookupTable.hh"
#include "OnlConsts/onlconsts.hh"

class RunConfig : public TObject {
public:
  static const int kNEXPERIMENT = 6;
  static const int kNMAINMENU = 15;
  enum MAINCONFIG {
    INCLUDE = 1,
    EXPERIMENT = 2,
    DAQMODE = 3,
    TCB = 4,
    FADCT = 5,
    FADCS = 6,
    GADCT = 7,
    GADCS = 8,
    MADCS = 9,
    SADCT = 10,
    SADCS = 11,
    IADCT = 12,
    AMOREADC = 13,
    STRG = 14,
    DAQ = 15
  };

  static const int kNTCBMENU = 21;
  enum TCBCONFIG {
    TM_TCB = 1,
    CW_TCB = 2,
    DLY_TCB = 3,
    PTRG_TCB = 4,
    MTHRF_TCB = 5,
    PSCF_TCB = 6,
    DTF_TCB = 7,
    TRGF_TCB = 8,
    MTHRSM_TCB = 9,
    PSCSM_TCB = 10,
    DTSM_TCB = 11,
    TRGSM_TCB = 12,
    MTHRSL_TCB = 13,
    PSCSL_TCB = 14,
    DTSL_TCB = 15,
    TRGSL_TCB = 16,
    MTHRI_TCB = 17,
    PSCI_TCB = 18,
    DTI_TCB = 19,
    TRGI_TCB = 20,    
    TYPE_TCB = 21
  };

  static const int kNFADCTMENU = 25;
  enum FADCTCONFIG {
    ENABLED_FADCT = 1,
    CID_FADCT = 2,
    PID_FADCT = 3,
    POL_FADCT = 4,
    DACOFF_FADCT = 5,
    DLY_FADCT = 6,
    DTIME_FADCT = 7,
    CW_FADCT = 8,
    TM_FADCT = 9,
    THR_FADCT = 10,
    PCT_FADCT = 11,
    PCI_FADCT = 12,
    PWT_FADCT = 13,
    PSW_FADCT = 14,
    RL_FADCT = 15,
    TLT_FADCT = 16,
    DSR_FADCT = 17,
    PEDRMS_FADCT = 18,
    PGAIN_FADCT = 19,
    PGAINS_FADCT = 20,
    PFTIME_FADCT = 21,
    PTTIME_FADCT = 22,
    PQEFF_FADCT = 23,
    PTTS_FADCT = 24,
    TRGON_FADCT = 25
  };

  static const int kNFADCSMENU = 20;
  enum FADCSCONFIG {
    ENABLED_FADCS = 1,
    CID_FADCS = 2,
    PID_FADCS = 3,
    POL_FADCS = 4,
    DACOFF_FADCS = 5,
    DLY_FADCS = 6,
    DTIME_FADCS = 7,
    CW_FADCS = 8,
    TM_FADCS = 9,
    THR_FADCS = 10,
    PCT_FADCS = 11,
    PCI_FADCS = 12,
    PWT_FADCS = 13,
    PSW_FADCS = 14,
    RL_FADCS = 15,
    TLT_FADCS = 16,
    DSR_FADCS = 17,
    TRGON_FADCS = 18,
    PTRG_FADCS = 19,
    PSC_FADCS = 20
  };

  static const int kNGADCTMENU = 17;
  enum GADCTCONFIG {
    ENABLED_GADCT = 1,
    CID_GADCT = 2,
    PID_GADCT = 3,
    POL_GADCT = 4,
    DACOFF_GADCT = 5,
    DLY_GADCT = 6,
    DTIME_GADCT = 7,
    CW_GADCT = 8,
    TM_GADCT = 9,
    THR_GADCT = 10,
    PCT_GADCT = 11,
    PCI_GADCT = 12,
    PWT_GADCT = 13,
    PSW_GADCT = 14,
    RL_GADCT = 15,
    TLT_GADCT = 16,
    DSR_GADCT = 17,
  };

  static const int kNGADCSMENU = 20;
  enum GADCSCONFIG {
    ENABLED_GADCS = 1,
    CID_GADCS = 2,
    PID_GADCS = 3,
    POL_GADCS = 4,
    DACOFF_GADCS = 5,
    DLY_GADCS = 6,
    DTIME_GADCS = 7,
    CW_GADCS = 8,
    TM_GADCS = 9,
    THR_GADCS = 10,
    PCT_GADCS = 11,
    PCI_GADCS = 12,
    PWT_GADCS = 13,
    PSW_GADCS = 14,
    RL_GADCS = 15,
    TLT_GADCS = 16,
    DSR_GADCS = 17,
    TRGON_GADCS = 18,
    PTRG_GADCS = 19,
    PSC_GADCS = 20
  };

  static const int kNMADCSMENU = 20;
  enum MADCSCONFIG {
    ENABLED_MADCS = 1,
    CID_MADCS = 2,
    PID_MADCS = 3,
    POL_MADCS = 4,
    DACOFF_MADCS = 5,
    DLY_MADCS = 6,
    DTIME_MADCS = 7,
    CW_MADCS = 8,
    TM_MADCS = 9,
    THR_MADCS = 10,
    PCT_MADCS = 11,
    PCI_MADCS = 12,
    PWT_MADCS = 13,
    PSW_MADCS = 14,
    RL_MADCS = 15,
    TLT_MADCS = 16,
    DSR_MADCS = 17,
    TRGON_MADCS = 18,
    PTRG_MADCS = 19,
    PSC_MADCS = 20
  };

  static const int kNSADCTMENU = 17;
  enum SADCTCONFIG {
    ENABLED_SADCT = 1,
    CID_SADCT = 2,
    PID_SADCT = 3,
    CW_SADCT = 4,
    GW_SADCT = 5,
    PSW_SADCT = 6,
    THR_SADCT = 7,
    DLY_SADCT = 8,
    SUBPED_SADCT = 9,
    TLT1_SADCT = 10,
    TLT2_SADCT = 11,
    TLT3_SADCT = 12,
    TLT4_SADCT = 13,
    TLT5_SADCT = 14,
    TLT6_SADCT = 15,
    TLT7_SADCT = 16,
    TLT8_SADCT = 17
  };

  static const int kNSADCSMENU = 20;
  enum SADCSCONFIG {
    ENABLED_SADCS = 1,
    CID_SADCS = 2,
    PID_SADCS = 3,
    CW_SADCS = 4,
    GW_SADCS = 5,
    PSW_SADCS = 6,
    THR_SADCS = 7,
    DLY_SADCS = 8,
    SUBPED_SADCS = 9,
    TLT1_SADCS = 10,
    TLT2_SADCS = 11,
    TLT3_SADCS = 12,
    TLT4_SADCS = 13,
    TLT5_SADCS = 14,
    TLT6_SADCS = 15,
    TLT7_SADCS = 16,
    TLT8_SADCS = 17,
    PTRG_SADCS = 18,
    MTHR_SADCS = 19,
    PSC_SADCS = 20
  };

  static const int kNIADCTMENU = 30;
  enum IADCTCONFIG {
    ENABLED_IADCT = 1,
    CID_IADCT = 2,
    PID_IADCT = 3,
    MODE_IADCT = 4,
    RL_IADCT = 5,
    CW_IADCT = 6,
    GW_IADCT = 7,
    PSW_IADCT = 8,
    THR1_IADCT = 9,
    THR2_IADCT = 10,
    THR3_IADCT = 11,
    THR4_IADCT = 12,
    THR5_IADCT = 13,
    THR6_IADCT = 14,
    THR7_IADCT = 15,
    THR8_IADCT = 16,
    THR9_IADCT = 17,
    THR10_IADCT = 18,
    DLY_IADCT = 19,
    HV_IADCT = 20,
    TLT1_IADCT = 21,
    TLT2_IADCT = 22,
    TLT3_IADCT = 23,
    TLT4_IADCT = 24,
    TLT5_IADCT = 25,
    TLT6_IADCT = 26,
    TLT7_IADCT = 27,
    TLT8_IADCT = 28,
    TLT9_IADCT = 29,
    TLT10_IADCT = 30
  };

  static const int kNAMOREADCMENU = 15;
  enum AMOREADCCONFIG {
    ENABLED_AMOREADC = 1,
    CID_AMOREADC = 2,
    PID_AMOREADC = 3,
    TRGON_AMOREADC = 4,
    ORDER_AMOREADC = 5,
    LOWER_AMOREADC = 6,
    UPPER_AMOREADC = 7,
    THR_AMOREADC = 8,
    DT_AMOREADC = 9,
    SR_AMOREADC = 10,
    RL_AMOREADC = 11,
    DLY_AMOREADC = 12,
    CW_AMOREADC = 13,
    SKBIN_AMOREADC = 14,
    ZEROSUP_AMOREADC = 15
  };

  static const int kNSTRGMENU = 5;
  enum STRGCONFIG {
    ENABLED_STRG = 1,
    ADCTYPE_STRG = 2,
    ZSU_STRG = 3,
    PSC_STRG = 4,
    ICRD_STRG = 5
  };

  static const int kNDAQMENU = 1;
  enum DAQCONFIG { SERVER = 1 };

  RunConfig();
  virtual ~RunConfig();

  bool ReadConfig(const char * fname);
  bool ReadConfig(std::ifstream & ticket);
  AbsConfList * GetConfigs() const;

  void SetDAQMode(int n);
  void SetExperiment(DAQ::EXPERIMENT exp);

  int GetDAQMode() const;
  DAQ::EXPERIMENT GetExperiment() const;

private:
  bool ConfigTCB(std::ifstream & ticket, TCBConf * conf);
  bool ConfigFADCT(std::ifstream & ticket, FADCTConf * conf);
  bool ConfigFADCS(std::ifstream & ticket, FADCSConf * conf);
  bool ConfigGADCT(std::ifstream & ticket, GADCTConf * conf);
  bool ConfigGADCS(std::ifstream & ticket, GADCSConf * conf);
  bool ConfigMADCS(std::ifstream & ticket, MADCSConf * conf);
  bool ConfigSADCT(std::ifstream & ticket, SADCTConf * conf);
  bool ConfigSADCS(std::ifstream & ticket, SADCSConf * conf);
  bool ConfigIADCT(std::ifstream & ticket, IADCTConf * conf);
  bool ConfigAmoreADC(std::ifstream & ticket, AmoreADCConf * conf);
  bool ConfigSTRG(std::ifstream & ticket, STRGConf * conf);
  bool ConfigDAQ(std::ifstream & ticket, DAQConf * conf);

private:
  TriggerLookupTable * fTLT; //!

  std::string EXPERIMENTNAME[kNEXPERIMENT];       //!
  std::string MAINCONFIGMENU[kNMAINMENU];         //!
  std::string TCBCONFIGMENU[kNTCBMENU];           //!
  std::string FADCTCONFIGMENU[kNFADCTMENU];       //!
  std::string FADCSCONFIGMENU[kNFADCSMENU];       //!
  std::string GADCTCONFIGMENU[kNGADCTMENU];       //!
  std::string GADCSCONFIGMENU[kNGADCSMENU];       //!
  std::string MADCSCONFIGMENU[kNMADCSMENU];       //!
  std::string SADCTCONFIGMENU[kNSADCTMENU];       //!
  std::string SADCSCONFIGMENU[kNSADCSMENU];       //!
  std::string IADCTCONFIGMENU[kNIADCTMENU];       //!
  std::string AMOREADCCONFIGMENU[kNAMOREADCMENU]; //!
  std::string STRGCONFIGMENU[kNSTRGMENU];         //!
  std::string DAQCONFIGMENU[kNDAQMENU];           //!

  std::map<std::string, int> fExpeimentItem; //!
  std::map<std::string, int> fMainItem;      //!
  std::map<std::string, int> fTCBItem;       //!
  std::map<std::string, int> fFADCTItem;     //!
  std::map<std::string, int> fFADCSItem;     //!
  std::map<std::string, int> fGADCTItem;     //!
  std::map<std::string, int> fGADCSItem;     //!
  std::map<std::string, int> fMADCSItem;     //!
  std::map<std::string, int> fSADCTItem;     //!
  std::map<std::string, int> fSADCSItem;     //!
  std::map<std::string, int> fIADCTItem;     //!
  std::map<std::string, int> fAMOREADCItem;  //!
  std::map<std::string, int> fSTRGItem;      //!
  std::map<std::string, int> fDAQItem;       //!

  int fDAQMode;
  DAQ::EXPERIMENT fExperiment;

  AbsConfList * fConfigs;

  ClassDef(RunConfig, 1)
};

inline AbsConfList * RunConfig::GetConfigs() const { return fConfigs; }

inline void RunConfig::SetDAQMode(int n) { fDAQMode = n; }

inline int RunConfig::GetDAQMode() const { return fDAQMode; }

inline void RunConfig::SetExperiment(DAQ::EXPERIMENT exp) { fExperiment = exp; }

inline DAQ::EXPERIMENT RunConfig::GetExperiment() const { return fExperiment; }

#endif

/**
$Log: RunConfig.hh,v $
Revision 1.28  2023/07/18 12:25:11  cupsoft
*** empty log message ***

Revision 1.27  2023/03/29 23:05:27  cupsoft
*** empty log message ***

Revision 1.26  2023/03/23 04:32:42  cupsoft
*** empty log message ***

Revision 1.25  2023/01/12 03:46:12  cupsoft
add DAQconfig

Revision 1.24  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.24  2020/07/10 02:38:36  cupsoft
*** empty log message ***

Revision 1.23  2019/11/25 02:04:07  cupsoft
*** empty log message ***

Revision 1.22  2019/07/24 07:04:23  cupsoft
*** empty log message ***

Revision 1.21  2019/07/12 07:18:42  cupsoft
*** empty log message ***

Revision 1.20  2019/07/11 08:22:10  cupsoft
*** empty log message ***

Revision 1.19  2019/07/09 07:39:06  cupsoft
*** empty log message ***

Revision 1.18  2019/05/30 23:59:05  cupsoft
*** empty log message ***

Revision 1.17  2018/11/01 01:40:44  cupsoft
*** empty log message ***

Revision 1.16  2018/09/20 01:01:10  cupsoft
*** empty log message ***

Revision 1.15  2018/04/30 05:42:41  cupsoft
*** empty log message ***

Revision 1.14  2018/01/08 05:14:45  cupsoft
*** empty log message ***

Revision 1.13  2017/06/12 02:12:21  cupsoft
*** empty log message ***

Revision 1.12  2017/05/19 01:58:58  cupsoft
*** empty log message ***

Revision 1.11  2017/04/06 23:52:36  cupsoft
*** empty log message ***

Revision 1.10  2017/04/06 12:11:56  cupsoft
*** empty log message ***

Revision 1.9  2017/03/08 05:21:44  cupsoft
*** empty log message ***

Revision 1.8  2017/01/25 02:14:18  cupsoft
*** empty log message ***

Revision 1.7  2016/12/14 04:54:26  cupsoft
*** empty log message ***

Revision 1.6  2016/12/05 09:52:50  cupsoft
*** empty log message ***

Revision 1.5  2016/11/12 04:05:28  cupsoft
*** empty log message ***

Revision 1.4  2016/10/02 10:55:13  cupsoft
*** empty log message ***

Revision 1.3  2016/10/03 06:57:03  cupsoft
*** empty log message ***

Revision 1.2  2016/10/01 17:10:53  cupsoft
clean up

Revision 1.1.1.1  2016/09/30 09:22:24  cupsoft
DAQConfig

Revision 1.4  2016/09/28 16:54:49  cupsoft
*** empty log message ***

Revision 1.3  2016/09/27 03:41:52  cupsoft
*** empty log message ***

Revision 1.2  2016/09/05 05:26:48  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/07/14 07:42:30  cupsoft
DAQSys

Revision 1.3  2016/05/10 01:57:35  cupsoft
*** empty log message ***

Revision 1.2  2016/03/23 03:26:13  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/03/08 04:37:46  amore
DAQSts

Revision 1.1.1.1  2016/02/29 08:26:44  cupsoft
DAQSys

**/
