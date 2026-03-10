#include "DAQConfig/RunConfig.hh"
#include "DAQConfig/TriggerLookupTable.hh"
#include "DAQUtils/ELog.hh"

RunConfig::RunConfig() { fConfigs = new AbsConfList(); }

RunConfig::~RunConfig() { delete fConfigs; }

bool RunConfig::ReadConfig() { return ReadConfig(fConfigFilename.c_str()); }

bool RunConfig::ReadConfig(const char * name)
{
  std::string filename(name);

  if (filename.empty()) {
    ERROR("config filename is empty");
    return false;
  }

  try {
    YAML::Node node = YAML::LoadFile(filename.c_str());
    if (node.IsNull()) {
      ERROR("config file is empty");
      return false;
    }

    if (node["Include"] && node["Include"].IsSequence()) {
      for (const auto & inc : node["Include"]) {
        std::string inc_file = inc.as<std::string>();
        try {
          YAML::Node inc_node = YAML::LoadFile(inc_file.c_str());

          ConfigDAQ(inc_node);
          ConfigTCB(inc_node);
          ConfigFADCT(inc_node);
          ConfigIADCT(inc_node);
          ConfigSADCT(inc_node);
          ConfigFADCS(inc_node);
          ConfigGADCS(inc_node);
          ConfigMADCS(inc_node);

          INFO("Included config %s is successfully loaded", inc_file.c_str());
        }
        catch (const std::exception & e) {
          ERROR("Failed to load included file %s: %s", inc_file.c_str(), e.what());
          return false;
        }
      }
    }

    ConfigDAQ(node);
    ConfigTCB(node);
    ConfigFADCT(node);
    ConfigIADCT(node);
    ConfigSADCT(node);
    ConfigFADCS(node);
    ConfigGADCS(node);
    ConfigMADCS(node);

    INFO("reading config %s is done", name);

    return true;
  }
  catch (const YAML::BadFile & e) {
    ERROR("file not found, %s", filename.c_str());
  }
  catch (const YAML::ParserException & e) {
    ERROR("syntax error (%s) at line %d, col %d of config file", e.msg.c_str(), e.mark.line + 1,
          e.mark.column + 1);
  }
  catch (const std::exception & e) {
    const char * err_msg = e.what();
    ERROR("unknown error(%s) on reading config file", err_msg ? err_msg : "Unknown");
  }

  return false;
}

template <typename T>
void RunConfig::FillConfigArray(YAML::Node node, int nch, std::function<void(int, T)> setter,
                                bool inc)
{
  if (!node) return;

  std::vector<T> val;
  if (node.IsScalar()) { val.push_back(node.as<T>()); }
  else {
    try {
      val = node.as<std::vector<T>>();
    }
    catch (...) {
      return;
    }
  }

  int valsize = val.size();
  if (valsize == 0) return;

  for (int i = 0; i < nch; ++i) {
    T target;
    if (i < valsize) { target = val[i]; }
    else {
      if (inc) { target = val[valsize - 1] + (i - (valsize - 1)); }
      else {
        target = val[valsize - 1];
      }
    }
    setter(i, target);
  }
}

void RunConfig::ConfigDAQ(YAML::Node ymlnode)
{
  if (!ymlnode["DAQ"]) return;

  auto * conf = new DAQConf();
  YAML::Node daq_list = ymlnode["DAQ"];

  for (const auto & server : daq_list) {
    try {
      int id = server["ID"].as<int>();
      std::string name = server["NAME"].as<std::string>();
      std::string ip = server["IP"].as<std::string>();
      int port = server["PORT"].as<int>();

      conf->AddDAQ(id, name, ip, port);
    }
    catch (const YAML::Exception & e) {
      WARNING("Failed to parse DAQ server entry: %s", e.what());
    }
  }

  fConfigs->Add(conf);
}

