#pragma once

#include <algorithm>
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
  herr_t ReadEvent(int n) override;

  AChannel_t * GetData();

protected:
  herr_t FlushBuffer() override;

private:
  hid_t fChType{H5I_INVALID_HID};
  hid_t fDsetIndex{H5I_INVALID_HID};
  hid_t fDsetChs{H5I_INVALID_HID};

  std::uint64_t fTotalChannels{0};

  // Write buffers for SADC Channels
  std::vector<AChannel_t> fChBuf;

  // Pre-allocated vector buffer for zero-overhead user access
  std::vector<AChannel_t> fDataBuf;

  // Trackers and Cached Dataspaces
  hid_t fCurrentReadFid{H5I_INVALID_HID};
  hid_t fFileSpaceInfo{H5I_INVALID_HID};
  hid_t fFileSpaceIndex{H5I_INVALID_HID};
  hid_t fFileSpaceChs{H5I_INVALID_HID};

  // ==========================================
  // Full Prefetching Buffers (Memory Safe Window)
  // ==========================================
  std::vector<EventInfo_t> fReadBufInfo;
  std::vector<std::uint64_t> fReadBufIndex;
  std::vector<AChannel_t> fPrefetchChs;

  int fReadBufStart{-1};
  int fReadBufSize{0};
  std::uint64_t fPrefetchChStart{0};

  ClassDef(H5SADCEvent, 0)
};

inline AChannel_t * H5SADCEvent::GetData() { return fDataBuf.data(); }