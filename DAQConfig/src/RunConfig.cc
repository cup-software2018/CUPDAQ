/*
 *
 *  Module:  RunConfig
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: Class for configuring DAQ frontend electronics
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/07/18 12:25:11 $
 *  CVS/RCS Revision: $Revision: 1.33 $
 *  Status:           $State: Exp $
 *
 */

#include "DAQConfig/RunConfig.hh"

#include <algorithm>
#include <sstream>

#include "TObjString.h"
#include "TString.h"

using namespace std;

ClassImp(RunConfig)

    RunConfig::RunConfig()
    : TObject()
{
  fDAQMode = 0;
  fExperiment = DAQ::NONE;

  EXPERIMENTNAME[0] = "NONE";
  EXPERIMENTNAME[1] = "NEOS";
  EXPERIMENTNAME[2] = "COSINE";
  EXPERIMENTNAME[3] = "NEON";
  EXPERIMENTNAME[4] = "AMORE1";
  EXPERIMENTNAME[5] = "AMORE2";

  for (int i = 0; i < kNEXPERIMENT; i++)
    fExpeimentItem.insert(make_pair(EXPERIMENTNAME[i], i));

  fConfigs = new AbsConfList();

  MAINCONFIGMENU[0] = "INCLUDE";
  MAINCONFIGMENU[1] = "EXPERIMENT";
  MAINCONFIGMENU[2] = "DAQMODE";
  MAINCONFIGMENU[3] = "TCB";
  MAINCONFIGMENU[4] = "FADCT";
  MAINCONFIGMENU[5] = "FADCS";
  MAINCONFIGMENU[6] = "GADCT";
  MAINCONFIGMENU[7] = "GADCS";
  MAINCONFIGMENU[8] = "MADCS";
  MAINCONFIGMENU[9] = "SADCT";
  MAINCONFIGMENU[10] = "SADCS";
  MAINCONFIGMENU[11] = "IADCT";
  MAINCONFIGMENU[12] = "AMOREADC";
  MAINCONFIGMENU[13] = "STRG";
  MAINCONFIGMENU[14] = "DAQ";

  for (int i = 0; i < kNMAINMENU; i++)
    fMainItem.insert(make_pair(MAINCONFIGMENU[i], i + 1));

  TCBCONFIGMENU[0] = "TRGON";
  TCBCONFIGMENU[1] = "CW";
  TCBCONFIGMENU[2] = "DLY";
  TCBCONFIGMENU[3] = "PTRIG";
  TCBCONFIGMENU[4] = "MTHRF";
  TCBCONFIGMENU[5] = "PSCF";
  TCBCONFIGMENU[6] = "DTF";
  TCBCONFIGMENU[7] = "SWF";
  TCBCONFIGMENU[8] = "MTHRSM";
  TCBCONFIGMENU[9] = "PSCSM";
  TCBCONFIGMENU[10] = "DTSM";
  TCBCONFIGMENU[11] = "SWSM";
  TCBCONFIGMENU[12] = "MTHRSL";
  TCBCONFIGMENU[13] = "PSCSL";
  TCBCONFIGMENU[14] = "DTSL";
  TCBCONFIGMENU[15] = "SWSL";
  TCBCONFIGMENU[16] = "MTHRI";
  TCBCONFIGMENU[17] = "PSCI";
  TCBCONFIGMENU[18] = "DTI";
  TCBCONFIGMENU[19] = "SWI";
  TCBCONFIGMENU[20] = "TYPE";

  for (int i = 0; i < kNTCBMENU; i++)
    fTCBItem.insert(make_pair(TCBCONFIGMENU[i], i + 1));

  FADCTCONFIGMENU[0] = "ENABLED";
  FADCTCONFIGMENU[1] = "CID";
  FADCTCONFIGMENU[2] = "PID";
  FADCTCONFIGMENU[3] = "POL";
  FADCTCONFIGMENU[4] = "DACOFF";
  FADCTCONFIGMENU[5] = "DLY";
  FADCTCONFIGMENU[6] = "DTIME";
  FADCTCONFIGMENU[7] = "CW";
  FADCTCONFIGMENU[8] = "TM";
  FADCTCONFIGMENU[9] = "THR";
  FADCTCONFIGMENU[10] = "PCT";
  FADCTCONFIGMENU[11] = "PCI";
  FADCTCONFIGMENU[12] = "PWT";
  FADCTCONFIGMENU[13] = "PSW";
  FADCTCONFIGMENU[14] = "RL";
  FADCTCONFIGMENU[15] = "TLT";
  FADCTCONFIGMENU[16] = "DSR";
  FADCTCONFIGMENU[17] = "PEDRMS";
  FADCTCONFIGMENU[18] = "PGAIN";
  FADCTCONFIGMENU[19] = "PGAINS";
  FADCTCONFIGMENU[20] = "PFTIME";
  FADCTCONFIGMENU[21] = "PTTIME";
  FADCTCONFIGMENU[22] = "PTTS";
  FADCTCONFIGMENU[23] = "PQEFF";
  FADCTCONFIGMENU[24] = "TRGON";

  for (int i = 0; i < kNFADCTMENU; i++)
    fFADCTItem.insert(make_pair(FADCTCONFIGMENU[i], i + 1));

  FADCSCONFIGMENU[0] = "ENABLED";
  FADCSCONFIGMENU[1] = "CID";
  FADCSCONFIGMENU[2] = "PID";
  FADCSCONFIGMENU[3] = "POL";
  FADCSCONFIGMENU[4] = "DACOFF";
  FADCSCONFIGMENU[5] = "DLY";
  FADCSCONFIGMENU[6] = "DTIME";
  FADCSCONFIGMENU[7] = "CW";
  FADCSCONFIGMENU[8] = "TM";
  FADCSCONFIGMENU[9] = "THR";
  FADCSCONFIGMENU[10] = "PCT";
  FADCSCONFIGMENU[11] = "PCI";
  FADCSCONFIGMENU[12] = "PWT";
  FADCSCONFIGMENU[13] = "PSW";
  FADCSCONFIGMENU[14] = "RL";
  FADCSCONFIGMENU[15] = "TLT";
  FADCSCONFIGMENU[16] = "DSR";
  FADCSCONFIGMENU[17] = "TRGON";
  FADCSCONFIGMENU[18] = "PTRG";
  FADCSCONFIGMENU[19] = "PSC";

  for (int i = 0; i < kNFADCSMENU; i++)
    fFADCSItem.insert(make_pair(FADCSCONFIGMENU[i], i + 1));

  GADCTCONFIGMENU[0] = "ENABLED";
  GADCTCONFIGMENU[1] = "CID";
  GADCTCONFIGMENU[2] = "PID";
  GADCTCONFIGMENU[3] = "POL";
  GADCTCONFIGMENU[4] = "DACOFF";
  GADCTCONFIGMENU[5] = "DLY";
  GADCTCONFIGMENU[6] = "DTIME";
  GADCTCONFIGMENU[7] = "CW";
  GADCTCONFIGMENU[8] = "TM";
  GADCTCONFIGMENU[9] = "THR";
  GADCTCONFIGMENU[10] = "PCT";
  GADCTCONFIGMENU[11] = "PCI";
  GADCTCONFIGMENU[12] = "PWT";
  GADCTCONFIGMENU[13] = "PSW";
  GADCTCONFIGMENU[14] = "RL";
  GADCTCONFIGMENU[15] = "TLT";
  GADCTCONFIGMENU[16] = "DSR";

  for (int i = 0; i < kNGADCTMENU; i++)
    fGADCTItem.insert(make_pair(GADCTCONFIGMENU[i], i + 1));

  GADCSCONFIGMENU[0] = "ENABLED";
  GADCSCONFIGMENU[1] = "CID";
  GADCSCONFIGMENU[2] = "PID";
  GADCSCONFIGMENU[3] = "POL";
  GADCSCONFIGMENU[4] = "DACOFF";
  GADCSCONFIGMENU[5] = "DLY";
  GADCSCONFIGMENU[6] = "DTIME";
  GADCSCONFIGMENU[7] = "CW";
  GADCSCONFIGMENU[8] = "TM";
  GADCSCONFIGMENU[9] = "THR";
  GADCSCONFIGMENU[10] = "PCT";
  GADCSCONFIGMENU[11] = "PCI";
  GADCSCONFIGMENU[12] = "PWT";
  GADCSCONFIGMENU[13] = "PSW";
  GADCSCONFIGMENU[14] = "RL";
  GADCSCONFIGMENU[15] = "TLT";
  GADCSCONFIGMENU[16] = "DSR";
  GADCSCONFIGMENU[17] = "TRGON";
  GADCSCONFIGMENU[18] = "PTRG";
  GADCSCONFIGMENU[19] = "PSC";

  for (int i = 0; i < kNGADCSMENU; i++)
    fGADCSItem.insert(make_pair(GADCSCONFIGMENU[i], i + 1));

  MADCSCONFIGMENU[0] = "ENABLED";
  MADCSCONFIGMENU[1] = "CID";
  MADCSCONFIGMENU[2] = "PID";
  MADCSCONFIGMENU[3] = "POL";
  MADCSCONFIGMENU[4] = "DACOFF";
  MADCSCONFIGMENU[5] = "DLY";
  MADCSCONFIGMENU[6] = "DTIME";
  MADCSCONFIGMENU[7] = "CW";
  MADCSCONFIGMENU[8] = "TM";
  MADCSCONFIGMENU[9] = "THR";
  MADCSCONFIGMENU[10] = "PCT";
  MADCSCONFIGMENU[11] = "PCI";
  MADCSCONFIGMENU[12] = "PWT";
  MADCSCONFIGMENU[13] = "PSW";
  MADCSCONFIGMENU[14] = "RL";
  MADCSCONFIGMENU[15] = "TLT";
  MADCSCONFIGMENU[16] = "DSR";
  MADCSCONFIGMENU[17] = "TRGON";
  MADCSCONFIGMENU[18] = "PTRG";
  MADCSCONFIGMENU[19] = "PSC";

  for (int i = 0; i < kNMADCSMENU; i++)
    fMADCSItem.insert(make_pair(MADCSCONFIGMENU[i], i + 1));

  SADCTCONFIGMENU[0] = "ENABLED";
  SADCTCONFIGMENU[1] = "CID";
  SADCTCONFIGMENU[2] = "PID";
  SADCTCONFIGMENU[3] = "CW";
  SADCTCONFIGMENU[4] = "GW";
  SADCTCONFIGMENU[5] = "PSW";
  SADCTCONFIGMENU[6] = "THR";
  SADCTCONFIGMENU[7] = "DLY";
  SADCTCONFIGMENU[8] = "SUBPED";
  SADCTCONFIGMENU[9] = "TLT1";
  SADCTCONFIGMENU[10] = "TLT2";
  SADCTCONFIGMENU[11] = "TLT3";
  SADCTCONFIGMENU[12] = "TLT4";
  SADCTCONFIGMENU[13] = "TLT5";
  SADCTCONFIGMENU[14] = "TLT6";
  SADCTCONFIGMENU[15] = "TLT7";
  SADCTCONFIGMENU[16] = "TLT8";

  for (int i = 0; i < kNSADCTMENU; i++)
    fSADCTItem.insert(make_pair(SADCTCONFIGMENU[i], i + 1));

  SADCSCONFIGMENU[0] = "ENABLED";
  SADCSCONFIGMENU[1] = "CID";
  SADCSCONFIGMENU[2] = "PID";
  SADCSCONFIGMENU[3] = "CW";
  SADCSCONFIGMENU[4] = "GW";
  SADCSCONFIGMENU[5] = "PSW";
  SADCSCONFIGMENU[6] = "THR";
  SADCSCONFIGMENU[7] = "DLY";
  SADCSCONFIGMENU[8] = "SUBPED";
  SADCSCONFIGMENU[9] = "TLT1";
  SADCSCONFIGMENU[10] = "TLT2";
  SADCSCONFIGMENU[11] = "TLT3";
  SADCSCONFIGMENU[12] = "TLT4";
  SADCSCONFIGMENU[13] = "TLT5";
  SADCSCONFIGMENU[14] = "TLT6";
  SADCSCONFIGMENU[15] = "TLT7";
  SADCSCONFIGMENU[16] = "TLT8";
  SADCSCONFIGMENU[17] = "PTRG";
  SADCSCONFIGMENU[18] = "MTHR";
  SADCSCONFIGMENU[19] = "PSC";

  for (int i = 0; i < kNSADCSMENU; i++)
    fSADCSItem.insert(make_pair(SADCSCONFIGMENU[i], i + 1));

  IADCTCONFIGMENU[0] = "ENABLED";
  IADCTCONFIGMENU[1] = "CID";
  IADCTCONFIGMENU[2] = "PID";
  IADCTCONFIGMENU[3] = "MODE";
  IADCTCONFIGMENU[4] = "RL";
  IADCTCONFIGMENU[5] = "CW";
  IADCTCONFIGMENU[6] = "GW";
  IADCTCONFIGMENU[7] = "PSW";
  IADCTCONFIGMENU[8] = "THR1";
  IADCTCONFIGMENU[9] = "THR2";
  IADCTCONFIGMENU[10] = "THR3";
  IADCTCONFIGMENU[11] = "THR4";
  IADCTCONFIGMENU[12] = "THR5";
  IADCTCONFIGMENU[13] = "THR6";
  IADCTCONFIGMENU[14] = "THR7";
  IADCTCONFIGMENU[15] = "THR8";
  IADCTCONFIGMENU[16] = "THR9";
  IADCTCONFIGMENU[17] = "THR10";
  IADCTCONFIGMENU[18] = "DLY";
  IADCTCONFIGMENU[19] = "HV";
  IADCTCONFIGMENU[20] = "TLT1";
  IADCTCONFIGMENU[21] = "TLT2";
  IADCTCONFIGMENU[22] = "TLT3";
  IADCTCONFIGMENU[23] = "TLT4";
  IADCTCONFIGMENU[24] = "TLT5";
  IADCTCONFIGMENU[25] = "TLT6";
  IADCTCONFIGMENU[26] = "TLT7";
  IADCTCONFIGMENU[27] = "TLT8";
  IADCTCONFIGMENU[28] = "TLT9";
  IADCTCONFIGMENU[29] = "TLT10";

  for (int i = 0; i < kNIADCTMENU; i++)
    fIADCTItem.insert(make_pair(IADCTCONFIGMENU[i], i + 1));

  AMOREADCCONFIGMENU[0] = "ENABLED";
  AMOREADCCONFIGMENU[1] = "CID";
  AMOREADCCONFIGMENU[2] = "PID";
  AMOREADCCONFIGMENU[3] = "TRGON";
  AMOREADCCONFIGMENU[4] = "ORDER";
  AMOREADCCONFIGMENU[5] = "LOWER";
  AMOREADCCONFIGMENU[6] = "UPPER";
  AMOREADCCONFIGMENU[7] = "THR";
  AMOREADCCONFIGMENU[8] = "DT";
  AMOREADCCONFIGMENU[9] = "SR";
  AMOREADCCONFIGMENU[10] = "RL";
  AMOREADCCONFIGMENU[11] = "DLY";
  AMOREADCCONFIGMENU[12] = "CW";
  AMOREADCCONFIGMENU[13] = "SKBIN";
  AMOREADCCONFIGMENU[14] = "ZEROSUP";

  for (int i = 0; i < kNAMOREADCMENU; i++)
    fAMOREADCItem.insert(make_pair(AMOREADCCONFIGMENU[i], i + 1));

  STRGCONFIGMENU[0] = "ENABLED";
  STRGCONFIGMENU[1] = "ADCTYPE";
  STRGCONFIGMENU[2] = "ZSU";
  STRGCONFIGMENU[3] = "PSC";
  STRGCONFIGMENU[4] = "ICRD";

  for (int i = 0; i < kNSTRGMENU; i++)
    fSTRGItem.insert(make_pair(STRGCONFIGMENU[i], i + 1));

  DAQCONFIGMENU[0] = "SERVER";

  for (int i = 0; i < kNDAQMENU; i++)
    fDAQItem.insert(make_pair(DAQCONFIGMENU[i], i + 1));

  fTLT = new TriggerLookupTable();
}

