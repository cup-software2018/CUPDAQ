#pragma once

#include <cstdint>
#include <vector>

#include "HDF5Utils/AbsH5Hit.hh"
#include "HDF5Utils/EDM.hh"

#include "hdf5.h"

class H5FADCHit : public AbsH5Hit {
public:
  H5FADCHit();
  ~H5FADCHit() override;

  void Open() override;
  void Close() override;

  // Append a single self-triggered FADC hit
  herr_t AppendHit(const FChannel_t & hit);

  // Read the n-th hit from the stream
  herr_t ReadHit(std::uint64_t n) override;

  void SetNDP(int ndp);
  int GetNDP();

  // Accessor for currently read hit data
  const FChannel_t & GetHit() const;

protected:
  herr_t FlushBuffer() override;

private:
  int fNDP{0};
  hid_t fDsetWave{H5I_INVALID_HID};

  // Write buffers for FADC hits
  std::vector<FChannelHeader_t> fChBuf;
  std::vector<std::uint16_t> fWaveBuf;

  // Data holder for currently read single hit
  FChannel_t fCurrentHit{};

  // Trackers and Cached Dataspaces for reading
  hid_t fCurrentReadFid{H5I_INVALID_HID};
  hid_t fFileSpaceChs{H5I_INVALID_HID};
  hid_t fFileSpaceWave{H5I_INVALID_HID};

  // ==========================================
  // Prefetching Buffers (Hit-based sliding window)
  // ==========================================
  std::vector<FChannelHeader_t> fPrefetchChs;
  std::vector<std::uint16_t> fPrefetchWave;

  std::uint64_t fReadBufStart{0};
  std::uint64_t fReadBufSize{0};

  ClassDef(H5FADCHit, 0)
};

// === inline definitions ===

inline void H5FADCHit::SetNDP(int ndp) { fNDP = ndp; }

inline const FChannel_t & H5FADCHit::GetHit() const { return fCurrentHit; }