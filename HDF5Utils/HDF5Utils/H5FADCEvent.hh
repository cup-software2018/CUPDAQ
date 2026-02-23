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

  FChannel_t * GetData() const;

protected:
  herr_t FlushBuffer() override;

private:
  FChannel_t * fData{nullptr};

  hid_t fDsetWave{H5I_INVALID_HID};

  std::uint64_t fTotalChannels{0};

    int fNDP{0};

  std::vector<FChannelHeader_t> fChBuf;
  std::vector<std::uint16_t> fWaveBuf;

  ClassDef(H5FADCEvent, 0)
};

inline void H5FADCEvent::SetNDP(int ndp) { fNDP = ndp; }

inline FChannel_t * H5FADCEvent::GetData() const { return fData; }
