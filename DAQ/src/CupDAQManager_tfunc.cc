#include <chrono>
#include <cstdio>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <vector>
#include <zmq.hpp>

#include "DAQ/CupDAQManager.hh"
#include "DAQUtils/ELog.hh"
#include "OnlConsts/onlconsts.hh"

void CupDAQManager::TF_RunManager()
{
  INFO("run manager started");

  if (!WaitRunState(fRunStatus, RUNSTATE::kCONFIGURED, fDoExit)) {
    WARNING("exited by error state");
    return;
  }

  if (fDAQType == DAQ::TCBDAQ) {
    fTCB->StartTrigger();
    time(&fStartDatime);
    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
  }
  else if (fDAQType == DAQ::STDDAQ) {
    StartTrigger();
    time(&fStartDatime);
    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
  }

  bool iendrun = false;
  std::unique_lock<std::mutex> mlock(fMonitorMutex, std::defer_lock);

  while (true) {
    if (fDoExit.load()) { break; }

    if (IsForcedEndRunFile()) {
      INFO("daq will be ended by ENDRUN command");
      fDoEndRun.store(true);
      break;
    }

    if (fSetNEvent > 0 || fSetDAQTime > 0) {
      mlock.lock();
      int triggernumber = fTriggerNumber;
      double triggertime = fTriggerTime / kDONESECOND;
      mlock.unlock();

      if (fSetNEvent > 0 && triggernumber > fSetNEvent) { iendrun = true; }
      if (fSetDAQTime > 0 && triggertime > fSetDAQTime) { iendrun = true; }
      if (iendrun) {
        INFO("daq will be ended by preset condition");
        fDoEndRun.store(true);
        break;
      }
    }

    if (fDoSplitOutputFile.load() && !fDoSendEvent) {
      if (!OpenNewOutputFile()) { RUNSTATE::SetError(fRunStatus); }
      fDoSplitOutputFile.store(false);
    }

    bool runstate = RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDED) ||
                    RUNSTATE::CheckError(fRunStatus) || fDoExit.load();
    if (runstate) { break; }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  if (fDAQType == DAQ::TCBDAQ) {
    fTCB->StopTrigger();
    time(&fEndDatime);
    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
  }
  else if (fDAQType == DAQ::STDDAQ) {
    StopTrigger();
    time(&fEndDatime);
    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
  }

  WaitRunState(fRunStatus, RUNSTATE::kPROCENDED, fDoExit);
  INFO("all processes ended");

  PrintDAQSummary();
  INFO("run manager ended");
}

