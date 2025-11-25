#include <string>
#include <vector>

#include "TRandom3.h"

#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5DataWriter.hh"
#include "HDF5Utils/H5Event.hh"

#include "hdf5.h"

int main(int argc, char ** argv)
{
  if (argc < 4) { return 1; }

  const int nch = std::stoi(argv[1]);
  const int nevent = std::stoi(argv[2]);
  const int nfile = std::stoi(argv[3]);

  EventInfo_t eventinfo{};
  eventinfo.ttype = 0;
  eventinfo.nhit = static_cast<std::uint16_t>(nch);

  std::vector<AChannel_t> chdata;
  chdata.reserve(nch);

  for (int j = 0; j < nch; ++j) {
    AChannel_t ch{};
    ch.id = static_cast<std::uint16_t>(j);
    ch.tbit = 1;
    ch.adc = static_cast<std::uint32_t>(gRandom->Gaus(30000.0, 100.0));
    ch.time = static_cast<std::uint32_t>(20482048);
    chdata.push_back(ch);
  }

  auto * event = new H5Event<AChannel_t>;
  auto * hfile = new H5DataWriter("sadc.h5", 1);
  hfile->SetEvent(event);

  for (int n = 0; n < nfile; ++n) {
    if (!hfile->Open()) {
      delete event;
      delete hfile;
      return 0;
    }

    for (int i = 0; i < nevent; ++i) {
      eventinfo.tnum = static_cast<std::uint32_t>(i);
      eventinfo.ttime = static_cast<std::uint64_t>(2048LL * i);
      event->WriteEvent(eventinfo, chdata);
    }

    hfile->Close();
  }

  hfile->PrintStats();

  delete event;
  delete hfile;

  return 0;
}
