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

  void SetBufferEventCapacity(int n);
  void SetBufferMaxBytes(std::size_t nbytes);

  herr_t AppendEvent(const EventInfo_t & info, const std::vector<AChannel_t> & data);
  herr_t ReadEvent(int n);

  EventInfo_t GetEventInfo() const;
  AChannel_t * GetData() const;

private:
  int fBufEventCap{1000};
  std::size_t fBufMaxBytes{32 * 1024 * 1024};
  int fBufEventCount{0};
  std::size_t fBufBytesUsed{0};

  AChannel_t * fData{nullptr};

  hid_t fDsetInfo{H5I_INVALID_HID};
  hid_t fDsetIndex{H5I_INVALID_HID};
  hid_t fDsetChs{H5I_INVALID_HID};

  std::uint64_t fTotalEvents{0};
  std::uint64_t fTotalChannels{0};

  std::vector<EventInfo_t> fEvtBuf;
  std::vector<AChannel_t> fChBuf;

  herr_t FlushBuffer();

  ClassDef(H5SADCEvent, 0)
};

inline void H5SADCEvent::SetBufferEventCapacity(int n)
{
  if (n > 0) { fBufEventCap = n; }
}

inline void H5SADCEvent::SetBufferMaxBytes(std::size_t nbytes)
{
  if (nbytes > 0) { fBufMaxBytes = nbytes; }
}

inline EventInfo_t H5SADCEvent::GetEventInfo() const { return fEvtInfo; }

inline AChannel_t * H5SADCEvent::GetData() const { return fData; }
