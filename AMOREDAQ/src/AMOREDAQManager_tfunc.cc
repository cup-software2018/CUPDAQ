#include "AMOREDAQ/AMOREDAQManager.hh"

void AMOREDAQManager::TF_ReadData_AMORE()
{
  fReadStatus = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("Exited by exit command before starting");
    return;
  }

  const int nADCInput = GetEntries();
  if (nADCInput <= 0) {
    ERROR("No ADC module included in the configuration");
    fReadStatus = ERROR;
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  INFO("Reading data from ADCs started. [Rate: 100kHz, Block: 4MB]");

  const std::size_t nADC = static_cast<std::size_t>(nADCInput);
  auto * adc0 = static_cast<AbsADC *>(fCont[0]);
  std::vector<int> currentBCounts(nADC);

  double sleepError = 0.0;
  double sleepIntegral = 0.0;
  bool isFlushingData = false;

  const int kTargetBlocks = 4;

  fReadStatus = RUNNING;

  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }

    if (RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDED) && !isFlushingData) {
      INFO("Run ended. Waiting 1s for remaining data...");
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      isFlushingData = true;
    }

    int minBCount = ReadBCountMin(currentBCounts.data());

    if (minBCount < 0) {
      ERROR("Failed to read buffer count");
      RUNSTATE::SetError(fRunStatus);
      fReadStatus = ERROR;
      break;
    }

    if (isFlushingData && fDoEndRun && minBCount < fMinimumBCount) {
      INFO("No more data [minBCount=%d]. Stop.", minBCount);
      break;
    }

    int nBlocksProcessed = 0;
    if (minBCount >= fMinimumBCount) {
      for (int i = 0; i < nADCInput; ++i) {
        if (ReadADCData(i, fMinimumBCount) < 0) {
          ERROR("Reading failed at ADC #%d", i);
          RUNSTATE::SetError(fRunStatus);
          fReadStatus = ERROR;
          break;
        }
      }

      if (fReadStatus == ERROR || RUNSTATE::CheckError(fRunStatus)) { break; }

      {
        std::lock_guard<std::mutex> lock(fMonitorMutex);
        fTriggerNumber = adc0->GetCurrentTrgNumber();
        fCurrentTime = adc0->GetCurrentTime();
        fTriggerTime = fCurrentTime;
        for (std::size_t i = 0; i < nADC; ++i) {
          fRemainingBCount[i] = currentBCounts[i] - fMinimumBCount;
        }
      }

      fTotalReadDataSize += static_cast<double>(nADC) * fMinimumBCount * kKILOBYTES;
      nBlocksProcessed = 1;
    }

    int nAvailableBlocks = (minBCount / fMinimumBCount) - nBlocksProcessed;
    ThreadSleep(fReadSleep, sleepError, sleepIntegral, nAvailableBlocks, kTargetBlocks);
  }

  if (fReadStatus != ERROR) { fReadStatus = ENDED; }
  INFO("Reading data from ADCs ended");
}