#include "DAQConfig/AbsConfList.hh"

ClassImp(AbsConfList)

AbsConfList::AbsConfList()
  : TObjArray()
{
}

int AbsConfList::GetNADC(ADC::TYPE type, bool isalive) const
{
  int nadc = GetAbsLast() + 1;
  int count = 0;

  for (int i = 0; i < nadc; i++) {
    auto * conf = static_cast<AbsConf *>(fCont[i]);
    if (conf->GetADCType() == type) {
      if (!isalive || (conf->IsEnabled() && conf->IsLinked())) { count++; }
    }
  }
  return count;
}

AbsConf * AbsConfList::FindConfig(int mid) const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; ++i) {
    auto * conf = static_cast<AbsConf *>(fCont[i]);
    if (conf->MID() == mid) return conf;
  }
  return nullptr;
}

AbsConf * AbsConfList::FindConfig(const char * name, int mid) const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; ++i) {
    auto * conf = static_cast<AbsConf *>(fCont[i]);
    if (TString(conf->GetName()).EqualTo(name) && conf->MID() == mid) return conf;
  }
  return nullptr;
}

AbsConf * AbsConfList::FindConfig(ADC::TYPE type, int mid) const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; i++) {
    auto * conf = static_cast<AbsConf *>(fCont[i]);
    if (conf->GetADCType() == type && conf->MID() == mid) return conf;
  }
  return nullptr;
}

AbsConf * AbsConfList::GetConfig(ADC::TYPE type, int n) const
{
  int nadc = GetAbsLast() + 1;
  int index = 0;

  for (int i = 0; i < nadc; i++) {
    auto * conf = static_cast<AbsConf *>(fCont[i]);
    if (conf->GetADCType() == type) {
      if (index == n) return conf;
      index++;
    }
  }
  return nullptr;
}

AbsConf * AbsConfList::GetTCBConfig() const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; i++) {
    auto * conf = static_cast<AbsConf *>(fCont[i]);
    if (conf->GetADCType() == ADC::TCB) return conf;
  }
  return nullptr;
}

AbsConf * AbsConfList::GetSTRGConfig(ADC::TYPE type) const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; i++) {
    auto * conf = static_cast<AbsConf *>(fCont[i]);
    if (conf && strcmp("STRG", conf->GetName()) == 0 && conf->GetADCType() == type) return conf;
  }
  return nullptr;
}

AbsConf * AbsConfList::GetDAQConfig() const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; i++) {
    auto * conf = static_cast<AbsConf *>(fCont[i]);
    if (conf && strcmp("DAQ", conf->GetName()) == 0) return conf;
  }
  return nullptr;
}

void AbsConfList::Dump() const
{
  int nadc = GetAbsLast() + 1;
  for (int i = 0; i < nadc; i++) {
    static_cast<AbsConf *>(fCont[i])->PrintConf();
  }
}
