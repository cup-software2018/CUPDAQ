#pragma once

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
  herr_t ReadEvent(int n);

  void SetNDP(int ndp);
  int GetNDP();

  FChannel_t * GetData();

protected:
  herr_t FlushBuffer() override;

private:
  hid_t fDsetWave{H5I_INVALID_HID};

  std::uint64_t fTotalChannels{0};

  int fNDP{0};

  std::vector<FChannelHeader_t> fChBuf;
  std::vector<std::uint16_t> fWaveBuf;

  // Optimization: Pre-allocated vector buffer to avoid new/delete overhead
  std::vector<FChannel_t> fDataBuf;

  // Optimization: Track current file ID to avoid redundant open/close
  hid_t fCurrentReadFid{H5I_INVALID_HID};

  // Optimization: Cache DataSpaces for blazing fast read speeds
  hid_t fFileSpaceInfo{H5I_INVALID_HID};
  hid_t fFileSpaceIndex{H5I_INVALID_HID};
  hid_t fFileSpaceChs{H5I_INVALID_HID};
  hid_t fFileSpaceWave{H5I_INVALID_HID};
  hid_t fMemSpaceEvt{H5I_INVALID_HID};

  ClassDef(H5FADCEvent, 0)
};

inline void H5FADCEvent::SetNDP(int ndp) { fNDP = ndp; }

inline FChannel_t * H5FADCEvent::GetData() { return fDataBuf.data(); }