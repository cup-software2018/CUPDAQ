#include <vector>
#include <algorithm>
#include <random>

#include "TString.h"
#include "TRandom3.h"
#include "hdf5.h"

#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5Event.hh"
#include "HDF5Utils/H5DataWriter.hh"

using namespace std;

int main(int argc, char ** argv)
{
  int nevent = TString(argv[1]).Atoi();
  int nfile = TString(argv[2]).Atoi();
  int nch_max = 90;

  vector<Crystal_t> maxdata;

  for (int j = 0; j < nch_max; j++) {
    Crystal_t ch;
    ch.id = j;
    for (int k = 0; k < kH5AMORENDP; k++) {
      ch.phonon[k] = unsigned(gRandom->Gaus(32768, 10));
      ch.photon[k] = unsigned(gRandom->Gaus(32786, 10));
    }
    maxdata.push_back(ch);
  }  

  auto rng = std::default_random_engine {};

  auto * event = new H5Event<Crystal_t>;

  for (int l = 0; l < nfile; l++) {
    auto * hfile = new H5DataWriter(Form("crystal.h5.%05d", l));
    hfile->SetEvent(event);
    hfile->SetSubrun(l);

    if (!hfile->Open()) break;

    for (int i = 0; i < nevent; i++) {
      EventInfo_t eventinfo;
      eventinfo.ttype = 0;
      eventinfo.tnum = nevent*l + i;
      eventinfo.ttime = 4096*l + 2048*i;
      eventinfo.nhit = gRandom->Integer(maxdata.size()) + 1;

      shuffle(std::begin(maxdata), std::end(maxdata), rng);

      vector<Crystal_t> chdata;
      for (int j = 0; j < eventinfo.nhit; j++) {
        maxdata[j].ttime = 2048*i;
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