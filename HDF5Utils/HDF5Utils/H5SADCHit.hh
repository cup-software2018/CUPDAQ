#pragma once

#include <cstdint>
#include <vector>

#include "HDF5Utils/AbsH5Hit.hh"
#include "HDF5Utils/EDM.hh"

#include "hdf5.h"

class H5SADCHit : public AbsH5Hit {
public:
  H5SADCHit();
  ~H5SADCHit() override;

  void Open() override;
  void Close() override;

  // Append a single self-triggered SADC hit
  herr_t AppendHit(const AChannel_t & hit);

  // Read the n-th hit from the stream
  herr_t ReadHit(std::uint64_t n) override;

  // Accessor for the currently read hit
  const AChannel_t & GetHit() const;

protected:
  herr_t FlushBuffer() override;

private:
  // Write buffer for SADC hits
  std::vector<AChannel_t> fChBuf;

  // Data holder for currently read single hit
  AChannel_t fCurrentHit{};

  // Trackers and Cached Dataspaces for reading
  hid_t fCurrentReadFid{H5I_INVALID_HID};
  hid_t fFileSpaceChs{H5I_INVALID_HID};

  // ==========================================
  // Prefetching Buffers (Hit-based sliding window)
  // ==========================================
  std::vector<AChannel_t> fPrefetchChs;

  std::uint64_t fReadBufStart{0};
  std::uint64_t fReadBufSize{0};

  ClassDef(H5SADCHit, 0)
};

// === inline definitions ===

inline const AChannel_t & H5SADCHit::GetHit() const { return fCurrentHit; }