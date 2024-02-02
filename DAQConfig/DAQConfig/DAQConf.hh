#ifndef DAQConf_hh
#define DAQConf_hh

#include <vector>
#include <tuple>
#include <string>

#include "DAQConfig/AbsConf.hh"

class DAQConf : public AbsConf {
public:
  DAQConf();
  virtual ~DAQConf();

  void AddDAQ(int id, std::string name, std::string ipaddr, int port);

  int GetN() const;
  int GetID(int i) const;
  std::string GetDAQName(int id) const;
  std::string GetIPAddr(int id) const;
  int GetPort(int id) const;

  //useless 
  virtual void SetCID(int ch, int val) {}
  virtual void SetPID(int ch, int val) {}
  virtual int CID(int ch) const { return 0; }
  virtual int PID(int ch) const { return 0; }
  virtual void PrintConf() const {}

private:
  std::vector<std::tuple<int,std::string,std::string,int> > fDAQs;

  ClassDef(DAQConf, 1)
};

#endif