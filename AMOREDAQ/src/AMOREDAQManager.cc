#include <fstream>
#include <limits>    // Required for std::numeric_limits
#include <stdexcept> // For std::stoi exceptions
#include <string>
#include <thread>

#include "AMOREDAQ/AMOREDAQManager.hh"
#include "AMORESystem/AMOREADC.hh"
#include "AMORESystem/AMOREADCConf.hh"
#include "AMORESystem/AMORETCBConf.hh"
#include "DAQConfig/DAQConf.hh"
#include "DAQUtils/ELog.hh"
#include "OnlConsts/adcconsts.hh"

ClassImp(AMOREDAQManager)

AMOREDAQManager::AMOREDAQManager()
  : CupDAQManager(),
    fTCB(AMORETCB::Instance())
{
  fADCType = ADC::AMOREADC; // only AMOREADC will be added, this is amoredaq
  fConfigList = new AbsConfList();
}

AMOREDAQManager::~AMOREDAQManager() { delete fConfigList; }

bool AMOREDAQManager::AddADC(AbsConfList * conflist)
{
  int nadc = conflist->GetNADC(fADCType);
  if (nadc == 0) {
    ERROR("there is no AMOREADC");
    return false;
  }

  for (int i = 0; i < nadc; i++) {
    AbsConf * conf = conflist->GetConfig(fADCType, i);
    if (!conf->IsEnabled()) continue;
    if (!conf->IsLinked()) {
      ERROR("AMOREADC[sid=%2d] enabled but not linked", conf->SID());
      return false;
    }
    if (conf->GetDAQID() == fDAQID) {
      auto * adc = new AMOREADC(conf);
      Add(adc);
      INFO("AMOREADC[sid=%2d] added to DAQ manager", adc->GetSID());
    }
  }

  return true;
}

bool AMOREDAQManager::ReadConfig()
{
  if (fConfigFilename.IsNull()) {
    ERROR("config filename is empty");
    return false;
  }

  std::string filename(fConfigFilename.Data());

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

          ReadConfigTCB(inc_node);
          ReadConfigADC(inc_node);

          INFO("Included config %s is successfully loaded", inc_file.c_str());
        }
        catch (const std::exception & e) {
          ERROR("Failed to load included file %s: %s", inc_file.c_str(), e.what());
          return false;
        }
      }
    }

    ReadConfigTCB(node);
    ReadConfigADC(node);

    INFO("reading config %s is done", filename.c_str());

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
void AMOREDAQManager::FillConfigArray(YAML::Node node, int nch, std::function<void(int, T)> setter,
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

void AMOREDAQManager::ReadConfigTCB(YAML::Node ymlnode)
{
  if (!ymlnode["AMORETCB"]) return;

  auto * conf = new AMORETCBConf(0);
  auto tcb = ymlnode["AMORETCB"];

  if (tcb["ID"]) conf->SetDAQID(tcb["ID"].as<int>());
  if (tcb["CW"]) conf->SetCW(tcb["CW"].as<int>());
  if (tcb["DT"]) conf->SetDT(tcb["DT"].as<int>());
  if (tcb["PSC"]) conf->SetPSC(tcb["PSC"].as<int>());

  fConfigList->Add(conf);
}

void AMOREDAQManager::ReadConfigADC(YAML::Node ymlnode)
{
  if (!ymlnode["AMOREADC"]) return;

  std::vector<YAML::Node> nodes;
  if (ymlnode["AMOREADC"].IsSequence()) {
    for (const auto & n : ymlnode["AMOREADC"])
      nodes.push_back(n);
  }
  else {
    nodes.push_back(ymlnode["AMOREADC"]);
  }

  for (auto & node : nodes) {
    int nch = 0;

    auto * conf = new AMOREADCConf();
    conf->SetName("AMOREADC");
    conf->SetADCType(ADC::AMOREADC);

    if (node["ENABLED"] && node["ENABLED"].as<int>()) { conf->SetEnable(); }

    if (node["DAQID"]) conf->SetDAQID(node["DAQID"].as<int>());

    if (node["SID"]) {
      int sid = node["SID"].as<int>();
      conf->SetSID(sid);
      conf->SetMID(sid+128);
    }
    if (node["NCH"]) {
      nch = node["NCH"].as<int>();
      conf->SetNCH(nch);
    }

    if (node["SR"]) conf->SetSR(node["SR"].as<int>());
    if (node["RL"]) conf->SetRL(node["RL"].as<int>());
    if (node["DLY"]) conf->SetDLY(node["DLY"].as<int>());
    if (node["ZSU"]) conf->SetZSU(node["ZSU"].as<int>());

    if (nch > 0) {
      FillConfigArray<int>(node["CID"], nch, [&](int i, int v) { conf->SetCID(i, v); }, true);
      FillConfigArray<int>(node["PID"], nch, [&](int i, int v) { conf->SetPID(i, v); }, true);
      FillConfigArray<int>(node["TRGON"], nch, [&](int i, int v) { conf->SetTRGON(i, v); });
    }

    fConfigList->Add(conf);
  }
}

bool AMOREDAQManager::PrepareDAQ()
{
  const int nadc = GetEntries();

  if (nadc <= 0) {
    ERROR("No ADC module included in the configuration");
    fReadStatus = ERROR;
    RUNSTATE::SetError(fRunStatus);
    return false;
  }

  fFIFOs.clear();
  int dsr = 0;
  for (int i = 0; i < nadc; ++i) {
    auto * adc = static_cast<AbsADC *>(fCont[i]);
    auto * conf = static_cast<AMOREADCConf *>(adc->GetConfig());
    dsr = conf->SR();

    int head = conf->DLY();
    int tail = conf->RL() - head;
    fFIFOs.push_back(std::make_unique<AMOREChunkFIFO>(kNCHAMOREADC, head, tail));
  }

  Sort();

  fTimeDelta = dsr * 1000;


  fRemainingBCount = new int[nadc];
  for (int i = 0; i < nadc; i++) {
    fRemainingBCount[i] = 0;
  }

  INFO("prepared to take data from AMOREADC");

  return true;
}
