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

  void SetNDP(int ndp);
  void SetBufferEventCapacity(int n);
  void SetBufferMaxBytes(std::size_t nbytes);

  herr_t AppendEvent(const EventInfo_t & info, const std::vector<FChannel_t> & data);
  herr_t ReadEvent(int n);

  int GetNDP();
  EventInfo_t GetEventInfo() const;
  FChannel_t * GetData() const;

private:
  int fNDP{0};
  int fBufEventCap{100};
  std::size_t fBufMaxBytes{32 * 1024 * 1024};
  int fBufEventCount{0};
  std::size_t fBufBytesUsed{0};

  FChannel_t * fData{nullptr};

  hid_t fDsetInfo{H5I_INVALID_HID};
  hid_t fDsetIndex{H5I_INVALID_HID};
  hid_t fDsetChs{H5I_INVALID_HID};
  hid_t fDsetWave{H5I_INVALID_HID};

  std::uint64_t fTotalEvents{0};
  std::uint64_t fTotalChannels{0};

  std::vector<EventInfo_t> fEvtBuf;
  std::vector<FChannelHeader_t> fChBuf;
  std::vector<std::uint16_t> fWaveBuf;

  herr_t FlushBuffer();

  ClassDef(H5FADCEvent, 0)
};

inline void H5FADCEvent::SetNDP(int ndp) { fNDP = ndp; }

inline void H5FADCEvent::SetBufferEventCapacity(int n)
{
  if (n > 0) { fBufEventCap = n; }
}

inline void H5FADCEvent::SetBufferMaxBytes(std::size_t nbytes)
{
  if (nbytes > 0) { fBufMaxBytes = nbytes; }
}

inline EventInfo_t H5FADCEvent::GetEventInfo() const { return fEvtInfo; }

inline FChannel_t * H5FADCEvent::GetData() const { return fData; }