RunConfig::~RunConfig()
{
  fConfigs->Clear();

  delete fConfigs;
  delete fTLT;
}

bool RunConfig::ReadConfig(const char * fname)
{
  ifstream input;
  input.open(fname, ifstream::in);
  if (input.is_open()) { return ReadConfig(input); }

  return false;
}

bool RunConfig::ReadConfig(ifstream & ticket)
{
  bool retVal = true;
  string line;

  while (ticket.good()) {
    getline(ticket, line);

    if (line.empty()) continue;

    istringstream iss(line);

    string key;
    int val = 0;

    iss >> key;
    if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

    switch (fMainItem[key]) {
      case INCLUDE: {
        string fname;
        iss >> fname;

        ifstream ifs(fname);
        if (ifs.is_open()) {
          ReadConfig(ifs);
        }
        else {
          Warning("ReadConfig", Form("there is no file %s.", fname));
        }
        
        break;
      }
      case DAQMODE: {
        int mode;
        iss >> mode;
        fDAQMode = mode;
        break;
      }
      case EXPERIMENT: {
        string exp;
        iss >> exp;
        if (fExpeimentItem.find(exp) != fExpeimentItem.end()) {
          fExperiment = static_cast<DAQ::EXPERIMENT>(fExpeimentItem[exp]);
        }
        else {
          fExperiment = DAQ::NONE;
          Warning("ReadConfig",
                  Form("experiment %s is ambiguous.", exp.data()));
        }
        break;
      }
      case TCB: {
        auto * conf = new TCBConf(0);
        fConfigs->Add(conf);

        retVal = ConfigTCB(ticket, conf);
        break;
      }
      case FADCT: {
        iss >> val;
        auto * conf = new FADCTConf(val);

        int nch;
        iss >> nch;
        conf->SetNCH(nch);

        int daqid;
        iss >> daqid;
        if (!iss.fail()) { conf->SetDAQID(daqid); }

        fConfigs->Add(conf);

        retVal = ConfigFADCT(ticket, conf);
        break;
      }
      case FADCS: {
        iss >> val;
        auto * conf = new FADCSConf(val);

        int nch;
        iss >> nch;

        conf->SetNCH(nch);
        fConfigs->Add(conf);

        retVal = ConfigFADCS(ticket, conf);
        break;
      }
      case GADCT: {
        iss >> val;
        auto * conf = new GADCTConf(val);

        int nch;
        iss >> nch;
        conf->SetNCH(nch);

        int daqid;
        iss >> daqid;
        if (!iss.fail()) { conf->SetDAQID(daqid); }

        fConfigs->Add(conf);

        retVal = ConfigGADCT(ticket, conf);
        break;
      }
      case GADCS: {
        iss >> val;
        auto * conf = new GADCSConf(val);

        int nch;
        iss >> nch;

        conf->SetNCH(nch);
        fConfigs->Add(conf);

        retVal = ConfigGADCS(ticket, conf);
        break;
      }
      case MADCS: {
        iss >> val;
        auto * conf = new MADCSConf(val);

        int nch;
        iss >> nch;

        conf->SetNCH(nch);
        fConfigs->Add(conf);

        retVal = ConfigMADCS(ticket, conf);
        break;
      }
      case SADCT: {
        iss >> val;
        auto * conf = new SADCTConf(val);

        int nch;
        iss >> nch;

        conf->SetNCH(nch);
        fConfigs->Add(conf);

        int daqid;
        iss >> daqid;
        if (!iss.fail()) { conf->SetDAQID(daqid); }

        retVal = ConfigSADCT(ticket, conf);
        break;
      }
      case SADCS: {
        iss >> val;
        auto * conf = new SADCSConf(val);

        int nch;
        iss >> nch;

        conf->SetNCH(nch);
        fConfigs->Add(conf);

        retVal = ConfigSADCS(ticket, conf);
        break;
      }
      case IADCT: {
        iss >> val;
        auto * conf = new IADCTConf(val);

        int nch;
        iss >> nch;

        conf->SetNCH(nch);
        fConfigs->Add(conf);

        int daqid;
        iss >> daqid;
        if (!iss.fail()) { conf->SetDAQID(daqid); }

        retVal = ConfigIADCT(ticket, conf);
        break;
      }
      case AMOREADC: {
        iss >> val;
        auto * conf = new AmoreADCConf(val);

        int nch;
        iss >> nch;

        conf->SetNCH(nch);
        fConfigs->Add(conf);

        int daqid;
        iss >> daqid;
        if (!iss.fail()) { conf->SetDAQID(daqid); }

        retVal = ConfigAmoreADC(ticket, conf);
        break;
      }
      case STRG: {
        auto * conf = new STRGConf();
        retVal = ConfigSTRG(ticket, conf);
        fConfigs->Add(conf);
        break;
      }
      case DAQ: {
        auto * conf = new DAQConf();
        retVal = ConfigDAQ(ticket, conf);
        fConfigs->Add(conf);
        break;
      }
      default: {
        Warning("ReadConfig",
                Form("Configuration item %s is ambiguous.", key.data()));
        break;
      }
    }
  }

  if (retVal) fConfigs->Sort();

  return retVal;
}

