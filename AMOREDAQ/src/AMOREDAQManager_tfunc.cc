#include "AMOREDAQ/AMOREDAQManager.hh"

void AMOREDAQManager::TF_ReadData_AMORE()
{
  fReadStatus = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("Exited by exit command before starting");
    return;
  }

  const int nadc = GetEntries();
  if (nadc <= 0) {
    ERROR("No ADC module included in the configuration");
    fReadStatus = ERROR;
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  INFO("Reading data from ADCs started.");

  auto * adc0 = static_cast<AbsADC *>(fCont[0]);
  std::vector<int> currentBCounts(nadc);

  double sleepError = 0.0;
  double sleepIntegral = 0.0;
  const int kTargetBlocks = 4;

  bool isFlushingData = false;

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
      for (int i = 0; i < nadc; ++i) {
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
        for (std::size_t i = 0; i < nadc; ++i) {
          fRemainingBCount[i] = currentBCounts[i] - fMinimumBCount;
        }
      }

      fTotalReadDataSize += static_cast<double>(nadc) * fMinimumBCount * kKILOBYTES;
      nBlocksProcessed = 1;
    }

    int nAvailableBlocks = (minBCount / fMinimumBCount) - nBlocksProcessed;
    ThreadSleep(fReadSleep, sleepError, sleepIntegral, nAvailableBlocks, kTargetBlocks);
  }

  if (fReadStatus != ERROR) { fReadStatus = ENDED; }
  INFO("Reading data from ADCs ended");
}

void AMOREDAQManager::TF_StreamData()
{
  fStreamStatus = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("Exited by exit command before starting");
    return;
  }

  const int nadc = GetEntries();

  INFO("Streaming data started.");

  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }

    for (int i = 0; i < nadc; ++i) {
      auto * adc = static_cast<AbsADC *>(fCont[i]);
      auto * conf = static_cast<AMOREADCConf *>(adc->GetConfig());

      auto chunkdata = adc->Bpop_front();
      if (!chunkdata) { continue; }

      unsigned char * data = chunkdata->data;
      int ndp = kKILOBYTES * chunkdata->size / 64;

      fFIFOs[i]->PushChunk(data, ndp, conf);
    }
  }

  INFO("Streaming data ended");
}
