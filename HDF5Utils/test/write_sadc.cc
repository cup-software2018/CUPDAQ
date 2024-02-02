#include <vector>

#include "TString.h"
#include "TRandom3.h"
#include "hdf5.h"

#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5Event.hh"
#include "HDF5Utils/H5DataWriter.hh"

using namespace std;

int main(int argc, char ** argv)
{
  int nch = TString(argv[1]).Atoi();
  int nevent = TString(argv[2]).Atoi();
  int nfile = TString(argv[3]).Atoi();

  EventInfo_t eventinfo;
  eventinfo.ttype = 0;
  eventinfo.nhit = nch; 

  vector<AChannel_t> chdata;

  for (int j = 0; j < nch; j++) {
    AChannel_t ch;
    ch.id = j;
    ch.tbit = 1;
    ch.adc = unsigned(gRandom->Gaus(30000, 100));
    ch.time = 20482048;
    chdata.push_back(ch);
  }  

  auto * event = new H5Event<AChannel_t>;
  auto * hfile = new H5DataWriter("sadc.h5", 1);
  hfile->SetEvent(event);

  for (int n = 0; n < nfile; n++) {
    if (!hfile->Open()) return 0;

    for (int i = 0; i < nevent; i++) {
      eventinfo.tnum = i;
      eventinfo.ttime = 2048*i;
      event->WriteEvent(eventinfo, chdata);
    }
    hfile->Close();
  }
  
  hfile->PrintStats();

  delete event;
  delete hfile;
  
  return 0;
}