bool RunConfig::ConfigTCB(std::ifstream & ticket, TCBConf * conf)
{
  bool retVal = true;

  string line;

  while (true) {
    getline(ticket, line);

    if (line == "END") break;
    if (line.empty()) continue;

    istringstream iss(line);

    string key;
    int val[4] = {0};

    iss >> key;
    if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

    switch (fTCBItem[key]) {
      case TM_TCB: {
        iss >> val[0];
        conf->SetTM(val[0]);
        break;
      }
      case CW_TCB: {
        iss >> val[0];
        conf->SetCW(val[0]);
        break;
      }
      case DLY_TCB: {
        iss >> val[0];
        conf->SetDLY(val[0]);
        break;
      }
      case PTRG_TCB: {
        iss >> val[0];
        conf->SetPTRG(val[0]);
        break;
      }
      case MTHRF_TCB: {
        iss >> val[0];
        conf->SetMTHRF(val[0]);
        break;
      }
      case PSCF_TCB: {
        iss >> val[0];
        conf->SetPSCF(val[0]);
        break;
      }
      case DTF_TCB: {
        iss >> val[0];
        conf->SetDTF(val[0]);
        break;
      }
      case SWF_TCB: {
        iss >> val[0] >> val[1] >> val[2] >> val[3];
        conf->SetSWF(val[0], val[1], val[2], val[3]);
        break;
      }
      case MTHRSM_TCB: {
        iss >> val[0];
        conf->SetMTHRSM(val[0]);
        break;
      }
      case PSCSM_TCB: {
        iss >> val[0];
        conf->SetPSCSM(val[0]);
        break;
      }
      case DTSM_TCB: {
        iss >> val[0];
        conf->SetDTSM(val[0]);
        break;
      }
      case SWSM_TCB: {
        iss >> val[0] >> val[1] >> val[2] >> val[3];
        conf->SetSWSM(val[0], val[1], val[2], val[3]);
        break;
      }
      case MTHRSL_TCB: {
        iss >> val[0];
        conf->SetMTHRSL(val[0]);
        break;
      }
      case PSCSL_TCB: {
        iss >> val[0];
        conf->SetPSCSL(val[0]);
        break;
      }
      case DTSL_TCB: {
        iss >> val[0];
        conf->SetDTSL(val[0]);
        break;
      }
      case SWSL_TCB: {
        iss >> val[0] >> val[1] >> val[2] >> val[3];
        conf->SetSWSL(val[0], val[1], val[2], val[3]);
        break;
      }
      case MTHRI_TCB: {
        iss >> val[0];
        conf->SetMTHRI(val[0]);
        break;
      }
      case PSCI_TCB: {
        iss >> val[0];
        conf->SetPSCI(val[0]);
        break;
      }
      case DTI_TCB: {
        iss >> val[0];
        conf->SetDTI(val[0]);
        break;
      }
      case SWI_TCB: {
        iss >> val[0] >> val[1] >> val[2] >> val[3];
        conf->SetSWI(val[0], val[1], val[2], val[3]);
        break;
      }
      case TYPE_TCB: {
        iss >> val[0];
        conf->SetTCBTYPE(static_cast<TCB::TYPE>(val[0]));
        break;
      }
      default: {
        Warning("ConfigTCB",
                Form("Configuration item %s is ambiguous.", key.data()));
        break;
      }
    }
  }

  return retVal;
}

