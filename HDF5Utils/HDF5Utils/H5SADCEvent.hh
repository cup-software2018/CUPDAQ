#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "HDF5Utils/AbsH5Event.hh"
#include "HDF5Utils/EDM.hh"

#include "hdf5.h"

class H5SADCEvent : public AbsH5Event {
public:
  H5SADCEvent();
  ~H5SADCEvent() override;

  void Open() override;
  void Close() override;

  herr_t AppendEvent(const EventInfo_t & info, const std::vector<AChannel_t> & data);
  herr_t ReadEvent(int n);

  AChannel_t * GetData();

protected:
  herr_t FlushBuffer() override;

private:
  std::uint64_t fTotalChannels{0};

  std::vector<AChannel_t> fChBuf;

  // Pre-allocated vector buffer to avoid new/delete overhead
  std::vector<AChannel_t> fDataBuf;

  // Track current file ID in read mode to avoid redundant dataset open/close
  hid_t fCurrentReadFid{H5I_INVALID_HID};

  ClassDef(H5SADCEvent, 0)
};

inline AChannel_t * H5SADCEvent::GetData() { return fDataBuf.data(); }