void RunConfig::ConfigTCB(YAML::Node ymlnode)
{
  if (!ymlnode["TCB"]) return;

  auto * conf = new TCBConf(0);
  auto tcb = ymlnode["TCB"];

  auto setSafeSW = [](YAML::Node node, std::function<void(int, int, int, int)> setter) {
    if (!node) return;
    std::vector<int> sw = node.as<std::vector<int>>();
    int s0 = (sw.size() > 0) ? sw[0] : 0;
    int s1 = (sw.size() > 1) ? sw[1] : 0;
    int s2 = (sw.size() > 2) ? sw[2] : 0;
    int s3 = (sw.size() > 3) ? sw[3] : 0;
    setter(s0, s1, s2, s3);
  };

  if (tcb["DAQID"]) conf->SetDAQID(tcb["DAQID"].as<int>());
  if (tcb["TYPE"]) conf->SetTCBTYPE(static_cast<TCB::TYPE>(tcb["TYPE"].as<int>()));
  if (tcb["TM"]) conf->SetTM(tcb["TM"].as<int>());
  if (tcb["CW"]) conf->SetCW(tcb["CW"].as<int>());
  if (tcb["DLY"]) conf->SetDLY(tcb["DLY"].as<int>());
  if (tcb["PTRG"]) conf->SetPTRG(tcb["PTRG"].as<int>());

  if (tcb["MTHRF"]) conf->SetMTHRF(tcb["MTHRF"].as<int>());
  if (tcb["PSCF"]) conf->SetPSCF(tcb["PSCF"].as<int>());
  if (tcb["DTF"]) conf->SetDTF(tcb["DTF"].as<int>());
  setSafeSW(tcb["SWF"], [&](int a, int b, int c, int d) { conf->SetSWF(a, b, c, d); });

  if (tcb["MTHRSM"]) conf->SetMTHRSM(tcb["MTHRSM"].as<int>());
  if (tcb["PSCSM"]) conf->SetPSCSM(tcb["PSCSM"].as<int>());
  if (tcb["DTSM"]) conf->SetDTSM(tcb["DTSM"].as<int>());
  setSafeSW(tcb["SWSM"], [&](int a, int b, int c, int d) { conf->SetSWSM(a, b, c, d); });

  if (tcb["MTHRSL"]) conf->SetMTHRSL(tcb["MTHRSL"].as<int>());
  if (tcb["PSCSL"]) conf->SetPSCSL(tcb["PSCSL"].as<int>());
  if (tcb["DTSL"]) conf->SetDTSL(tcb["DTSL"].as<int>());
  setSafeSW(tcb["SWSL"], [&](int a, int b, int c, int d) { conf->SetSWSL(a, b, c, d); });

  if (tcb["MTHRI"]) conf->SetMTHRI(tcb["MTHRI"].as<int>());
  if (tcb["PSCI"]) conf->SetPSCI(tcb["PSCI"].as<int>());
  if (tcb["DTI"]) conf->SetDTI(tcb["DTI"].as<int>());
  setSafeSW(tcb["SWI"], [&](int a, int b, int c, int d) { conf->SetSWI(a, b, c, d); });

  fConfigs->Add(conf);
}

