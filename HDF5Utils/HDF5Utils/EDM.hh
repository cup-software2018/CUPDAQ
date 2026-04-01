#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>

#include "hdf5.h"

constexpr int kH5FADCNDPMAX = 16384; // 64 us for FADC

struct SubRun_t {
  std::uint32_t subrun;
  std::uint32_t nevent;
  std::uint32_t first;
  std::uint32_t last;

  static hid_t BuildType();
  hsize_t GetSize() const noexcept;
};

struct EventInfo_t {
  std::uint16_t ttype;
  std::uint16_t nhit;
  std::uint32_t tnum;
  std::uint64_t ttime;

  static hid_t BuildType();
  hsize_t GetSize() const noexcept;
};

struct FChannel_t {
  std::uint16_t id;
  std::uint16_t tbit;
  std::uint16_t ped;
  std::uint32_t time; // Added individual hit time
  std::uint16_t waveform[kH5FADCNDPMAX];

  FChannel_t() noexcept
    : id(0),
      tbit(0),
      ped(0),
      time(0)
  {
    std::memset(waveform, 0, sizeof(waveform));
  }

  static hid_t BuildType();
  hsize_t GetSize() const noexcept;

  void SetWaveform(const std::uint16_t * wave, int ndp) noexcept;
};

struct FChannelHeader_t {
  std::uint16_t id;
  std::uint16_t tbit;
  std::uint16_t ped;
  std::uint32_t time; // Added individual hit time

  static hid_t BuildType();
  hsize_t GetSize() const noexcept;
};

struct AChannel_t {
  std::uint16_t id;
  std::uint16_t tbit;
  std::uint32_t adc;
  std::uint32_t time;

  AChannel_t() noexcept
    : id(0),
      tbit(0),
      adc(0),
      time(0)
  {
  }

  static hid_t BuildType();
  hsize_t GetSize() const noexcept;
};

struct DataFile_t {
  hid_t fid;
  std::string filename;
  hsize_t memsize;
  hsize_t filesize;
  int global_start;
  int nevent;

  DataFile_t()
    : fid(H5I_INVALID_HID),
      filename(),
      memsize(0),
      filesize(0),
      global_start(0),
      nevent(0)
  {
  }
};

// === inline definitions ===

inline hid_t SubRun_t::BuildType()
{
  hid_t type = H5Tcreate(H5T_COMPOUND, sizeof(SubRun_t));
  H5Tinsert(type, "subrun", HOFFSET(SubRun_t, subrun), H5T_STD_U32LE);
  H5Tinsert(type, "nevent", HOFFSET(SubRun_t, nevent), H5T_STD_U32LE);
  H5Tinsert(type, "first", HOFFSET(SubRun_t, first), H5T_STD_U32LE);
  H5Tinsert(type, "last", HOFFSET(SubRun_t, last), H5T_STD_U32LE);
  return type;
}

inline hsize_t SubRun_t::GetSize() const noexcept { return sizeof(SubRun_t); }

inline hid_t EventInfo_t::BuildType()
{
  hid_t type = H5Tcreate(H5T_COMPOUND, sizeof(EventInfo_t));
  H5Tinsert(type, "ttype", HOFFSET(EventInfo_t, ttype), H5T_STD_U16LE);
  H5Tinsert(type, "nhit", HOFFSET(EventInfo_t, nhit), H5T_STD_U16LE);
  H5Tinsert(type, "tnum", HOFFSET(EventInfo_t, tnum), H5T_STD_U32LE);
  H5Tinsert(type, "ttime", HOFFSET(EventInfo_t, ttime), H5T_STD_U64LE);
  return type;
}

inline hsize_t EventInfo_t::GetSize() const noexcept { return sizeof(EventInfo_t); }

inline hid_t FChannel_t::BuildType()
{
  hid_t type = H5Tcreate(H5T_COMPOUND, sizeof(FChannel_t));
  H5Tinsert(type, "id", HOFFSET(FChannel_t, id), H5T_STD_U16LE);
  H5Tinsert(type, "tbit", HOFFSET(FChannel_t, tbit), H5T_STD_U16LE);
  H5Tinsert(type, "ped", HOFFSET(FChannel_t, ped), H5T_STD_U16LE);
  H5Tinsert(type, "time", HOFFSET(FChannel_t, time), H5T_STD_U32LE);

  hsize_t dim[1] = {kH5FADCNDPMAX};
  hid_t arrtype = H5Tarray_create2(H5T_STD_U16LE, 1, dim);
  H5Tinsert(type, "waveform", HOFFSET(FChannel_t, waveform), arrtype);

  return type;
}

inline void FChannel_t::SetWaveform(const std::uint16_t * wave, int ndp) noexcept
{
  std::memcpy(waveform, wave, static_cast<std::size_t>(ndp) * sizeof(std::uint16_t));
}

inline hsize_t FChannel_t::GetSize() const noexcept { return sizeof(FChannel_t); }

inline hid_t FChannelHeader_t::BuildType()
{
  hid_t type = H5Tcreate(H5T_COMPOUND, sizeof(FChannelHeader_t));
  H5Tinsert(type, "id", HOFFSET(FChannelHeader_t, id), H5T_STD_U16LE);
  H5Tinsert(type, "tbit", HOFFSET(FChannelHeader_t, tbit), H5T_STD_U16LE);
  H5Tinsert(type, "ped", HOFFSET(FChannelHeader_t, ped), H5T_STD_U16LE);
  H5Tinsert(type, "time", HOFFSET(FChannelHeader_t, time), H5T_STD_U32LE);
  return type;
}

inline hsize_t FChannelHeader_t::GetSize() const noexcept { return sizeof(FChannelHeader_t); }

inline hid_t AChannel_t::BuildType()
{
  hid_t type = H5Tcreate(H5T_COMPOUND, sizeof(AChannel_t));
  H5Tinsert(type, "id", HOFFSET(AChannel_t, id), H5T_STD_U16LE);
  H5Tinsert(type, "tbit", HOFFSET(AChannel_t, tbit), H5T_STD_U16LE);
  H5Tinsert(type, "adc", HOFFSET(AChannel_t, adc), H5T_STD_U32LE);
  H5Tinsert(type, "time", HOFFSET(AChannel_t, time), H5T_STD_U32LE);
  return type;
}

inline hsize_t AChannel_t::GetSize() const noexcept { return sizeof(AChannel_t); }