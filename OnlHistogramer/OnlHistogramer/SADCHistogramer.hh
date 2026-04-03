#pragma once
#include "OnlHistogramer/AbsHistogramer.hh"

class SADCHistogramer : public AbsHistogramer {
public:
  SADCHistogramer() = default;
  ~SADCHistogramer() override = default;

  void Book() override;
  void Fill(BuiltEvent * bevent) override;
};
