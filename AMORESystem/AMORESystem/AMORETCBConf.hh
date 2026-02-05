#pragma once

#include "DAQConfig/AbsConf.hh"

class AMORETCBConf : public AbsConf {
public:
  AMORETCBConf() = default;
  explicit AMORETCBConf(int sid);
  ~AMORETCBConf() override = default;

  void SetCW(int cw);
  void SetDT(int dt);
  void SetSCALE(int s);

  int CW() const;
  int DT() const;
  int SCALE() const;

  void SetCID(int, int) override {}
  void SetPID(int, int) override {}
  int CID(int) const override { return 0; }
  int PID(int) const override { return 0; }

  void PrintConf() const override;

private:
  int fCW{};
  int fDT{};
  int fSCALE{};

  ClassDef(AMORETCBConf, 1)
};

inline void AMORETCBConf::SetCW(int cw) { fCW = cw; }
inline void AMORETCBConf::SetDT(int dt) { fDT = dt; }
inline void AMORETCBConf::SetSCALE(int s) { fSCALE = s; }

inline int AMORETCBConf::CW() const { return fCW; }
inline int AMORETCBConf::DT() const { return fDT; }
inline int AMORETCBConf::SCALE() const { return fSCALE; }