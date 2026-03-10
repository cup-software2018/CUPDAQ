#pragma once

#include "DAQConfig/FADCSConf.hh"

class MADCSConf : public FADCSConf {
public:
  MADCSConf();
  explicit MADCSConf(int sid);
  ~MADCSConf() override = default;

  ClassDef(MADCSConf, 1)
};
