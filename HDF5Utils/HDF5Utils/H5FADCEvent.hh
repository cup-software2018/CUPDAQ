#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "HDF5Utils/AbsH5Event.hh"
#include "HDF5Utils/EDM.hh"

#include "hdf5.h"

class H5FADCEvent : public AbsH5Event {
public:
  H5FADCEvent();
  ~H5FADCEvent() override;

  void Open() override;
  void Close() override;

    herr_t AppendEvent(const EventInfo_t & info, const std::vector<FChannel_t> & data);
  herr_t ReadEvent(int n) override;

  void SetNDP(int ndp);
  int GetNDP();

  FChannel_t * GetData();

protected:
  herr_t FlushBuffer() override;

private:
  hid_t fChType{H5I_INVALID_HID};
  hid_t fDsetIndex{H5I_INVALID_HID};
  hid_t fDsetChs{H5I_INVALID_HID};
  hid_t fDsetWave{H5I_INVALID_HID};

  std::uint64_t fTotalChannels{0};
  int fNDP{0};

  // Write & Read internal buffers for Channels
  std::vector<FChannelHeader_t> fChBuf;
  std::vector<std::uint16_t> fWaveBuf;

  // Pre-allocated vector buffer for zero-overhead user access
  std::vector<FChannel_t> fDataBuf;

  // Trackers and Cached Dataspaces
  hid_t fCurrentReadFid{H5I_INVALID_HID};
  hid_t fFileSpaceInfo{H5I_INVALID_HID};
  hid_t fFileSpaceIndex{H5I_INVALID_HID};
  hid_t fFileSpaceChs{H5I_INVALID_HID};
  hid_t fFileSpaceWave{H5I_INVALID_HID};

  // ==========================================
  // Full Prefetching Buffers (Memory Safe Window)
  // ==========================================
  std::vector<EventInfo_t> fReadBufInfo;
  std::vector<std::uint64_t> fReadBufIndex;
  std::vector<FChannelHeader_t> fPrefetchChs;
  std::vector<std::uint16_t> fPrefetchWave;

  int fReadBufStart{-1};
  int fReadBufSize{0};
  std::uint64_t fPrefetchChStart{0};

  ClassDef(H5FADCEvent, 0)
};

inline void H5FADCEvent::SetNDP(int ndp) { fNDP = ndp; }

inline FChannel_t * H5FADCEvent::GetData() { return fDataBuf.data(); }