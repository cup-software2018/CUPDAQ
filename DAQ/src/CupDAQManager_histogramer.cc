#include <chrono> // For std::chrono
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <random>
#include <string>

#include "DAQ/CupDAQManager.hh"
#include "DAQUtils/ELog.hh"
#include "OnlHistogramer/AbsHistogramer.hh"
#include "OnlHistogramer/FADCHistogramer.hh"
#include "OnlHistogramer/SADCHistogramer.hh"

void CupDAQManager::TF_Histogramer()
{
  if (!WaitRunState(fRunStatus, RUNSTATE::kRUNNING, fDoExit)) { return; }
  INFO("histogramer started");

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

      if (!fs::exists(dirname)) {
        fs::create_directories(dirname);
        INFO("%s created", dirname.c_str());
      }
      else {
        INFO("%s already exist", dirname.c_str());
      }

      fs::path full_path = dirname / (std::string("hist_") + adc_name + "_" + run_str + ".root");
      filename = full_path.string();
    }

    histogramer->SetFilename(filename.c_str());
    fHistFilename = filename;
  }

  if (!histogramer->Open()) {
    WARNING("cannot start online monitoring server, histogramer will be ended");
    return;
  }

  histogramer->Book();
  histogramer->Update();

  int eventnumber = 0;
  int ntotalmonitoredevent = 0;

  const double max_mfrac = 1.0;
  const double min_mfrac = 0.01;
  double mfrac = max_mfrac;

  const int buffer_safe_size = 10;
  const int buffer_warn_size = 100;
  const int buffer_critical_size = 200;
  const int buffer_emergency_size = 500;

  bool is_emergency_mode = false;
  bool prev_emergency_mode = false;

  double perror = 0.0;
  double integral = 0.0;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dis(0.0, 1.0);

  auto start_time = std::chrono::steady_clock::now();

  fBenchmark->Start("Histogramer");

  while (true) {
    if (fDoExit.load() || RUNSTATE::CheckError(fRunStatus)) { break; }
    if (fBuildStatus.load() == ENDED && fBuiltEventBuffer2.empty()) { break; }

    int size = static_cast<int>(fBuiltEventBuffer2.size());
    int n_to_pop = 1;

    if (size > buffer_emergency_size) { is_emergency_mode = true; }
    else if (size < buffer_warn_size) {
      is_emergency_mode = false;
    }

    if (is_emergency_mode != prev_emergency_mode) {
      if (is_emergency_mode) {
        WARNING("Histogramer [EMERGENCY MODE ON] : Buffer size (%d) exceeded High Watermark!",
                size);
      }
      else {
        INFO("Histogramer [EMERGENCY MODE OFF]: Buffer size (%d) stabilized below Low Watermark.",
             size);
      }
      prev_emergency_mode = is_emergency_mode;
    }

    if (is_emergency_mode) {
      mfrac = 0.0;
      n_to_pop = 10;
    }
    else {
      if (size <= buffer_safe_size) { mfrac = max_mfrac; }
      else if (size <= buffer_warn_size) {
        double ratio =
            static_cast<double>(size - buffer_safe_size) / (buffer_warn_size - buffer_safe_size);
        mfrac = max_mfrac - ratio * (max_mfrac - min_mfrac);
      }
      else if (size <= buffer_critical_size) {
        double ratio = static_cast<double>(size - buffer_warn_size) /
                       (buffer_critical_size - buffer_warn_size);
        mfrac = min_mfrac * (1.0 - ratio);
      }
      else {
        mfrac = 0.0;
        n_to_pop = 1;
      }
    }

    for (int i = 0; i < n_to_pop; ++i) {
      if (fBuiltEventBuffer2.empty()) { break; }

      auto opt = fBuiltEventBuffer2.pop_front();
      if (opt) {
        std::shared_ptr<BuiltEvent> builtevent = opt.value();
        eventnumber = builtevent->GetEventNumber();

        if (mfrac > 0.0 && dis(gen) < mfrac) {
          histogramer->Fill(builtevent.get());
          ntotalmonitoredevent += 1;
        }
      }
    }

    auto current_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = current_time - start_time;
    if (elapsed.count() >= 1.0) {
      start_time = std::chrono::steady_clock::now();
      histogramer->Update();
    }

    if (size < buffer_safe_size) { ThreadSleep(fHistSleep, perror, integral, size); }
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