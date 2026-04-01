// AbsH5Event.hh

#pragma once

#include <cstdint>
#include <vector>

#include "HDF5Utils/AbsH5Base.hh"
#include "HDF5Utils/EDM.hh"

#include "hdf5.h"

class AbsH5Event : public AbsH5Base {
public:
  AbsH5Event() = default;
  ~AbsH5Event() override = default;

  // Retrieve the current event metadata
  EventInfo_t GetEventInfo() const;

  // Pure virtual method to read the n-th event
  // AppendEvent is not defined here because its signature varies by channel type (FChannel vs
  // AChannel)
  virtual herr_t ReadEvent(int n) = 0;

protected:
  // HDF5 datatype and dataset for event metadata (/events/info)
  hid_t fEvtType{H5I_INVALID_HID};
  hid_t fDsetInfo{H5I_INVALID_HID};

  // Current event information
  EventInfo_t fEvtInfo{};

  // Total number of events processed
  std::uint64_t fTotalEvents{0};

  // Internal buffer for event metadata
  std::vector<EventInfo_t> fEvtBuf;

  ClassDef(AbsH5Event, 0)
};

inline EventInfo_t AbsH5Event::GetEventInfo() const { return fEvtInfo; }