bool RunConfig::ConfigFADCT(std::ifstream & ticket, FADCTConf * conf)
{
  bool retVal = true;

  int nch = conf->NCH();

  string line;

  while (true) {
    getline(ticket, line);

    if (line == "END") break;
    if (line.empty()) continue;

    istringstream iss(line);

    string key;
    int val[4] = {0};
    double dval[4] = {0.};

    iss >> key;
    if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

    switch (fFADCTItem[key]) {
      case ENABLED_FADCT: {
        iss >> val[0];
        if (val[0] > 0) conf->SetEnable();
        break;
      }
      case CID_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Error("ConfigFADCT", "Channel ID is not defined ....");
            retVal = false;
            break;
          }
          conf->SetCID(i, val[i]);
        }
        break;
      }
      case PID_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Warning("ConfigFADCT", "PMT ID is not defined ....");
          }
          conf->SetPID(i, val[i]);
        }
        break;
      }
      case POL_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPOL(i, val[0]);
          }
          else {
            conf->SetPOL(i, val[i]);
          }
        }
        break;
      }
      case DACOFF_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDACOFF(i, val[0]);
          }
          else {
            conf->SetDACOFF(i, val[i]);
          }
        }
        break;
      }
      case DLY_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDLY(i, val[0]);
          }
          else {
            conf->SetDLY(i, val[i]);
          }
        }
        break;
      }
      case DTIME_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDT(i, val[0]);
          }
          else {
            conf->SetDT(i, val[i]);
          }
        }
        break;
      }
      case CW_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetCW(i, val[0]);
          }
          else {
            conf->SetCW(i, val[i]);
          }
        }
        break;
      }
      case TM_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetTM(i, val[0]);
          }
          else {
            conf->SetTM(i, val[i]);
          }
        }
        break;
      }
      case THR_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetTHR(i, val[0]);
          }
          else {
            conf->SetTHR(i, val[i]);
          }
        }
        break;
      }
      case PCT_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPCT(i, val[0]);
          }
          else {
            conf->SetPCT(i, val[i]);
          }
        }
        break;
      }
      case PCI_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPCI(i, val[0]);
          }
          else {
            conf->SetPCI(i, val[i]);
          }
        }
        break;
      }
      case PWT_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPWT(i, val[0]);
          }
          else {
            conf->SetPWT(i, val[i]);
          }
        }
        break;
      }
      case PSW_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPSW(i, val[0]);
          }
          else {
            conf->SetPSW(i, val[i]);
          }
        }
        break;
      }
      case PEDRMS_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> dval[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPedRMS(i, dval[0]);
          }
          else {
            conf->SetPedRMS(i, dval[i]);
          }
        }
        break;
      }
      case PGAIN_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> dval[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPMTGain(i, dval[0]);
          }
          else {
            conf->SetPMTGain(i, dval[i]);
          }
        }
        break;
      }
      case PGAINS_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> dval[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPMTGainScale(i, dval[0]);
          }
          else {
            conf->SetPMTGainScale(i, dval[i]);
          }
        }
        break;
      }
      case PFTIME_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> dval[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPMTFallTime(i, dval[0]);
          }
          else {
            conf->SetPMTFallTime(i, dval[i]);
          }
        }
        break;
      }
      case PTTIME_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> dval[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPMTTransitTime(i, dval[0]);
          }
          else {
            conf->SetPMTTransitTime(i, dval[i]);
          }
        }
        break;
      }
      case PTTS_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> dval[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPMTTTS(i, dval[0]);
          }
          else {
            conf->SetPMTTTS(i, dval[i]);
          }
        }
        break;
      }
      case PQEFF_FADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> dval[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPMTQEff(i, dval[0]);
          }
          else {
            conf->SetPMTQEff(i, dval[i]);
          }
        }
        break;
      }
      case RL_FADCT: {
        iss >> val[0];
        conf->SetRL(val[0]);
        break;
      }
      case TLT_FADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(fTLT->GetTLT(tlt.data()));
        break;
      }
      case DSR_FADCT: {
        iss >> val[0];
        conf->SetDSR(val[0]);
        break;
      }
      case TRGON_FADCT: {
        iss >> val[0];
        conf->SetTRGON(val[0]);
        break;
      }
      default: {
        Warning("ConfigFADCT",
                Form("Configuration item %s is ambiguous.", key.data()));
        break;
      }
    }
  }

  return retVal;
}

