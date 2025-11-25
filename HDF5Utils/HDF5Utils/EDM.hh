#pragma once

#include <cstdint>
#include <cstring>
#include <map>
#include <string>

#include "hdf5.h"

constexpr int kH5FADCNDP = 512;
constexpr int kH5AMORENDP = 30000;

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
  std::uint16_t waveform[kH5FADCNDP];

  static hid_t BuildType();
  void SetWaveform(const std::uint16_t * wave) noexcept;
  hsize_t GetSize() const noexcept;
};

struct AChannel_t {
  std::uint16_t id;
  std::uint16_t tbit;
  std::uint32_t adc;
  std::uint32_t time;

  static hid_t BuildType();
  hsize_t GetSize() const noexcept;
};

struct Crystal_t {
  std::uint16_t id;
  std::uint16_t phonon[kH5AMORENDP];
  std::uint16_t photon[kH5AMORENDP];
  std::uint64_t ttime;

  static hid_t BuildType();
  void SetPhonon(const std::uint16_t * wave) noexcept;
  void SetPhoton(const std::uint16_t * wave) noexcept;
  void SetWaveform(const std::uint16_t * pn, const std::uint16_t * pt) noexcept;
  hsize_t GetSize() const noexcept;
};

struct DataFile_t {
  hid_t fid;
  std::string filename;
  hsize_t memsize;
  hsize_t filesize;
  std::map<int, int> entries;

  DataFile_t()
    : fid(H5I_INVALID_HID),
      filename(),
      memsize(0),
      filesize(0),
      entries()
  {
  }
};

// === inline definitions ===

inline hid_t SubRun_t::BuildType()
{
  hid_t type = H5Tcreate(H5T_COMPOUND, sizeof(SubRun_t));
  H5Tinsert(type, "subrun", HOFFSET(SubRun_t, subrun), H5T_NATIVE_UINT);
  H5Tinsert(type, "nevent", HOFFSET(SubRun_t, nevent), H5T_NATIVE_UINT);
  H5Tinsert(type, "first", HOFFSET(SubRun_t, first), H5T_NATIVE_UINT);
  H5Tinsert(type, "last", HOFFSET(SubRun_t, last), H5T_NATIVE_UINT);
  return type;
}

inline hsize_t SubRun_t::GetSize() const noexcept { return sizeof(SubRun_t); }

inline hid_t EventInfo_t::BuildType()
{
  hid_t type = H5Tcreate(H5T_COMPOUND, sizeof(EventInfo_t));
  H5Tinsert(type, "ttype", HOFFSET(EventInfo_t, ttype), H5T_NATIVE_USHORT);
  H5Tinsert(type, "nhit", HOFFSET(EventInfo_t, nhit), H5T_NATIVE_USHORT);
  H5Tinsert(type, "tnum", HOFFSET(EventInfo_t, tnum), H5T_NATIVE_UINT);
  H5Tinsert(type, "ttime", HOFFSET(EventInfo_t, ttime), H5T_NATIVE_ULONG);
  return type;
}

inline hsize_t EventInfo_t::GetSize() const noexcept { return sizeof(EventInfo_t); }

inline hid_t FChannel_t::BuildType()
{
  hid_t type = H5Tcreate(H5T_COMPOUND, sizeof(FChannel_t));
  H5Tinsert(type, "id", HOFFSET(FChannel_t, id), H5T_NATIVE_USHORT);
  H5Tinsert(type, "bit", HOFFSET(FChannel_t, tbit), H5T_NATIVE_USHORT);
  H5Tinsert(type, "ped", HOFFSET(FChannel_t, ped), H5T_NATIVE_USHORT);

  hsize_t dim[1] = {kH5FADCNDP};
  hid_t arrtype = H5Tarray_create2(H5T_NATIVE_USHORT, 1, dim);
  H5Tinsert(type, "waveform", HOFFSET(FChannel_t, waveform), arrtype);

  return type;
}

inline void FChannel_t::SetWaveform(const std::uint16_t * wave) noexcept
{
  std::memcpy(waveform, wave, kH5FADCNDP * sizeof(std::uint16_t));
}

inline hsize_t FChannel_t::GetSize() const noexcept { return sizeof(FChannel_t); }

inline hid_t AChannel_t::BuildType()
{
  hid_t type = H5Tcreate(H5T_COMPOUND, sizeof(AChannel_t));
  H5Tinsert(type, "id", HOFFSET(AChannel_t, id), H5T_NATIVE_USHORT);
  H5Tinsert(type, "bit", HOFFSET(AChannel_t, tbit), H5T_NATIVE_USHORT);
  H5Tinsert(type, "adc", HOFFSET(AChannel_t, adc), H5T_NATIVE_UINT);
  H5Tinsert(type, "time", HOFFSET(AChannel_t, time), H5T_NATIVE_UINT);
  return type;
}

inline hsize_t AChannel_t::GetSize() const noexcept { return sizeof(AChannel_t); }

inline hid_t Crystal_t::BuildType()
{
  hid_t type = H5Tcreate(H5T_COMPOUND, sizeof(Crystal_t));
  H5Tinsert(type, "id", HOFFSET(Crystal_t, id), H5T_NATIVE_USHORT);

  hsize_t dim[1] = {kH5AMORENDP};
  hid_t arrtype = H5Tarray_create2(H5T_NATIVE_USHORT, 1, dim);
  H5Tinsert(type, "phonon", HOFFSET(Crystal_t, phonon), arrtype);
  H5Tinsert(type, "photon", HOFFSET(Crystal_t, photon), arrtype);
  H5Tinsert(type, "ttime", HOFFSET(Crystal_t, ttime), H5T_NATIVE_ULONG);

  return type;
}

inline void Crystal_t::SetPhonon(const std::uint16_t * wave) noexcept
{
  std::memcpy(phonon, wave, kH5AMORENDP * sizeof(std::uint16_t));
}

inline void Crystal_t::SetPhoton(const std::uint16_t * wave) noexcept
{
  std::memcpy(photon, wave, kH5AMORENDP * sizeof(std::uint16_t));
}

inline void Crystal_t::SetWaveform(const std::uint16_t * pn, const std::uint16_t * pt) noexcept
{
  SetPhonon(pn);
  SetPhoton(pt);
}

inline hsize_t Crystal_t::GetSize() const noexcept { return sizeof(Crystal_t); }
