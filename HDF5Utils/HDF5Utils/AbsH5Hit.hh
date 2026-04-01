// AbsH5Hit.hh

#pragma once

#include <cstdint>

#include "HDF5Utils/AbsH5Base.hh"
#include "HDF5Utils/EDM.hh"

#include "hdf5.h"

class AbsH5Hit : public AbsH5Base {
public:
  AbsH5Hit() = default;
  ~AbsH5Hit() override = default;

  // Pure virtual method for reading the n-th hit stream
  // AppendHit is not defined here because FADC and SADC have different hit structures
  virtual herr_t ReadHit(std::uint64_t n) = 0;

  // Retrieve the total number of hits
  std::uint64_t GetTotalHits() const;

protected:
  // HDF5 datatype and dataset for hit channels
  hid_t fChType{H5I_INVALID_HID};
  hid_t fDsetChs{H5I_INVALID_HID};

  // Total number of hits processed
  std::uint64_t fTotalHits{0};

  ClassDef(AbsH5Hit, 0)
};

inline std::uint64_t AbsH5Hit::GetTotalHits() const { return fTotalHits; }