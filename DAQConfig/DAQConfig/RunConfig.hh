#pragma once

#include <string>
#include <yaml-cpp/yaml.h>

#include "DAQConfig/AbsConfList.hh"
#include "DAQConfig/DAQConf.hh"
#include "DAQConfig/FADCSConf.hh"
#include "DAQConfig/FADCTConf.hh"
#include "DAQConfig/GADCSConf.hh"
#include "DAQConfig/IADCTConf.hh"
#include "DAQConfig/MADCSConf.hh"
#include "DAQConfig/SADCTConf.hh"
#include "DAQConfig/TCBConf.hh"

class RunConfig {
public:
  RunConfig();
  virtual ~RunConfig() = default;

  void SetConfigFilename(const char * name);
  AbsConfList * GetConfigs() const;

  virtual bool ReadConfig();
  virtual bool ReadConfig(const char * name);

private:
  YAML::Node MergeNodes(YAML::Node target, YAML::Node source);
  template <typename T>
  void FillConfigArray(YAML::Node node, int nch, std::function<void(int, T)> setter,
                       bool inc = false);
  void ConfigDAQ(YAML::Node ymlnode);
  void ConfigTCB(YAML::Node ymlnode);
  void ConfigFADCT(YAML::Node ymlnode);
  void ConfigIADCT(YAML::Node ymlnode);
  void ConfigSADCT(YAML::Node ymlnode);
  void ConfigFADCS(YAML::Node ymlnode);
  void ConfigGADCS(YAML::Node ymlnode);
  void ConfigMADCS(YAML::Node ymlnode);

protected:
  std::string fConfigFilename;
  std::unique_ptr<AbsConfList> fConfigs;
};

inline void RunConfig::SetConfigFilename(const char * name) { fConfigFilename = name; }
inline AbsConfList * RunConfig::GetConfigs() const { return fConfigs.get(); }
