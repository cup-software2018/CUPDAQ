#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "HDF5Utils/AbsH5Event.hh"
#include "HDF5Utils/EDM.hh"

#include "hdf5.h"

class H5AMOREEvent : public AbsH5Event {
public:
  H5AMOREEvent();
  ~H5AMOREEvent() override;

  void Open() override;
  void Close() override;

  herr_t AppendEvent(const EventInfo_t & info, const std::vector<Crystal_t> & data);
  herr_t ReadEvent(int n);

  void SetNDP(int ndp);
  int GetNDP();

  Crystal_t * GetData();

protected:
  herr_t FlushBuffer() override;

private:
  hid_t fDsetPhonon{H5I_INVALID_HID};
  hid_t fDsetPhoton{H5I_INVALID_HID};

  std::uint64_t fTotalCrystals{0};

  int fNDP{0};

  std::vector<CrystalHeader_t> fChBuf;
  std::vector<std::uint16_t> fPhononBuf;
  std::vector<std::uint16_t> fPhotonBuf;

  // Optimization: Pre-allocated vector buffer to avoid new/delete overhead
  std::vector<Crystal_t> fDataBuf;

  // Optimization: Track current file ID to avoid redundant open/close
  hid_t fCurrentReadFid{H5I_INVALID_HID};

  // Optimization: Cache DataSpaces for blazing fast read speeds
  hid_t fFileSpaceInfo{H5I_INVALID_HID};
  hid_t fFileSpaceIndex{H5I_INVALID_HID};
  hid_t fFileSpaceChs{H5I_INVALID_HID};
  hid_t fFileSpacePhonon{H5I_INVALID_HID};
  hid_t fFileSpacePhoton{H5I_INVALID_HID};
  hid_t fMemSpaceEvt{H5I_INVALID_HID};

  ClassDef(H5AMOREEvent, 0)
};

inline void H5AMOREEvent::SetNDP(int ndp) { fNDP = ndp; }

inline Crystal_t * H5AMOREEvent::GetData() { return fDataBuf.data(); }