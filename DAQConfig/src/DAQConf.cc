#include "DAQConfig/DAQConf.hh"

using namespace std;

ClassImp(DAQConf)

DAQConf::DAQConf()
    : AbsConf()
{
  SetNameTitle("DAQ", "DAQ machine");
}

DAQConf::~DAQConf() {}

void DAQConf::AddDAQ(int id, string name, string ipaddr, int port)
{
  auto daq = std::make_tuple(id, name, ipaddr, port);
  fDAQs.push_back(daq);
}

int DAQConf::GetN() const
{
  return fDAQs.size();
}

int DAQConf::GetID(int i) const
{
  return get<0>(fDAQs.at(i));
}

string DAQConf::GetDAQName(int id) const
{
  for (auto daq : fDAQs) {
    if (get<0>(daq) == id) return get<1>(daq);
  }
  return "none";
}

string DAQConf::GetIPAddr(int id) const
{
  for (auto daq : fDAQs) {
    if (get<0>(daq) == id) return get<2>(daq);
  }
  return "none";
}

int DAQConf::GetPort(int id) const
{
  for (auto daq : fDAQs) {
    if (get<0>(daq) == id) return get<3>(daq);
  }
  return 0;  
}