void RunConfig::ConfigFADCT(YAML::Node ymlnode)
{
  if (!ymlnode["FADCT"]) return;

  std::vector<YAML::Node> nodes;
  if (ymlnode["FADCT"].IsSequence()) {
    for (const auto & n : ymlnode["FADCT"])
      nodes.push_back(n);
  }
  else {
    nodes.push_back(ymlnode["FADCT"]);
  }

  for (auto & node : nodes) {
    int nch = 0;

    FADCTConf * conf = new FADCTConf();

    if (node["ENABLED"] && node["ENABLED"].as<int>()) { conf->SetEnable(); }
    if (node["DAQID"]) conf->SetDAQID(node["DAQID"].as<int>());

    if (node["SID"]) {
      int sid = node["SID"].as<int>();
      conf->SetSID(sid);
      conf->SetMID(sid);
    }
    if (node["NCH"]) {
      nch = node["NCH"].as<int>();
      conf->SetNCH(nch);
    }

    // TLT String Parsing
    if (node["TLT"]) {
      std::string tlt_str = node["TLT"].as<std::string>();
      unsigned short tlt_val = TriggerLookupTable::Instance().GetTLT(tlt_str.c_str());
      conf->SetTLT(tlt_val);
    }

    if (node["DSR"]) conf->SetDSR(node["DSR"].as<int>());

    if (nch > 0) {
      FillConfigArray<int>(node["CID"], nch, [&](int i, int v) { conf->SetCID(i, v); }, true);
      FillConfigArray<int>(node["PID"], nch, [&](int i, int v) { conf->SetPID(i, v); }, true);
      FillConfigArray<int>(node["POL"], nch, [&](int i, int v) { conf->SetPOL(i, v); });
      FillConfigArray<int>(node["DACOFF"], nch, [&](int i, int v) { conf->SetDACOFF(i, v); });
      FillConfigArray<int>(node["AMD"], nch, [&](int i, int v) { conf->SetAMD(i, v); });
      FillConfigArray<int>(node["DLY"], nch, [&](int i, int v) { conf->SetDLY(i, v); });
      FillConfigArray<int>(node["DT"], nch, [&](int i, int v) { conf->SetDT(i, v); });
      FillConfigArray<int>(node["CW"], nch, [&](int i, int v) { conf->SetCW(i, v); });
      FillConfigArray<int>(node["TM"], nch, [&](int i, int v) { conf->SetTM(i, v); });
      FillConfigArray<int>(node["THR"], nch, [&](int i, int v) { conf->SetTHR(i, v); });
      FillConfigArray<int>(node["PCT"], nch, [&](int i, int v) { conf->SetPCT(i, v); });
      FillConfigArray<int>(node["PCI"], nch, [&](int i, int v) { conf->SetPCI(i, v); });
      FillConfigArray<int>(node["PWT"], nch, [&](int i, int v) { conf->SetPWT(i, v); });
      FillConfigArray<int>(node["PSW"], nch, [&](int i, int v) { conf->SetPSW(i, v); });
    }
    fConfigs->Add(conf);
  }
}

void RunConfig::ConfigIADCT(YAML::Node ymlnode)
{
  if (!ymlnode["IADCT"]) return;

  std::vector<YAML::Node> nodes;
  if (ymlnode["IADCT"].IsSequence()) {
    for (const auto & n : ymlnode["IADCT"])
      nodes.push_back(n);
  }
  else {
    nodes.push_back(ymlnode["IADCT"]);
  }

  for (auto & node : nodes) {
    int nch = 0;

    IADCTConf * conf = new IADCTConf();

    if (node["ENABLED"] && node["ENABLED"].as<int>()) { conf->SetEnable(); }
    if (node["DAQID"]) conf->SetDAQID(node["DAQID"].as<int>());

    if (node["SID"]) {
      int sid = node["SID"].as<int>();
      conf->SetSID(sid);
      conf->SetMID(sid + 192);
    }
    if (node["NCH"]) {
      nch = node["NCH"].as<int>();
      conf->SetNCH(nch);
    }

    if (node["MODE"]) conf->SetMODE(node["MODE"].as<int>());
    if (node["RL"]) conf->SetRL(node["RL"].as<int>());
    if (node["CW"]) conf->SetCW(node["CW"].as<int>());
    if (node["GW"]) conf->SetGW(node["GW"].as<int>());
    if (node["PSW"]) conf->SetPSW(node["PSW"].as<int>());

    if (node["TLT"]) {
      if (node["TLT"].IsSequence()) {
        std::vector<std::string> tlt_strs = node["TLT"].as<std::vector<std::string>>();
        int valsize = tlt_strs.size();
        for (int i = 0; i < 10; ++i) {
          std::string target_s = (i < valsize) ? tlt_strs[i] : tlt_strs[valsize - 1];
          unsigned short tlt_val = TriggerLookupTable::Instance().GetTLT(target_s.c_str());
          conf->SetTLT(i, tlt_val);
        }
      }
      else {
        std::string tlt_s = node["TLT"].as<std::string>();
        unsigned short tlt_val = TriggerLookupTable::Instance().GetTLT(tlt_s.c_str());
        for (int i = 0; i < 10; ++i)
          conf->SetTLT(i, tlt_val);
      }
    }

    if (node["HV"]) {
      FillConfigArray<float>(node["HV"], 5, [&](int group, float v) { conf->SetHV(group, v); });
    }

    if (nch > 0) {
      FillConfigArray<int>(node["CID"], nch, [&](int i, int v) { conf->SetCID(i, v); }, true);
      FillConfigArray<int>(node["PID"], nch, [&](int i, int v) { conf->SetPID(i, v); }, true);
      FillConfigArray<int>(node["THR"], nch, [&](int i, int v) { conf->SetTHR(i, v); });
      FillConfigArray<int>(node["DLY"], nch, [&](int i, int v) { conf->SetDLY(i, v); });
    }

    fConfigs->Add(conf);
  }
}

