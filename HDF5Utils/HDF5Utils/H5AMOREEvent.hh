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

  void SetNDP(int ndp);
  void SetBufferEventCapacity(int n);
  void SetBufferMaxBytes(std::size_t nbytes);

  herr_t AppendEvent(const EventInfo_t & info, const std::vector<Crystal_t> & data);
  herr_t ReadEvent(int n);

  int GetNDP();
  EventInfo_t GetEventInfo() const;
  Crystal_t * GetData() const;

private:
  int fNDP{0};
  int fBufEventCap{100};
  std::size_t fBufMaxBytes{64 * 1024 * 1024};
  int fBufEventCount{0};
  std::size_t fBufBytesUsed{0};

  Crystal_t * fData{nullptr};

  hid_t fDsetInfo{H5I_INVALID_HID};
  hid_t fDsetIndex{H5I_INVALID_HID};
  hid_t fDsetChs{H5I_INVALID_HID};
  hid_t fDsetPhonon{H5I_INVALID_HID};
  hid_t fDsetPhoton{H5I_INVALID_HID};

  std::uint64_t fTotalEvents{0};
  std::uint64_t fTotalCrystals{0};

  std::vector<EventInfo_t> fEvtBuf;
  std::vector<CrystalHeader_t> fChBuf;
  std::vector<std::uint16_t> fPhononBuf;
  std::vector<std::uint16_t> fPhotonBuf;

  herr_t FlushBuffer();

  ClassDef(H5AMOREEvent, 0)
};

inline void H5AMOREEvent::SetNDP(int ndp) { fNDP = ndp; }

inline void H5AMOREEvent::SetBufferEventCapacity(int n)
{
  if (n > 0) { fBufEventCap = n; }
}

inline void H5AMOREEvent::SetBufferMaxBytes(std::size_t nbytes)
{
  if (nbytes > 0) { fBufMaxBytes = nbytes; }
}

inline EventInfo_t H5AMOREEvent::GetEventInfo() const { return fEvtInfo; }

inline Crystal_t * H5AMOREEvent::GetData() const { return fData; }