bool RunConfig::ConfigFADCS(std::ifstream & ticket, FADCSConf * conf)
{
  bool retVal = true;

  int nch = conf->NCH();

  string line;

  while (true) {
    getline(ticket, line);

    if (line == "END") break;
    if (line.empty()) continue;

    istringstream iss(line);

    string key;
    int val[4] = {0};

    iss >> key;
    if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

    switch (fFADCSItem[key]) {
      case ENABLED_FADCS: {
        iss >> val[0];
        if (val[0] > 0) conf->SetEnable();
        break;
      }
      case CID_FADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Error("ConfigFADCS", "Channel ID is not defined ....");
            retVal = false;
            break;
          }
          conf->SetCID(i, val[i]);
        }
        break;
      }
      case PID_FADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Warning("ConfigFADCS", "PMT ID is not defined ....");
          }
          conf->SetPID(i, val[i]);
        }
        break;
      }
      case POL_FADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPOL(i, val[0]);
          }
          else {
            conf->SetPOL(i, val[i]);
          }
        }
        break;
      }
      case DACOFF_FADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDACOFF(i, val[0]);
          }
          else {
            conf->SetDACOFF(i, val[i]);
          }
        }
        break;
      }
      case DLY_FADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDLY(i, val[0]);
          }
          else {
            conf->SetDLY(i, val[i]);
          }
        }
        break;
      }
      case DTIME_FADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDT(i, val[0]);
          }
          else {
            conf->SetDT(i, val[i]);
          }
        }
        break;
      }
      case CW_FADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetCW(i, val[0]);
          }
          else {
            conf->SetCW(i, val[i]);
          }
        }
        break;
      }
      case TM_FADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetTM(i, val[0]);
          }
          else {
            conf->SetTM(i, val[i]);
          }
        }
        break;
      }
      case THR_FADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetTHR(i, val[0]);
          }
          else {
            conf->SetTHR(i, val[i]);
          }
        }
        break;
      }
      case PCT_FADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPCT(i, val[0]);
          }
          else {
            conf->SetPCT(i, val[i]);
          }
        }
        break;
      }
      case PCI_FADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPCI(i, val[0]);
          }
          else {
            conf->SetPCI(i, val[i]);
          }
        }
        break;
      }
      case PWT_FADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPWT(i, val[0]);
          }
          else {
            conf->SetPWT(i, val[i]);
          }
        }
        break;
      }
      case PSW_FADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigFADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPSW(i, val[0]);
          }
          else {
            conf->SetPSW(i, val[i]);
          }
        }
        break;
      }
      case RL_FADCS: {
        iss >> val[0];
        conf->SetRL(val[0]);
        break;
      }
      case TLT_FADCS: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(fTLT->GetTLT(tlt.data()));
        break;
      }
      case DSR_FADCS: {
        iss >> val[0];
        conf->SetDSR(val[0]);
        break;
      }
      case TRGON_FADCS: {
        iss >> val[0];
        conf->SetTRGON(val[0]);
        break;
      }
      case PTRG_FADCS: {
        iss >> val[0];
        conf->SetPTRG(val[0]);
        break;
      }
      case PSC_FADCS: {
        iss >> val[0];
        conf->SetPSC(val[0]);
        break;
      }
      default: {
        Warning("ConfigFADCS",
                Form("Configuration item %s is ambiguous.", key.data()));
        break;
      }
    }
  }

  return retVal;
}

bool RunConfig::ConfigGADCT(std::ifstream & ticket, GADCTConf * conf)
{
  bool retVal = true;

  int nch = conf->NCH();

  string line;

  while (true) {
    getline(ticket, line);

    if (line == "END") break;
    if (line.empty()) continue;

    istringstream iss(line);

    string key;
    int val[4] = {0};

    iss >> key;
    if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

    switch (fGADCTItem[key]) {
      case ENABLED_GADCT: {
        iss >> val[0];
        if (val[0] > 0) conf->SetEnable();
        break;
      }
      case CID_GADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Error("ConfigGADCT", "Channel ID is not defined ....");
            retVal = false;
            break;
          }
          conf->SetCID(i, val[i]);
        }
        break;
      }
      case PID_GADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Warning("ConfigGADCT", "PMT ID is not defined ....");
          }
          conf->SetPID(i, val[i]);
        }
        break;
      }
      case POL_GADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPOL(i, val[0]);
          }
          else {
            conf->SetPOL(i, val[i]);
          }
        }
        break;
      }
      case DACOFF_GADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDACOFF(i, val[0]);
          }
          else {
            conf->SetDACOFF(i, val[i]);
          }
        }
        break;
      }
      case DLY_GADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDLY(i, val[0]);
          }
          else {
            conf->SetDLY(i, val[i]);
          }
        }
        break;
      }
      case DTIME_GADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDT(i, val[0]);
          }
          else {
            conf->SetDT(i, val[i]);
          }
        }
        break;
      }
      case CW_GADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetCW(i, val[0]);
          }
          else {
            conf->SetCW(i, val[i]);
          }
        }
        break;
      }
      case TM_GADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetTM(i, val[0]);
          }
          else {
            conf->SetTM(i, val[i]);
          }
        }
        break;
      }
      case THR_GADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetTHR(i, val[0]);
          }
          else {
            conf->SetTHR(i, val[i]);
          }
        }
        break;
      }
      case PCT_GADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPCT(i, val[0]);
          }
          else {
            conf->SetPCT(i, val[i]);
          }
        }
        break;
      }
      case PCI_GADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPCI(i, val[0]);
          }
          else {
            conf->SetPCI(i, val[i]);
          }
        }
        break;
      }
      case PWT_GADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPWT(i, val[0]);
          }
          else {
            conf->SetPWT(i, val[i]);
          }
        }
        break;
      }
      case PSW_GADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPSW(i, val[0]);
          }
          else {
            conf->SetPSW(i, val[i]);
          }
        }
        break;
      }
      case RL_GADCT: {
        iss >> val[0];
        conf->SetRL(val[0]);
        break;
      }
      case TLT_GADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(fTLT->GetTLT(tlt.data()));
        break;
      }
      case DSR_GADCT: {
        iss >> val[0];
        conf->SetDSR(val[0]);
        break;
      }
      default: {
        Warning("ConfigGADCT",
                Form("Configuration item %s is ambiguous.", key.data()));
        break;
      }
    }
  }

  return retVal;
}

bool RunConfig::ConfigGADCS(std::ifstream & ticket, GADCSConf * conf)
{
  bool retVal = true;

  int nch = conf->NCH();

  string line;

  while (true) {
    getline(ticket, line);

    if (line == "END") break;
    if (line.empty()) continue;

    istringstream iss(line);

    string key;
    int val[16] = {0};

    iss >> key;
    if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

    switch (fGADCSItem[key]) {
      case ENABLED_GADCS: {
        iss >> val[0];
        if (val[0] > 0) conf->SetEnable();
        break;
      }
      case CID_GADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Error("ConfigGADCS", "Channel ID is not defined ....");
            retVal = false;
            break;
          }
          conf->SetCID(i, val[i]);
        }
        break;
      }
      case PID_GADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Warning("ConfigGADCS", "PMT ID is not defined ....");
          }
          conf->SetPID(i, val[i]);
        }
        break;
      }
      case POL_GADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPOL(i, val[0]);
          }
          else {
            conf->SetPOL(i, val[i]);
          }
        }
        break;
      }
      case DACOFF_GADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDACOFF(i, val[0]);
          }
          else {
            conf->SetDACOFF(i, val[i]);
          }
        }
        break;
      }
      case DLY_GADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDLY(i, val[0]);
          }
          else {
            conf->SetDLY(i, val[i]);
          }
        }
        break;
      }
      case DTIME_GADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDT(i, val[0]);
          }
          else {
            conf->SetDT(i, val[i]);
          }
        }
        break;
      }
      case CW_GADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetCW(i, val[0]);
          }
          else {
            conf->SetCW(i, val[i]);
          }
        }
        break;
      }
      case TM_GADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetTM(i, val[0]);
          }
          else {
            conf->SetTM(i, val[i]);
          }
        }
        break;
      }
      case THR_GADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetTHR(i, val[0]);
          }
          else {
            conf->SetTHR(i, val[i]);
          }
        }
        break;
      }
      case PCT_GADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPCT(i, val[0]);
          }
          else {
            conf->SetPCT(i, val[i]);
          }
        }
        break;
      }
      case PCI_GADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPCI(i, val[0]);
          }
          else {
            conf->SetPCI(i, val[i]);
          }
        }
        break;
      }
      case PWT_GADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPWT(i, val[0]);
          }
          else {
            conf->SetPWT(i, val[i]);
          }
        }
        break;
      }
      case PSW_GADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigGADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPSW(i, val[0]);
          }
          else {
            conf->SetPSW(i, val[i]);
          }
        }
        break;
      }
      case RL_GADCS: {
        iss >> val[0];
        conf->SetRL(val[0]);
        break;
      }
      case TLT_GADCS: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(fTLT->GetTLT(tlt.data()));
        break;
      }
      case DSR_GADCS: {
        iss >> val[0];
        conf->SetDSR(val[0]);
        break;
      }
      case TRGON_GADCS: {
        iss >> val[0];
        conf->SetTRGON(val[0]);
        break;
      }
      case PTRG_GADCS: {
        iss >> val[0];
        conf->SetPTRG(val[0]);
        break;
      }
      case PSC_GADCS: {
        iss >> val[0];
        conf->SetPSC(val[0]);
        break;
      }
      default: {
        Warning("ConfigGADCS",
                Form("Configuration item %s is ambiguous.", key.data()));
        break;
      }
    }
  }

  return retVal;
}