void RunConfig::ConfigSADCT(YAML::Node ymlnode)
{
  if (!ymlnode["SADCT"]) return;

  std::vector<YAML::Node> nodes;
  if (ymlnode["SADCT"].IsSequence()) {
    for (const auto & n : ymlnode["SADCT"])
      nodes.push_back(n);
  }
  else {
    nodes.push_back(ymlnode["SADCT"]);
  }

  for (auto & node : nodes) {
    int nch = 0;

    SADCTConf * conf = new SADCTConf();

    if (node["ENABLED"] && node["ENABLED"].as<int>()) { conf->SetEnable(); }
    if (node["DAQID"]) conf->SetDAQID(node["DAQID"].as<int>());

    if (node["SID"]) {
      int sid = node["SID"].as<int>();
      conf->SetSID(sid);
      conf->SetMID(sid + 64);
    }
    if (node["NCH"]) {
      nch = node["NCH"].as<int>();
      conf->SetNCH(nch);
    }
    if (node["CW"]) conf->SetCW(node["CW"].as<int>());
    if (node["GW"]) conf->SetGW(node["GW"].as<int>());
    if (node["PSW"]) conf->SetPSW(node["PSW"].as<int>());
    if (node["SUBPED"]) conf->SetSUBPED(node["SUBPED"].as<int>());

    if (node["TLT"]) {
      if (node["TLT"].IsSequence()) {
        std::vector<std::string> tlt_strs = node["TLT"].as<std::vector<std::string>>();
        int valsize = tlt_strs.size();
        for (int i = 0; i < 8; ++i) {
          std::string target_s = (i < valsize) ? tlt_strs[i] : tlt_strs[valsize - 1];
          unsigned short tlt_val = TriggerLookupTable::Instance().GetTLT(target_s.c_str());
          conf->SetTLT(i, tlt_val);
        }
      }
      else if (node["TLT"].IsScalar()) {
        std::string tlt_s = node["TLT"].as<std::string>();
        unsigned short tlt_val = TriggerLookupTable::Instance().GetTLT(tlt_s.c_str());
        for (int i = 0; i < 8; ++i)
          conf->SetTLT(i, tlt_val);
      }
    }
    if (nch > 0) {
      FillConfigArray<int>(node["CID"], nch, [&](int i, int v) { conf->SetCID(i, v); }, true);
      FillConfigArray<int>(node["PID"], nch, [&](int i, int v) { conf->SetPID(i, v); }, true);
      FillConfigArray<int>(node["THR"], nch, [&](int i, int v) { conf->SetTHR(i, v); });
      FillConfigArray<int>(node["DLY"], nch, [&](int i, int v) { conf->SetDLY(i, v); });
    }

    fConfigs->Add(conf);
  }
}

