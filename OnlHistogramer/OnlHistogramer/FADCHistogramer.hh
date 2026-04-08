#pragma once

#include "OnlHistogramer/AbsHistogramer.hh"

class FADCHistogramer : public AbsHistogramer {
public:
  FADCHistogramer() = default;
  ~FADCHistogramer() override = default;

  void Book() override;
  void Fill(BuiltEvent * bevent) override;

private:
  int fNDP{};
};