bool RunConfig::ConfigMADCS(std::ifstream & ticket, MADCSConf * conf)
{
  bool retVal = true;

  int nch = conf->NCH();

  string line;

  while (true) {
    getline(ticket, line);

    if (line == "END") break;
    if (line.empty()) continue;

    istringstream iss(line);

    string key;
    int val[16] = {0};

    iss >> key;
    if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

    switch (fMADCSItem[key]) {
      case ENABLED_MADCS: {
        iss >> val[0];
        if (val[0] > 0) conf->SetEnable();
        break;
      }
      case CID_MADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Error("ConfigMADCS", "Channel ID is not defined ....");
            retVal = false;
            break;
          }
          conf->SetCID(i, val[i]);
        }
        break;
      }
      case PID_MADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Warning("ConfigMADCS", "PMT ID is not defined ....");
          }
          conf->SetPID(i, val[i]);
        }
        break;
      }
      case POL_MADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigMADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPOL(i, val[0]);
          }
          else {
            conf->SetPOL(i, val[i]);
          }
        }
        break;
      }
      case DACOFF_MADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigMADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDACOFF(i, val[0]);
          }
          else {
            conf->SetDACOFF(i, val[i]);
          }
        }
        break;
      }
      case DLY_MADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigMADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDLY(i, val[0]);
          }
          else {
            conf->SetDLY(i, val[i]);
          }
        }
        break;
      }
      case DTIME_MADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigMADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDT(i, val[0]);
          }
          else {
            conf->SetDT(i, val[i]);
          }
        }
        break;
      }
      case CW_MADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigMADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetCW(i, val[0]);
          }
          else {
            conf->SetCW(i, val[i]);
          }
        }
        break;
      }
      case TM_MADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigMADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetTM(i, val[0]);
          }
          else {
            conf->SetTM(i, val[i]);
          }
        }
        break;
      }
      case THR_MADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigMADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetTHR(i, val[0]);
          }
          else {
            conf->SetTHR(i, val[i]);
          }
        }
        break;
      }
      case PCT_MADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigMADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPCT(i, val[0]);
          }
          else {
            conf->SetPCT(i, val[i]);
          }
        }
        break;
      }
      case PCI_MADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigMADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPCI(i, val[0]);
          }
          else {
            conf->SetPCI(i, val[i]);
          }
        }
        break;
      }
      case PWT_MADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigMADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPWT(i, val[0]);
          }
          else {
            conf->SetPWT(i, val[i]);
          }
        }
        break;
      }
      case PSW_MADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigMADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetPSW(i, val[0]);
          }
          else {
            conf->SetPSW(i, val[i]);
          }
        }
        break;
      }
      case RL_MADCS: {
        iss >> val[0];
        conf->SetRL(val[0]);
        break;
      }
      case TLT_MADCS: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(fTLT->GetTLT(tlt.data()));
        break;
      }
      case DSR_MADCS: {
        iss >> val[0];
        conf->SetDSR(val[0]);
        break;
      }
      case TRGON_MADCS: {
        iss >> val[0];
        conf->SetTRGON(val[0]);
        break;
      }
      case PTRG_MADCS: {
        iss >> val[0];
        conf->SetPTRG(val[0]);
        break;
      }
      case PSC_MADCS: {
        iss >> val[0];
        conf->SetPSC(val[0]);
        break;
      }
      default: {
        Warning("ConfigMADCS",
                Form("Configuration item %s is ambiguous.", key.data()));
        break;
      }
    }
  }

  return retVal;
}

