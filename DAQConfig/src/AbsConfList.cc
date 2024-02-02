#include "DAQConfig/AbsConfList.hh"
#include <iostream>

using namespace std;

ClassImp(AbsConfList)

    AbsConfList::AbsConfList()
    : TObjArray()
{
}

AbsConfList::~AbsConfList() {}

int AbsConfList::GetNADC(ADC::TYPE type, bool isalive) const
{
  int n = 0;
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; i++) {
    auto * conf = (AbsConf *)fCont[i];
    if (conf->GetADCType() == type) {
      if (!isalive) { n += 1; }
      else {
        if (conf->IsEnabled() && conf->IsLinked()) { n += 1; }
      }
    }
  }
  return n;
}

AbsConf * AbsConfList::FindConfig(int mid) const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; ++i) {
    auto * conf = (AbsConf *)fCont[i];
    if (conf->MID() == mid) { return conf; }
  }
  return nullptr;
}

AbsConf * AbsConfList::FindConfig(const char * name, int mid) const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; ++i) {
    auto * conf = (AbsConf *)fCont[i];
    if (TString(conf->GetName()).EqualTo(name)) {
      if (conf->MID() == mid) { return conf; }
    }
  }
  return nullptr;
}

AbsConf * AbsConfList::FindConfig(ADC::TYPE type, int mid) const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; i++) {
    auto * conf = (AbsConf *)fCont[i];
    if (conf->GetADCType() == type) {
      if (conf->MID() == mid) { return conf; }
    }
  }
  return nullptr;
}

AbsConf * AbsConfList::GetConfig(ADC::TYPE type, int n) const
{
  int nn = 0;

  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; i++) {
    auto * conf = (AbsConf *)fCont[i];
    if (conf->GetADCType() == type) {
      if (nn == n) { return conf; }
      nn += 1;
    }
  }
  return nullptr;
}

AbsConf * AbsConfList::GetTCBConfig() const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; i++) {
    auto * conf = (AbsConf *)fCont[i];
    if (conf->GetADCType() == ADC::TCB) { return conf; }
  }
  return nullptr;
}

AbsConf * AbsConfList::GetSTRGConfig(ADC::TYPE type) const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; i++) {
    auto * conf = (AbsConf *)fCont[i];
    if (conf && strcmp("STRG", conf->GetName()) == 0) {
      if (conf->GetADCType() == type) { return conf; }
    }
  }
  return nullptr;
}

AbsConf * AbsConfList::GetDAQConfig() const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; i++) {
    auto * conf = (AbsConf *)fCont[i];
    if (conf && strcmp("DAQ", conf->GetName()) == 0) return conf;
  }
  return nullptr;
}

void AbsConfList::Dump() const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; i++) {
    auto * conf = (AbsConf *)fCont[i];
    cout << conf->InfoStr() << endl;
  }
}