void CupDAQManager::TF_TriggerMon()
{
  if (!WaitRunState(fRunStatus, RUNSTATE::kRUNNING, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }

  INFO("started");

  unsigned int dummynevent = 0;
  double dummytime = 0.0;

  std::unique_lock<std::mutex> mlock(fMonitorMutex, std::defer_lock);

  auto start_time = std::chrono::steady_clock::now();

  auto get_hms = [](double sec)
  {
    if (sec < 0) sec = 0;

    int h = static_cast<int>(sec) / 3600;
    int m = (static_cast<int>(sec) % 3600) / 60;
    double s = std::fmod(sec, 60.0);

    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%04.1f", h, m, s);
    return std::string(buf);
  };

  while (true) {
    auto current_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = current_time - start_time;
    double elapsetime = elapsed.count();

    if (elapsetime > fTriggerMonTime) {
      start_time = std::chrono::steady_clock::now();

      mlock.lock();
      int triggernumber = fTriggerNumber;
      double triggertime = fTriggerTime / kDONESECOND;
      mlock.unlock();

      double dtime = triggertime - dummytime;
      double dnevent = triggernumber - dummynevent;

      double insrate = dtime > 0.0 ? dnevent / dtime : 0.0;
      double accrate = triggertime > 0.0 ? triggernumber / triggertime : 0.0;

      STATS("%5d events triggered [%7d | %7d / %5.1f(%5.1f) Hz / %s]", static_cast<int>(dnevent),
            triggernumber, fNBuiltEvent, insrate, accrate, get_hms(triggertime).c_str());

      dummynevent = static_cast<unsigned int>(triggernumber);
      dummytime = triggertime;

      fCurrentTriggerRate = insrate;
    }

    bool runstate = RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDED) ||
                    RUNSTATE::CheckError(fRunStatus) || fDoExit.load();
    if (runstate) { break; }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  INFO("ended");
}

void CupDAQManager::TF_DebugMon()
{
  if (!WaitRunState(fRunStatus, RUNSTATE::kCONFIGURED, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }

  const int nadc_int = GetEntries();
  const double debugmontime = 1.0;

  INFO("started");

  auto start_time = std::chrono::steady_clock::now();

  while (true) {
    auto current_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = current_time - start_time;
    double elapsetime = elapsed.count();

    if (elapsetime > debugmontime) {
      start_time = std::chrono::steady_clock::now();

      std::string adcbcountsize;
      std::string adcbufsize;
      std::string sortbufsize;

      char buf[32];

      {
        std::lock_guard<std::mutex> lock(fMonitorMutex);

        for (int i = 0; i < nadc_int; ++i) {
          std::snprintf(buf, sizeof(buf), "%5d ", fRemainingBCount[i]);
          adcbcountsize += buf;

          auto * adc = static_cast<AbsADC *>(fCont[i]);
          std::snprintf(buf, sizeof(buf), "%5d ", adc->Bsize());
          adcbufsize += buf;

          auto * modraw = fADCRawBuffers.at(static_cast<std::size_t>(i));
          std::snprintf(buf, sizeof(buf), "%5d ", static_cast<int>(modraw->size()));
          sortbufsize += buf;
        }
      }

      DEBUG("ADC bcount size: %s", adcbcountsize.c_str());
      DEBUG("ADC buffer size: %s", adcbufsize.c_str());
      DEBUG("sorting buffer size: %s", sortbufsize.c_str());
      DEBUG("building buffer size: %5zu %5zu", fBuiltEventBuffer1.size(),
            fBuiltEventBuffer2.size());
      DEBUG("%.3f(r) %.3f(s) %.3f(b) %.3f(w)", fReadSleep / 1000.0, fSortSleep / 1000.0,
            fBuildSleep / 1000.0, fWriteSleep / 1000.0);
    }

    bool runstate = RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDED) ||
                    RUNSTATE::CheckError(fRunStatus) || fDoExit.load();
    if (runstate) { break; }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  INFO("ended");
}

void CupDAQManager::TF_ShrinkToFit()
{
  if (!WaitRunState(fRunStatus, RUNSTATE::kRUNNING, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }

  INFO("started");

  auto start_time = std::chrono::steady_clock::now();

  const int nadc_int = GetEntries();

  while (true) {
    auto current_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = current_time - start_time;
    double elapsetime = elapsed.count();
    if (elapsetime >= 10.0) {
      start_time = std::chrono::steady_clock::now();

      for (int i = 0; i < nadc_int; ++i) {
        auto * adc = static_cast<AbsADC *>(fCont[i]);
        adc->Bshrink_to_fit();

        auto * modbuffer = fADCRawBuffers.at(static_cast<std::size_t>(i));
        modbuffer->shrink_to_fit();
      }

      fBuiltEventBuffer1.shrink_to_fit();
      fBuiltEventBuffer2.shrink_to_fit();

      if (!fRecvEventBuffers.empty()) {
        for (auto & [daqId, buf] : fRecvEventBuffers) {
          buf->shrink_to_fit();
        }
      }

      if (fVerboseLevel > 0) { INFO("shrink buffer memory to fit"); }
    }

    bool runstate = RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDED) ||
                    RUNSTATE::CheckError(fRunStatus) || fDoExit.load();
    if (runstate) { break; }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  INFO("ended");
}

void CupDAQManager::TF_SplitOutput(bool ontcb)
{
  std::atomic<unsigned long> & current_run_status = ontcb ? fRunStatusTCB : fRunStatus;
  std::atomic<bool> & current_do_exit = ontcb ? fDoExitTCB : fDoExit;

  if (!WaitRunState(current_run_status, RUNSTATE::kRUNNING, current_do_exit)) { return; }

  INFO("started%s", ontcb ? " (TCB)" : "");

  auto start_time = std::chrono::steady_clock::now();

  while (true) {
    auto current_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = current_time - start_time;
    double elapsetime = elapsed.count();

    if (elapsetime >= fOutputSplitTime) {
      start_time = std::chrono::steady_clock::now();
      fDoSplitOutputFile.store(true);
      fSubRunNumber.fetch_add(1);

      if (fVerboseLevel >= 1) { INFO("output file will be split"); }
    }

    if (ontcb) {
      if (RUNSTATE::CheckState(fRunStatusTCB, RUNSTATE::kRUNENDED) ||
          RUNSTATE::CheckError(fRunStatusTCB) || fDoExitTCB.load()) {
        break;
      }
    }
    else {
      if (RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDED) ||
          RUNSTATE::CheckError(fRunStatus) || fDoExit.load()) {
        break;
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  INFO("ended");
}