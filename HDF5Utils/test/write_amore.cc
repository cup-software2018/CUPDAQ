#include <algorithm>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

#include "TRandom3.h"

#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5DataWriter.hh"
#include "HDF5Utils/H5Event.hh"

#include "hdf5.h"

int main(int argc, char ** argv)
{
  if (argc < 3) { return 1; }

  const int nevent = std::stoi(argv[1]);
  const int nfile = std::stoi(argv[2]);
  const int nch_max = 90;

  std::vector<Crystal_t> maxdata;
  maxdata.reserve(nch_max);

  for (int j = 0; j < nch_max; ++j) {
    Crystal_t ch{};
    ch.id = static_cast<std::uint16_t>(j);
    for (int k = 0; k < kH5AMORENDP; ++k) {
      ch.phonon[k] = static_cast<std::uint16_t>(gRandom->Gaus(32768.0, 10.0));
      ch.photon[k] = static_cast<std::uint16_t>(gRandom->Gaus(32786.0, 10.0));
    }
    maxdata.push_back(ch);
  }

  std::default_random_engine rng{std::random_device{}()};

  auto * event = new H5Event<Crystal_t>;

  for (int l = 0; l < nfile; ++l) {
    char fname[64];
    std::snprintf(fname, sizeof(fname), "crystal.h5.%05d", l);

    auto * hfile = new H5DataWriter(fname);
    hfile->SetEvent(event);
    hfile->SetSubrun(l);

    if (!hfile->Open()) {
      delete hfile;
      break;
    }

    for (int i = 0; i < nevent; ++i) {
      EventInfo_t eventinfo{};
      eventinfo.ttype = 0;
      eventinfo.tnum = static_cast<std::uint32_t>(nevent * l + i);
      eventinfo.ttime = static_cast<std::uint64_t>(4096LL * l + 2048LL * i);
      eventinfo.nhit = static_cast<std::uint16_t>(gRandom->Integer(static_cast<Int_t>(maxdata.size())) + 1);

      std::shuffle(maxdata.begin(), maxdata.end(), rng);

      std::vector<Crystal_t> chdata;
      chdata.reserve(eventinfo.nhit);
      for (std::uint16_t j = 0; j < eventinfo.nhit; ++j) {
        maxdata[j].ttime = static_cast<std::uint64_t>(2048LL * i);
        chdata.push_back(maxdata[j]);
      }

      event->WriteEvent(eventinfo, chdata);
    }

    hfile->PrintStats();
    hfile->Close();
    delete hfile;
  }

  delete event;

  return 0;
}
