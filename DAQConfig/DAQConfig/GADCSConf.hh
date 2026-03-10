#pragma once

#include "DAQConfig/FADCSConf.hh"

class GADCSConf : public FADCSConf {
public:
  GADCSConf();
  explicit GADCSConf(int sid);
  ~GADCSConf() override = default;

  ClassDef(GADCSConf, 1)
};