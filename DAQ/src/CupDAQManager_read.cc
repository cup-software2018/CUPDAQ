#include "DAQ/CupDAQManager.hh"

void CupDAQManager::TF_ReadData()
{
  fReadStatus = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }
  INFO("reading data from ADCs started");

  if (fTriggerMode == TRIGGER::GLOBAL) { ReadData_GLT(); }
  else {
    ReadData_MOD();
  }

  if (fReadStatus != ERROR) { fReadStatus = ENDED; }
  INFO("reading data from ADCs ended");
}

void CupDAQManager::ReadData_GLT()
{
  const int nadc_int = GetEntries();
  if (nadc_int <= 0) {
    ERROR("no ADC modules in ReadData_GLT");
    fReadStatus = ERROR;
    RUNSTATE::SetError(fRunStatus);
    return;
  }
  const std::size_t nadc = static_cast<std::size_t>(nadc_int);

  auto * theADC = static_cast<AbsADC *>(fCont[0]);
  std::vector<int> bcounts(nadc);

  double perror = 0.0;
  double integral = 0.0;

  bool endsleep = false;

  std::unique_lock<std::mutex> mlock(fMonitorMutex, std::defer_lock);

  fReadStatus = RUNNING;
  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }

    if (RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDED) && !endsleep) {
      INFO("waiting for reading remaining data in ADCs");
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      endsleep = true;
    }

    int bcount = ReadBCountMin(bcounts.data());
    if (bcount < 0) {
      RUNSTATE::SetError(fRunStatus);
      fReadStatus = ERROR;
      break;
    }
    if (endsleep && fDoEndRun && bcount < fMinimumBCount) {
      INFO("no more data in ADCs [bcount=%d]", bcount);
      break;
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
          fReadStatus = ERROR;
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
