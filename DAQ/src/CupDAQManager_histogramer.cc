#include "TRandom3.h"

#include "DAQ/CupDAQManager.hh"
#include "OnlHistogramer/AbsHistogramer.hh"
#include "OnlHistogramer/FADCHistogramer.hh"
#include "OnlHistogramer/SADCHistogramer.hh"

void CupDAQManager::TF_Histogramer()
{
  if (!ThreadWait(fRunStatus, fDoExit)) { return; }
  fLog->Info("CupDAQManager::TF_Histogramer", "histogramer started");

  //
  // Create histogramer
  //
  AbsHistogramer * histogramer = nullptr;
  switch (fADCMode) {
    case ADC::FMODE: histogramer = new FADCHistogramer(); break;
    case ADC::SMODE: histogramer = new SADCHistogramer(); break;
    default: break;
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
      fLog->Warning("CupDAQManager::TF_Histogramer",
                    "variable RAWDATA_DIR is not set");
      filename = Form("hist_%s_%06d.root", GetADCName(fADCType), fRunNumber);
    }
    else {
      dirname += Form("/HIST/%06d", fRunNumber);
      int isdir = gSystem->Exec(Form("test -d %s", dirname.Data()));
      if (!isdir) {
        gSystem->Exec(Form("mkdir %s", dirname.Data()));
        fLog->Info("CupDAQManager::TF_Histogramer", "%s created",
                   dirname.Data());
      }
      fLog->Info("CupDAQManager::TF_Histogramer", "%s already exist",
                 dirname.Data());

      filename = Form("%s/hist_%s_%06d.root", dirname.Data(),
                      GetADCName(fADCType), fRunNumber);
    }
    histogramer->SetFilename(filename);
    fHistFilename = filename;
  }

  if (!histogramer->Open()) {
    fLog->Warning(
        "CupDAQManager::TF_Histogramer",
        "Can\'t open histogramer root file %s, histogramer will be ended",
        fHistFilename.Data());
    return;
  }

  // booking histograms
  histogramer->Book();
  histogramer->Update();

  int eventnumber = 0;
  int ntotalmonitoredevent = 0;

  double mfrac = 0.3; // monitoring fraction: 30%

  double perror = 0;
  double integral = 0;

  TStopwatch sw;
  sw.Start();

  fBenchmark->Start("Histogramer");
  while (true) {
    // for emergent exit
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) break;
    if (fBuildStatus == ENDED) {
      if (fBuiltEventBuffer2.empty()) break;
    }

    BuiltEvent * builtevent = nullptr;

    int size = fBuiltEventBuffer2.size();
    if (size > 0) { builtevent = fBuiltEventBuffer2.popfront(); }

    if (builtevent) {
      eventnumber = builtevent->GetEventNumber();
      if (gRandom->Rndm() < mfrac) {
        histogramer->Fill(builtevent);
        ntotalmonitoredevent += 1;
      }
      delete builtevent;
    }

    // update histogramer every 1s
    double elapsetime = sw.RealTime();
    sw.Continue();
    if (elapsetime >= 1) {
      sw.Start(true);
      histogramer->Update();
    }

    ThreadSleep(fHistSleep, perror, integral, size);
  }
  fBenchmark->Stop("Histogramer");

  histogramer->Close();
  delete histogramer;

  fHistogramerEnded = true;

  fLog->Info("CupDAQManager::TF_Histogramer",
             "total monitored event = %d (%.2f%%)", ntotalmonitoredevent,
             100. * ntotalmonitoredevent / double(eventnumber));
  fLog->Info("CupDAQManager::TF_Histogramer", "online histogramer ended");
}
