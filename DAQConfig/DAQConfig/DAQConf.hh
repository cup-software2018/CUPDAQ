#ifndef DAQConf_hh
#define DAQConf_hh

#include <string>
#include <tuple>
#include <vector>

#include "DAQConfig/AbsConf.hh"

class DAQConf : public AbsConf {
public:
  DAQConf();
  ~DAQConf() override = default;

  void AddDAQ(int id, std::string name, std::string ipaddr, int port);

  int GetN() const;
  int GetID(int i) const;
  std::string GetDAQName(int id) const;
  std::string GetIPAddr(int id) const;
  int GetPort(int id) const;

  void SetCID(int ch, int val) override {}
  void SetPID(int ch, int val) override {}
  int CID(int) const override { return 0; }
  int PID(int) const override { return 0; }
  void PrintConf() const override;

private:
  std::vector<std::tuple<int, std::string, std::string, int>> fDAQs;

  ClassDef(DAQConf, 1)
};

#endif
