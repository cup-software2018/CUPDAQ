#pragma once

#include <yaml-cpp/yaml.h>

#include "TObjArray.h"
#include "TString.h"

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class AbsConfList : public TObjArray {
public:
  AbsConfList();
  ~AbsConfList() override = default;

  int GetNADC(ADC::TYPE type, bool isalive = false) const;

  AbsConf * FindConfig(int mid) const;
  AbsConf * FindConfig(const char * name, int mid) const;
  AbsConf * FindConfig(ADC::TYPE type, int mid) const;
  AbsConf * GetConfig(ADC::TYPE type, int n) const;

  AbsConf * GetTCBConfig() const;
  AbsConf * GetSTRGConfig(ADC::TYPE type) const;
  AbsConf * GetDAQConfig() const;

  void SetYAMLString(const TString & yaml);
  const TString & GetYAMLString() const;
  YAML::Node GetYAMLNode() const;
  YAML::Node GetYAMLNode(const char * key) const;

  void Dump() const;

private:
  TString fYAMLString;

  ClassDef(AbsConfList, 2)
};