bool RunConfig::ConfigSADCT(std::ifstream & ticket, SADCTConf * conf)
{
  bool retVal = true;

  int nch = conf->NCH();

  string line;

  while (true) {
    getline(ticket, line);

    if (line == "END") break;
    if (line.empty()) continue;

    istringstream iss(line);

    string key;
    int val[32] = {0};

    iss >> key;
    if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

    switch (fSADCTItem[key]) {
      case ENABLED_SADCT: {
        iss >> val[0];
        if (val[0] > 0) conf->SetEnable();
        break;
      }
      case CID_SADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Error("ConfigSADCT", "Channel ID is not defined ....");
            retVal = false;
            break;
          }
          conf->SetCID(i, val[i]);
        }
        break;
      }
      case PID_SADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Error("ConfigSADCT", "PMT ID is not defined ....");
            retVal = false;
            break;
          }
          conf->SetPID(i, val[i]);
        }
        break;
      }
      case THR_SADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigSADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetTHR(i, val[0]);
          }
          else {
            conf->SetTHR(i, val[i]);
          }
        }
        break;
      }
      case DLY_SADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigSADCT",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDLY(i, val[0]);
          }
          else {
            conf->SetDLY(i, val[i]);
          }
        }
        break;
      }
      case CW_SADCT: {
        iss >> val[0];
        conf->SetCW(val[0]);
        break;
      }
      case GW_SADCT: {
        iss >> val[0];
        conf->SetGW(val[0]);
        break;
      }
      case PSW_SADCT: {
        iss >> val[0];
        conf->SetPSW(val[0]);
        break;
      }
      case SUBPED_SADCT: {
        iss >> val[0];
        conf->SetSUBPED(val[0]);
        break;
      }
      case TLT1_SADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(0, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT2_SADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(1, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT3_SADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(2, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT4_SADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(3, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT5_SADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(4, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT6_SADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(5, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT7_SADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(6, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT8_SADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(7, fTLT->GetTLT(tlt.data()));
        break;
      }
      default: {
        Warning("ConfigSADCT",
                Form("Configuration item %s is ambiguous.", key.data()));
        break;
      }
    }
  }

  return retVal;
}

bool RunConfig::ConfigSADCS(std::ifstream & ticket, SADCSConf * conf)
{
  bool retVal = true;

  int nch = conf->NCH();

  string line;

  while (true) {
    getline(ticket, line);

    if (line == "END") break;
    if (line.empty()) continue;

    istringstream iss(line);

    string key;
    int val[32] = {0};

    iss >> key;
    if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

    switch (fSADCSItem[key]) {
      case ENABLED_SADCS: {
        iss >> val[0];
        if (val[0] > 0) conf->SetEnable();
        break;
      }
      case CID_SADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Error("ConfigSADCS", "Channel ID is not defined ....");
            retVal = false;
            break;
          }
          conf->SetCID(i, val[i]);
        }
        break;
      }
      case PID_SADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Error("ConfigSADCS", "PMT ID is not defined ....");
            retVal = false;
            break;
          }
          conf->SetPID(i, val[i]);
        }
        break;
      }
      case THR_SADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigSADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetTHR(i, val[0]);
          }
          else {
            conf->SetTHR(i, val[i]);
          }
        }
        break;
      }
      case DLY_SADCS: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigSADCS",Form("%s of all channels are configured as
            // same value.", key.data()));
            conf->SetDLY(i, val[0]);
          }
          else {
            conf->SetDLY(i, val[i]);
          }
        }
        break;
      }
      case CW_SADCS: {
        iss >> val[0];
        conf->SetCW(val[0]);
        break;
      }
      case GW_SADCS: {
        iss >> val[0];
        conf->SetGW(val[0]);
        break;
      }
      case PSW_SADCS: {
        iss >> val[0];
        conf->SetPSW(val[0]);
        break;
      }
      case SUBPED_SADCS: {
        iss >> val[0];
        conf->SetSUBPED(val[0]);
        break;
      }
      case TLT1_SADCS: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(0, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT2_SADCS: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(1, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT3_SADCS: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(2, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT4_SADCS: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(3, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT5_SADCS: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(4, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT6_SADCS: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(5, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT7_SADCS: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(6, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT8_SADCS: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(7, fTLT->GetTLT(tlt.data()));
        break;
      }
      case PTRG_SADCS: {
        iss >> val[0];
        conf->SetPTRG(val[0]);
        break;
      }
      case MTHR_SADCS: {
        iss >> val[0];
        conf->SetMTHR(val[0]);
        break;
      }
      case PSC_SADCS: {
        iss >> val[0];
        conf->SetPSC(val[0]);
        break;
      }
      default: {
        Warning("ConfigSADCS",
                Form("Configuration item %s is ambiguous.", key.data()));
        break;
      }
    }
  }

  return retVal;
}

bool RunConfig::ConfigIADCT(std::ifstream & ticket, IADCTConf * conf)
{
  bool retVal = true;

  int nch = conf->NCH();

  string line;

  while (true) {
    getline(ticket, line);

    if (line == "END") break;
    if (line.empty()) continue;

    istringstream iss(line);

    string key;
    int val[40] = {0};
    float fval[5] = {0.};

    iss >> key;
    if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

    bool iscon = false;

    switch (fIADCTItem[key]) {
      case ENABLED_IADCT: {
        iss >> val[0];
        if (val[0] > 0) conf->SetEnable();
        break;
      }
      case CID_IADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail() && i == 1) {
            iscon = true;
            break;
          }
          conf->SetCID(i, val[i]);
        }
        if (iscon) {
          for (int i = 1; i < nch; i++) {
            conf->SetCID(i, ++val[0]);
          }
        }
        break;
      }
      case PID_IADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail() && i == 1) {
            iscon = true;
            break;
          }
          conf->SetPID(i, val[i]);
        }
        if (iscon) {
          for (int i = 1; i < nch; i++) {
            conf->SetPID(i, ++val[0]);
          }
        }
        break;
      }
      case THR1_IADCT: {
        for (int i = 0; i < 4; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) { conf->SetTHR(i, val[0]); }
          else {
            conf->SetTHR(i, val[i]);
          }
        }
        break;
      }
      case THR2_IADCT: {
        for (int i = 0; i < 4; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) { conf->SetTHR(4 + i, val[0]); }
          else {
            conf->SetTHR(4 + i, val[i]);
          }
        }
        break;
      }
      case THR3_IADCT: {
        for (int i = 0; i < 4; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) { conf->SetTHR(8 + i, val[0]); }
          else {
            conf->SetTHR(8 + i, val[i]);
          }
        }
        break;
      }
      case THR4_IADCT: {
        for (int i = 0; i < 4; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) { conf->SetTHR(12 + i, val[0]); }
          else {
            conf->SetTHR(12 + i, val[i]);
          }
        }
        break;
      }
      case THR5_IADCT: {
        for (int i = 0; i < 4; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) { conf->SetTHR(16 + i, val[0]); }
          else {
            conf->SetTHR(16 + i, val[i]);
          }
        }
        break;
      }
      case THR6_IADCT: {
        for (int i = 0; i < 4; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) { conf->SetTHR(20 + i, val[0]); }
          else {
            conf->SetTHR(20 + i, val[i]);
          }
        }
        break;
      }
      case THR7_IADCT: {
        for (int i = 0; i < 4; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) { conf->SetTHR(24 + i, val[0]); }
          else {
            conf->SetTHR(24 + i, val[i]);
          }
        }
        break;
      }
      case THR8_IADCT: {
        for (int i = 0; i < 4; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) { conf->SetTHR(28 + i, val[0]); }
          else {
            conf->SetTHR(28 + i, val[i]);
          }
        }
        break;
      }
      case THR9_IADCT: {
        for (int i = 0; i < 4; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) { conf->SetTHR(32 + i, val[0]); }
          else {
            conf->SetTHR(32 + i, val[i]);
          }
        }
        break;
      }
      case THR10_IADCT: {
        for (int i = 0; i < 4; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) { conf->SetTHR(36 + i, val[0]); }
          else {
            conf->SetTHR(36 + i, val[i]);
          }
        }
        break;
      }
      case DLY_IADCT: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) { conf->SetDLY(i, val[0]); }
          else {
            conf->SetDLY(i, val[i]);
          }
        }
        break;
      }
      case HV_IADCT: {
        for (int i = 0; i < 5; i++) {
          iss >> fval[i];
          if (i != 0 && iss.fail()) { conf->SetHV(i, fval[0]); }
          else {
            conf->SetHV(i, fval[i]);
          }
        }
        break;
      }
      case MODE_IADCT: {
        iss >> val[0];
        conf->SetMODE(val[0]);
        break;
      }
      case RL_IADCT: {
        iss >> val[0];
        conf->SetRL(val[0]);
        break;
      }
      case CW_IADCT: {
        iss >> val[0];
        conf->SetCW(val[0]);
        break;
      }
      case GW_IADCT: {
        iss >> val[0];
        conf->SetGW(val[0]);
        break;
      }
      case PSW_IADCT: {
        iss >> val[0];
        conf->SetPSW(val[0]);
        break;
      }
      case TLT1_IADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(0, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT2_IADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(1, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT3_IADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(2, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT4_IADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(3, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT5_IADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(4, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT6_IADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(5, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT7_IADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(6, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT8_IADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(7, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT9_IADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(8, fTLT->GetTLT(tlt.data()));
        break;
      }
      case TLT10_IADCT: {
        string tlt;
        iss >> tlt;
        conf->SetTLT(9, fTLT->GetTLT(tlt.data()));
        break;
      }
      default: {
        Warning("ConfigIADCT",
                Form("Configuration item %s is ambiguous.", key.data()));
        break;
      }
    }
  }

  return retVal;
}

bool RunConfig::ConfigAmoreADC(std::ifstream & ticket, AmoreADCConf * conf)
{
  bool retVal = true;

  int nch = conf->NCH();

  string line;

  while (true) {
    getline(ticket, line);

    if (line == "END") break;
    if (line.empty()) continue;

    istringstream iss(line);

    string key;
    int val[32] = {0};

    iss >> key;
    if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

    switch (fAMOREADCItem[key]) {
      case ENABLED_AMOREADC: {
        iss >> val[0];
        if (val[0] > 0) conf->SetEnable();
        break;
      }
      case CID_AMOREADC: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Error("ConfigAMOREADC", "Channel ID is not defined ....");
            retVal = false;
            break;
          }
          conf->SetCID(i, val[i]);
        }
        break;
      }
      case PID_AMOREADC: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (iss.fail()) {
            Error("ConfigAMOREADC", "PMT ID is not defined ....");
            retVal = false;
            break;
          }
          conf->SetPID(i, val[i]);
        }
        break;
      }
      case TRGON_AMOREADC: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigAMOREADC",Form("%s of all channels are configured
            // as same value.", key.data()));
            conf->SetTRGON(i, val[0]);
          }
          else {
            conf->SetTRGON(i, val[i]);
          }
        }
        break;
      }
      case ORDER_AMOREADC: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigAMOREADC",Form("%s of all channels are configured
            // as same value.", key.data()));
            conf->SetORDER(i, val[0]);
          }
          else {
            conf->SetORDER(i, val[i]);
          }
        }
        break;
      }
      case LOWER_AMOREADC: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigAMOREADC",Form("%s of all channels are configured
            // as same value.", key.data()));
            conf->SetLOWER(i, val[0]);
          }
          else {
            conf->SetLOWER(i, val[i]);
          }
        }
        break;
      }
      case UPPER_AMOREADC: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigAMOREADC",Form("%s of all channels are configured
            // as same value.", key.data()));
            conf->SetUPPER(i, val[0]);
          }
          else {
            conf->SetUPPER(i, val[i]);
          }
        }
        break;
      }
      case THR_AMOREADC: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigAMOREADC",Form("%s of all channels are configured
            // as same value.", key.data()));
            conf->SetTHR(i, val[0]);
          }
          else {
            conf->SetTHR(i, val[i]);
          }
        }
        break;
      }
      case DT_AMOREADC: {
        for (int i = 0; i < nch; i++) {
          iss >> val[i];
          if (i != 0 && iss.fail()) {
            // Warning("ConfigAMOREADC",Form("%s of all channels are configured
            // as same value.", key.data()));
            conf->SetDT(i, val[0]);
          }
          else {
            conf->SetDT(i, val[i]);
          }
        }
        break;
      }
      case SR_AMOREADC: {
        iss >> val[0];
        conf->SetSR(val[0]);
        break;
      }
      case RL_AMOREADC: {
        iss >> val[0];
        conf->SetRL(val[0]);
        break;
      }
      case DLY_AMOREADC: {
        iss >> val[0];
        conf->SetDLY(val[0]);
        break;
      }
      case CW_AMOREADC: {
        iss >> val[0];
        conf->SetCW(val[0]);
        break;
      }
      case SKBIN_AMOREADC: {
        iss >> val[0];
        conf->SetSKBIN(val[0]);
        break;
      }
      case ZEROSUP_AMOREADC: {
        iss >> val[0];
        conf->SetZEROSUP(val[0]);
        break;
      }
      default: {
        Warning("ConfigAMOREADC",
                Form("Configuration item %s is ambiguous.", key.data()));
        break;
      }
    }
  }

  return retVal;
}

