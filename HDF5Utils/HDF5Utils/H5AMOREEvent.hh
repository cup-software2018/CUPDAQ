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

  Crystal_t * GetData() const;

protected:
  herr_t FlushBuffer() override;

private:
  Crystal_t * fData{nullptr};

  hid_t fDsetPhonon{H5I_INVALID_HID};
  hid_t fDsetPhoton{H5I_INVALID_HID};

  std::uint64_t fTotalCrystals{0};

  int fNDP{0};

  std::vector<CrystalHeader_t> fChBuf;
  std::vector<std::uint16_t> fPhononBuf;
  std::vector<std::uint16_t> fPhotonBuf;

  ClassDef(H5AMOREEvent, 0)
};

inline void H5AMOREEvent::SetNDP(int ndp) { fNDP = ndp; }

inline Crystal_t * H5AMOREEvent::GetData() const { return fData; }