void RunConfig::ConfigFADCS(YAML::Node ymlnode)
{
  YAML::Node node = ymlnode["FADCS"];
  if (!node) return;

  int nch = 0;

  FADCSConf * conf = new FADCSConf();

  if (node["ENABLED"] && node["ENABLED"].as<int>()) { conf->SetEnable(); }

  if (node["SID"]) {
    conf->SetSID(node["SID"].as<int>());
    conf->SetMID(node["SID"].as<int>());
  }

  if (node["NCH"]) {
    nch = node["NCH"].as<int>();
    conf->SetNCH(nch);
  }

  if (node["RL"]) conf->SetRL(node["RL"].as<int>());
  if (node["DSR"]) conf->SetDSR(node["DSR"].as<int>());
  
  if (node["TRGON"]) conf->SetTRGON(node["TRGON"].as<int>());
  if (node["PTRG"]) conf->SetPTRG(node["PTRG"].as<int>());
  if (node["PSC"]) conf->SetPSC(node["PSC"].as<int>());

  if (node["TLT"]) {
    std::string tlt_str = node["TLT"].as<std::string>();
    unsigned short tlt_val = TriggerLookupTable::Instance().GetTLT(tlt_str.c_str());
    conf->SetTLT(tlt_val);
  }

  if (nch > 0) {
    FillConfigArray<int>(node["CID"], nch, [&](int i, int v) { conf->SetCID(i, v); }, true);
    FillConfigArray<int>(node["PID"], nch, [&](int i, int v) { conf->SetPID(i, v); }, true);
    FillConfigArray<int>(node["POL"], nch, [&](int i, int v) { conf->SetPOL(i, v); });
    FillConfigArray<int>(node["DACOFF"], nch, [&](int i, int v) { conf->SetDACOFF(i, v); });
    FillConfigArray<int>(node["AMD"], nch, [&](int i, int v) { conf->SetAMD(i, v); });
    FillConfigArray<int>(node["DLY"], nch, [&](int i, int v) { conf->SetDLY(i, v); });
    FillConfigArray<int>(node["DT"], nch, [&](int i, int v) { conf->SetDT(i, v); });
    FillConfigArray<int>(node["CW"], nch, [&](int i, int v) { conf->SetCW(i, v); });
    FillConfigArray<int>(node["TM"], nch, [&](int i, int v) { conf->SetTM(i, v); });
    FillConfigArray<int>(node["THR"], nch, [&](int i, int v) { conf->SetTHR(i, v); });
    FillConfigArray<int>(node["PCT"], nch, [&](int i, int v) { conf->SetPCT(i, v); });
    FillConfigArray<int>(node["PCI"], nch, [&](int i, int v) { conf->SetPCI(i, v); });
    FillConfigArray<int>(node["PWT"], nch, [&](int i, int v) { conf->SetPWT(i, v); });
    FillConfigArray<int>(node["PSW"], nch, [&](int i, int v) { conf->SetPSW(i, v); });
  }
  fConfigs->Add(conf);
}

void RunConfig::ConfigGADCS(YAML::Node ymlnode)
{
  YAML::Node node = ymlnode["GADCS"];
  if (!node) return;

  int nch = 0;

  GADCSConf * conf = new GADCSConf();

  if (node["ENABLED"] && node["ENABLED"].as<int>()) { conf->SetEnable(); }

  if (node["SID"]) {
    conf->SetSID(node["SID"].as<int>());
    conf->SetMID(node["SID"].as<int>());
  }

  if (node["NCH"]) {
    nch = node["NCH"].as<int>();
    conf->SetNCH(nch);
  }
  if (node["RL"]) conf->SetRL(node["RL"].as<int>());
  if (node["DSR"]) conf->SetDSR(node["DSR"].as<int>());

  if (node["TRGON"]) conf->SetTRGON(node["TRGON"].as<int>());
  if (node["PTRG"]) conf->SetPTRG(node["PTRG"].as<int>());
  if (node["PSC"]) conf->SetPSC(node["PSC"].as<int>());

  if (node["TLT"]) {
    std::string tlt_str = node["TLT"].as<std::string>();
    unsigned short tlt_val = TriggerLookupTable::Instance().GetTLT(tlt_str.c_str());
    conf->SetTLT(tlt_val);
  }

  if (nch > 0) {
    FillConfigArray<int>(node["CID"], nch, [&](int i, int v) { conf->SetCID(i, v); }, true);
    FillConfigArray<int>(node["PID"], nch, [&](int i, int v) { conf->SetPID(i, v); }, true);
    FillConfigArray<int>(node["POL"], nch, [&](int i, int v) { conf->SetPOL(i, v); });
    FillConfigArray<int>(node["DACOFF"], nch, [&](int i, int v) { conf->SetDACOFF(i, v); });
    FillConfigArray<int>(node["AMD"], nch, [&](int i, int v) { conf->SetAMD(i, v); });
    FillConfigArray<int>(node["DLY"], nch, [&](int i, int v) { conf->SetDLY(i, v); });
    FillConfigArray<int>(node["DT"], nch, [&](int i, int v) { conf->SetDT(i, v); });
    FillConfigArray<int>(node["CW"], nch, [&](int i, int v) { conf->SetCW(i, v); });
    FillConfigArray<int>(node["TM"], nch, [&](int i, int v) { conf->SetTM(i, v); });
    FillConfigArray<int>(node["THR"], nch, [&](int i, int v) { conf->SetTHR(i, v); });
    FillConfigArray<int>(node["PCT"], nch, [&](int i, int v) { conf->SetPCT(i, v); });
    FillConfigArray<int>(node["PCI"], nch, [&](int i, int v) { conf->SetPCI(i, v); });
    FillConfigArray<int>(node["PWT"], nch, [&](int i, int v) { conf->SetPWT(i, v); });
    FillConfigArray<int>(node["PSW"], nch, [&](int i, int v) { conf->SetPSW(i, v); });
  }
  fConfigs->Add(conf);
}