bool RunConfig::ConfigSTRG(std::ifstream & ticket, STRGConf * conf)
{
  bool retVal = true;

  string line;

  while (true) {
    getline(ticket, line);

    if (line == "END") break;
    if (line.empty()) continue;

    istringstream iss(line);

    string key;
    int ipar;

    iss >> key;
    if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

    switch (fSTRGItem[key]) {
      case ENABLED_STRG: {
        iss >> ipar;
        if (ipar > 0) conf->SetEnable();
        break;
      }
      case ADCTYPE_STRG: {
        iss >> ipar;
        if (ipar > 0) conf->SetADCType((ADC::TYPE)ipar);
        break;
      }
      case ZSU_STRG: {
        iss >> ipar;
        if (ipar > 0) conf->SetZSUMode(ipar);
        break;
      }
      case PSC_STRG: {
        iss >> ipar;
        conf->SetPrescale(ipar);
        break;
      }
      case ICRD_STRG: {
        string spar;
        iss >> spar;
        conf->SetInputCard(spar.c_str());
        break;
      }
      default: {
        Warning("ConfigSTRG",
                Form("Configuration item %s is ambiguous.", key.data()));
        break;
      }
    }
  }

  return retVal;
}

bool RunConfig::ConfigDAQ(std::ifstream & ticket, DAQConf * conf)
{
  bool retVal = true;

  string line;

  while (true) {
    getline(ticket, line);

    if (line == "END") break;
    if (line.empty()) continue;

    istringstream iss(line);

    string key;
    int id, port;
    string name, addr;

    iss >> key;
    if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

    switch (fDAQItem[key]) {
      case SERVER: {
        iss >> id >> name >> addr >> port;
        conf->AddDAQ(id, name, addr, port);
        break;
      }
      default: {
        Warning("ConfigDAQ",
                Form("Configuration item %s is ambiguous.", key.data()));
        break;
      }
    }
  }

  return retVal;
}

/**
$Log: RunConfig.cc,v $
Revision 1.33  2023/07/18 12:25:11  cupsoft
*** empty log message ***

Revision 1.32  2023/03/23 04:32:42  cupsoft
*** empty log message ***

Revision 1.31  2023/02/08 02:04:34  cupsoft
*** empty log message ***

Revision 1.30  2023/01/12 03:46:12  cupsoft
add DAQconfig

Revision 1.29  2023/01/06 05:25:10  cupsoft
*** empty log message ***

Revision 1.28  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.28  2020/07/10 02:38:36  cupsoft
*** empty log message ***

Revision 1.27  2020/01/09 04:57:26  cupsoft
*** empty log message ***

Revision 1.26  2019/11/25 02:04:07  cupsoft
*** empty log message ***

Revision 1.25  2019/07/24 07:04:23  cupsoft
*** empty log message ***

Revision 1.24  2019/07/12 07:18:42  cupsoft
*** empty log message ***

Revision 1.23  2019/07/11 08:22:11  cupsoft
*** empty log message ***

Revision 1.22  2019/07/09 07:39:06  cupsoft
*** empty log message ***

Revision 1.21  2019/07/05 00:11:07  cupsoft
*** empty log message ***

Revision 1.20  2019/05/30 23:59:05  cupsoft
*** empty log message ***

Revision 1.19  2018/11/01 01:40:44  cupsoft
*** empty log message ***

Revision 1.18  2018/09/20 01:01:10  cupsoft
*** empty log message ***

Revision 1.17  2018/04/30 05:42:41  cupsoft
*** empty log message ***

Revision 1.16  2018/02/21 02:32:37  cupsoft
*** empty log message ***

Revision 1.15  2018/01/09 04:52:05  cupsoft
*** empty log message ***

Revision 1.14  2018/01/08 05:14:45  cupsoft
*** empty log message ***

Revision 1.13  2017/06/12 02:12:22  cupsoft
*** empty log message ***

Revision 1.12  2017/05/19 01:58:58  cupsoft
*** empty log message ***

Revision 1.11  2017/04/06 23:52:37  cupsoft
*** empty log message ***

Revision 1.10  2017/04/06 12:11:57  cupsoft
*** empty log message ***

Revision 1.9  2017/03/08 05:21:44  cupsoft
*** empty log message ***

Revision 1.8  2017/01/25 02:14:18  cupsoft
*** empty log message ***

Revision 1.7  2016/12/14 04:54:27  cupsoft
*** empty log message ***

Revision 1.6  2016/12/05 09:52:51  cupsoft
*** empty log message ***

Revision 1.5  2016/11/12 04:05:28  cupsoft
*** empty log message ***

Revision 1.4  2016/10/02 10:55:13  cupsoft
*** empty log message ***

Revision 1.3  2016/10/03 06:57:04  cupsoft
*** empty log message ***

Revision 1.2  2016/10/01 17:10:54  cupsoft
clean up

Revision 1.1.1.1  2016/09/30 09:22:24  cupsoft
DAQConfig

Revision 1.4  2016/09/28 16:54:49  cupsoft
*** empty log message ***

Revision 1.3  2016/09/27 03:41:55  cupsoft
*** empty log message ***

Revision 1.2  2016/09/05 05:26:51  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/07/14 07:42:30  cupsoft
DAQSys

Revision 1.2  2016/05/10 01:57:35  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/03/08 04:37:46  amore
DAQSts

Revision 1.1.1.1  2016/02/29 08:26:44  cupsoft
DAQSys

**/
