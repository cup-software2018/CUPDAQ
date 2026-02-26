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

  AChannel_t * GetData() const;

protected:
  herr_t FlushBuffer() override;

private:
  AChannel_t * fData{nullptr};

  std::uint64_t fTotalChannels{0};

  std::vector<AChannel_t> fChBuf;

  ClassDef(H5SADCEvent, 0)
};

inline AChannel_t * H5SADCEvent::GetData() const { return fData; }