void RunConfig::ConfigMADCS(YAML::Node ymlnode)
{
  YAML::Node node = ymlnode["MADCS"];
  if (!node) return;

  int nch = 0;

  auto * conf = new MADCSConf();

  if (node["ENABLED"] && node["ENABLED"].as<int>()) { conf->SetEnable(); }

  if (node["SID"]) {
    conf->SetSID(node["SID"].as<int>());
    conf->SetMID(node["SID"].as<int>());
  }

  if (node["NCH"]) {
    nch = node["NCH"].as<int>();
    conf->SetNCH(nch);
  }

  if (node["RL"]) conf->SetRL(node["RL"].as<int>());
  if (node["DSR"]) conf->SetDSR(node["DSR"].as<int>());

  if (node["TRGON"]) conf->SetTRGON(node["TRGON"].as<int>());
  if (node["PTRG"]) conf->SetPTRG(node["PTRG"].as<int>());
  if (node["PSC"]) conf->SetPSC(node["PSC"].as<int>());

  if (node["TLT"]) {
    std::string tlt_str = node["TLT"].as<std::string>();
    unsigned short tlt_val = TriggerLookupTable::Instance().GetTLT(tlt_str.c_str());
    conf->SetTLT(tlt_val);
  }

  if (nch > 0) {
    FillConfigArray<int>(node["CID"], nch, [&](int i, int v) { conf->SetCID(i, v); }, true);
    FillConfigArray<int>(node["PID"], nch, [&](int i, int v) { conf->SetPID(i, v); }, true);
    FillConfigArray<int>(node["POL"], nch, [&](int i, int v) { conf->SetPOL(i, v); });
    FillConfigArray<int>(node["DACOFF"], nch, [&](int i, int v) { conf->SetDACOFF(i, v); });
    FillConfigArray<int>(node["AMD"], nch, [&](int i, int v) { conf->SetAMD(i, v); });
    FillConfigArray<int>(node["DLY"], nch, [&](int i, int v) { conf->SetDLY(i, v); });
    FillConfigArray<int>(node["DT"], nch, [&](int i, int v) { conf->SetDT(i, v); });
    FillConfigArray<int>(node["CW"], nch, [&](int i, int v) { conf->SetCW(i, v); });
    FillConfigArray<int>(node["TM"], nch, [&](int i, int v) { conf->SetTM(i, v); });
    FillConfigArray<int>(node["THR"], nch, [&](int i, int v) { conf->SetTHR(i, v); });
    FillConfigArray<int>(node["PCT"], nch, [&](int i, int v) { conf->SetPCT(i, v); });
    FillConfigArray<int>(node["PCI"], nch, [&](int i, int v) { conf->SetPCI(i, v); });
    FillConfigArray<int>(node["PWT"], nch, [&](int i, int v) { conf->SetPWT(i, v); });
    FillConfigArray<int>(node["PSW"], nch, [&](int i, int v) { conf->SetPSW(i, v); });
  }
  fConfigs->Add(conf);
}