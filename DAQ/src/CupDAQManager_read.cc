#include <thread>

#include "DAQ/CupDAQManager.hh"
#include "DAQUtils/ELog.hh"

void CupDAQManager::TF_ReadData()
{
  fReadStatus.store(READY);

  if (!WaitRunState(fRunStatus, RUNSTATE::kRUNNING, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }

  INFO("started");

  if (fTriggerMode == TRIGGER::GLOBAL) { ReadData_GLT(); }
  else { ReadData_MOD(); }

  fReadStatus.store(ENDED);

  INFO("ended");
}

void CupDAQManager::ReadData_GLT()
{
  const int nadc_int = GetEntries();
  if (nadc_int <= 0) {
    ERROR("no ADC modules in ReadData_GLT");
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  const std::size_t nadc = static_cast<std::size_t>(nadc_int);
  std::vector<int> bcounts(nadc);

  auto * theADC = static_cast<AbsADC *>(fCont[0]);

  double perror = 0.0;
  double integral = 0.0;
  bool endsleep = false;
  std::chrono::time_point<std::chrono::steady_clock> end_run_start_time;

  std::unique_lock<std::mutex> mlock(fMonitorMutex, std::defer_lock);

  fReadStatus.store(RUNNING);

  while (true) {
    if (fDoExit.load() || RUNSTATE::CheckError(fRunStatus)) { break; }

    if (RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDING) && !endsleep) {
      INFO("waiting for reading remaining data in ADCs");
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      endsleep = true;
      end_run_start_time = std::chrono::steady_clock::now();
    }

    int bcount = ReadBCountMin(bcounts.data());
    if (bcount < 0) {
      RUNSTATE::SetError(fRunStatus);
      break;
    }

    if (endsleep) {
      if (bcount < fMinimumBCount) {
        INFO("no more data in ADCs [bcount=%d]", bcount);
        break;
      }
      auto current_time = std::chrono::steady_clock::now();
      std::chrono::duration<double> elapsed = current_time - end_run_start_time;
      if (elapsed.count() > 5.0) {
        WARNING("timeout (5s) reached while reading remaining data, force exit");
        break;
      }
    }

    int n = bcount / fMinimumBCount;
    int size = n;
    if (n > 16) { n = 16; }
    bcount = n * fMinimumBCount;

    if (bcount > 0) {
      StartBenchmark("ReadData");

      for (int i = 0; i < nadc_int; ++i) {
        if (ReadADCData(i, bcount) < 0) {
          RUNSTATE::SetError(fRunStatus);
          break;
        }
      }

      if (RUNSTATE::CheckError(fRunStatus)) { break; }

      mlock.lock();
      fTriggerNumber = theADC->GetCurrentTrgNumber();
      fCurrentTime = theADC->GetCurrentTime();
      fTriggerTime = fCurrentTime;
      for (std::size_t i = 0; i < nadc; ++i) {
        fRemainingBCount[i] = bcounts[i] - bcount;
      }
      mlock.unlock();

      fTotalReadDataSize += static_cast<double>(nadc) * bcount * kKILOBYTES;

      StopBenchmark("ReadData");
    }

    ThreadSleep(fReadSleep, perror, integral, size, 16);
  }
}

void CupDAQManager::ReadData_MOD() {}
