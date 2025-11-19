#include "DAQConfig/DAQConf.hh"

ClassImp(DAQConf)

DAQConf::DAQConf()
  : AbsConf()
{
  SetNameTitle("DAQ", "DAQ machine");
}

void DAQConf::AddDAQ(int id, std::string name, std::string ipaddr, int port)
{
  fDAQs.emplace_back(id, std::move(name), std::move(ipaddr), port);
}

int DAQConf::GetN() const { return static_cast<int>(fDAQs.size()); }

int DAQConf::GetID(int i) const { return std::get<0>(fDAQs.at(i)); }

std::string DAQConf::GetDAQName(int id) const
{
  for (const auto & daq : fDAQs)
    if (std::get<0>(daq) == id) return std::get<1>(daq);
  return "none";
}

std::string DAQConf::GetIPAddr(int id) const
{
  for (const auto & daq : fDAQs)
    if (std::get<0>(daq) == id) return std::get<2>(daq);
  return "none";
}

int DAQConf::GetPort(int id) const
{
  for (const auto & daq : fDAQs)
    if (std::get<0>(daq) == id) return std::get<3>(daq);
  return 0;
}
