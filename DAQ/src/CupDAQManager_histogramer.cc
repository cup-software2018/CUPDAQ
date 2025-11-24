#include <memory>

#include "TRandom3.h"

#include "DAQ/CupDAQManager.hh"
#include "OnlHistogramer/AbsHistogramer.hh"
#include "OnlHistogramer/FADCHistogramer.hh"
#include "OnlHistogramer/SADCHistogramer.hh"

void CupDAQManager::TF_Histogramer()
{
  if (!ThreadWait(fRunStatus, fDoExit)) { return; }
  INFO("histogramer started");

  //
  // Create histogramer
  //
  std::unique_ptr<AbsHistogramer> histogramer;

  switch (fADCMode) {
    case ADC::FMODE: histogramer = std::make_unique<FADCHistogramer>(); break;
    case ADC::SMODE: histogramer = std::make_unique<SADCHistogramer>(); break;
    default: WARNING("unsupported ADC mode in TF_Histogramer"); return;
  }

  histogramer->SetRunNumber(fRunNumber);
  histogramer->SetADCType(fADCType);
  histogramer->SetConfigList(fConfigList);
  histogramer->SetStartDatime(fStartDatime);

  // open histogramer root file
  if (fHistFilename.IsNull()) {
    TString filename;
    TString dirname = gSystem->Getenv("RAWDATA_DIR");
    if (dirname.IsNull()) {
      WARNING("variable RAWDATA_DIR is not set");
      filename = Form("hist_%s_%06d.root", GetADCName(fADCType), fRunNumber);
    }
    else {
      dirname += Form("/HIST/%06d", fRunNumber);
      int isdir = gSystem->Exec(Form("test -d %s", dirname.Data()));
      if (!isdir) {
        gSystem->Exec(Form("mkdir %s", dirname.Data()));
        INFO("%s created", dirname.Data());
      }
      INFO("%s already exist", dirname.Data());

      filename = Form("%s/hist_%s_%06d.root", dirname.Data(), GetADCName(fADCType), fRunNumber);
    }
    histogramer->SetFilename(filename);
    fHistFilename = filename;
  }

  if (!histogramer->Open()) {
    WARNING("cannot open histogramer root file %s, histogramer will be ended", fHistFilename.Data());
    return;
  }

  // booking histograms
  histogramer->Book();
  histogramer->Update();

  int eventnumber = 0;
  int ntotalmonitoredevent = 0;

  double mfrac = 0.3; // monitoring fraction: 30%

  double perror = 0.0;
  double integral = 0.0;

  TStopwatch sw;
  sw.Start();

  fBenchmark->Start("Histogramer");
  while (true) {
    // for emergent exit
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }
    if (fBuildStatus == ENDED) {
      if (fBuiltEventBuffer2.empty()) { break; }
    }

    std::unique_ptr<BuiltEvent> builtevent;

    int size = static_cast<int>(fBuiltEventBuffer2.size());
    if (size > 0) {
      auto opt = fBuiltEventBuffer2.pop_front();
      if (opt) { builtevent = std::move(*opt); }
    }

    if (builtevent) {
      eventnumber = builtevent->GetEventNumber();
      if (gRandom->Rndm() < mfrac) {
        histogramer->Fill(builtevent.get());
        ntotalmonitoredevent += 1;
      }
      // unique_ptr goes out of scope and deletes builtevent
    }

    // update histogramer every 1s
    double elapsetime = sw.RealTime();
    sw.Continue();
    if (elapsetime >= 1.0) {
      sw.Start(true);
      histogramer->Update();
    }

    ThreadSleep(fHistSleep, perror, integral, size);
  }
  fBenchmark->Stop("Histogramer");

  histogramer->Close();

  fHistogramerEnded = true;

  if (eventnumber > 0) {
    INFO("total monitored event = %d (%.2f%%)", ntotalmonitoredevent,
         100.0 * ntotalmonitoredevent / static_cast<double>(eventnumber));
  }
  else {
    INFO("total monitored event = %d (0.00%%)", ntotalmonitoredevent);
  }

  INFO("online histogramer ended");
}
