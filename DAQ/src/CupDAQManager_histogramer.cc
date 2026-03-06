#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <string>

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

  if (fHistFilename.empty()) {
    std::string filename;

    const char * rawdata_dir_env = std::getenv("RAWDATA_DIR");

    char run_str[16];
    std::snprintf(run_str, sizeof(run_str), "%06d", fRunNumber);

    const char * adc_name = GetADCName(fADCType);

    if (!rawdata_dir_env) {
      WARNING("variable RAWDATA_DIR is not set");

      filename = std::string("hist_") + adc_name + "_" + run_str + ".root";
    }
    else {

      namespace fs = std::filesystem;
      fs::path dirname = fs::path(rawdata_dir_env) / "HIST" / run_str;

      // 4. Safe directory creation (fixes the 'test -d' shell return code bug)
      if (!fs::exists(dirname)) {
        fs::create_directories(dirname);
        INFO("%s created", dirname.c_str());
      }
      else {
        INFO("%s already exist", dirname.c_str());
      }

      // Combine directory and filename naturally
      fs::path full_path = dirname / (std::string("hist_") + adc_name + "_" + run_str + ".root");
      filename = full_path.string();
    }

    histogramer->SetFilename(filename.c_str());
    fHistFilename = filename;
  }

  if (!histogramer->Open()) {
    WARNING("cannot open histogramer root file %s, histogramer will be ended",
            fHistFilename.c_str());
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
