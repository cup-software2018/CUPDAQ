#ifndef EDM_hh
#define EDM_hh

#include <map>

#include "TString.h"
#include "hdf5.h"

const int kH5FADCNDP = 512;
const int kH5AMORENDP = 30000;

struct SubRun_t {
  unsigned int subrun;
  unsigned int nevent;
  unsigned int first;
  unsigned int last;
  hid_t BuildType()
  {
    hid_t type = H5Tcreate(H5T_COMPOUND, sizeof(SubRun_t));
    H5Tinsert(type, "subrun", HOFFSET(SubRun_t, subrun), H5T_NATIVE_UINT);
    H5Tinsert(type, "nevent", HOFFSET(SubRun_t, nevent), H5T_NATIVE_UINT);
    H5Tinsert(type, "first", HOFFSET(SubRun_t, first), H5T_NATIVE_UINT);
    H5Tinsert(type, "last", HOFFSET(SubRun_t, last), H5T_NATIVE_UINT);
    return type;
  }
  hsize_t GetSize() { return 16; }
};

struct EventInfo_t {
  unsigned short ttype;
  unsigned short nhit;
  unsigned int tnum;
  unsigned long ttime;
  hid_t BuildType()
  {
    hid_t type = H5Tcreate(H5T_COMPOUND, sizeof(EventInfo_t));
    H5Tinsert(type, "ttype", HOFFSET(EventInfo_t, ttype), H5T_NATIVE_USHORT);
    H5Tinsert(type, "nhit", HOFFSET(EventInfo_t, nhit), H5T_NATIVE_USHORT);
    H5Tinsert(type, "tnum", HOFFSET(EventInfo_t, tnum), H5T_NATIVE_UINT);
    H5Tinsert(type, "ttime", HOFFSET(EventInfo_t, ttime), H5T_NATIVE_ULONG);
    return type;
  }
  hsize_t GetSize() { return 16; }
};

struct FChannel_t {
  unsigned short id;
  unsigned short tbit;
  unsigned short ped;
  unsigned short waveform[kH5FADCNDP];
  hid_t BuildType()
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
  void SetWaveform(unsigned short * wave)
  {
    memcpy(waveform, wave, kH5FADCNDP * sizeof(unsigned short));
  }
  hsize_t GetSize() { return 6 + kH5FADCNDP * 2; }
};

struct AChannel_t {
  unsigned short id;
  unsigned short tbit;
  unsigned int adc;
  unsigned int time;
  hid_t BuildType()
  {
    hid_t type = H5Tcreate(H5T_COMPOUND, sizeof(AChannel_t));
    H5Tinsert(type, "id", HOFFSET(AChannel_t, id), H5T_NATIVE_USHORT);
    H5Tinsert(type, "bit", HOFFSET(AChannel_t, tbit), H5T_NATIVE_USHORT);
    H5Tinsert(type, "adc", HOFFSET(AChannel_t, adc), H5T_NATIVE_UINT);
    H5Tinsert(type, "time", HOFFSET(AChannel_t, time), H5T_NATIVE_UINT);
    return type;
  }
  hsize_t GetSize() { return 12; }
};

struct Crystal_t {
  unsigned short id;
  unsigned short phonon[kH5AMORENDP];
  unsigned short photon[kH5AMORENDP];
  unsigned long ttime;
  hid_t BuildType()
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
  void SetPhonon(unsigned short * wave)
  {
    memcpy(phonon, wave, kH5AMORENDP * sizeof(unsigned short));
  }
  void SetPhoton(unsigned short * wave)
  {
    memcpy(photon, wave, kH5AMORENDP * sizeof(unsigned short));
  }
  void SetWaveform(unsigned short * pn, unsigned short * pt)
  {
    SetPhonon(pn);
    SetPhoton(pt);
  }
  hsize_t GetSize() { return 10 + 2 * 2 * kH5AMORENDP; }
};

struct DataFile_t {
  hid_t fid;
  TString filename;
  hsize_t memsize;
  hsize_t filesize;
  std::map<int, int> entries;
};

#endif