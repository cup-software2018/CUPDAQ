#include "AMOREDAQ/AMOREDAQManager.hh"

void AMOREDAQManager::TF_ReadData_AMORE()
{
  fReadStatus = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("Exited by exit command before starting");
    return;
  }

  const int nadc = GetEntries();

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

  double sleepError = 0.0;
  double sleepIntegral = 0.0;
  const int kTargetChunk = 4;

  INFO("Streaming data started.");
  fStreamStatus = RUNNING;

  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }

    if (fReadStatus == ENDED) {
      int remain = 0;
      for (int i = 0; i < nadc; ++i) {
        auto * adc = static_cast<AbsADC *>(fCont[i]);
        remain += adc->Bsize();
      }
      if (remain == 0) { break; }
    }

    int nTotalChunkinADC = 0;
    for (int i = 0; i < nadc; ++i) {
      auto * adc = static_cast<AbsADC *>(fCont[i]);
      auto * conf = static_cast<AMOREADCConf *>(adc->GetConfig());

      nTotalChunkinADC += adc->Bsize();

      auto chunkdata = adc->Bpop_front();
      if (!chunkdata) { continue; }

      unsigned char * data = chunkdata->data;
      int ndp = kKILOBYTES * chunkdata->size / 64;

      fFIFOs[i]->PushChunk(data, ndp, conf);

      nTotalChunkinADC -= 1;
    }

    int nAvailableChunk = nTotalChunkinADC / nadc;
    ThreadSleep(fReadSleep, sleepError, sleepIntegral, nAvailableChunk, kTargetChunk);
  }

  for (int i = 0; i < nadc; ++i) {
    fFIFOs[i]->Stop();
  }

  fStreamStatus = ENDED;
  INFO("Streaming data ended");
}

void AMOREDAQManager::TF_SWTrigger(int n)
{
  auto * adc = static_cast<AbsADC *>(fCont[n]);
  auto * conf = static_cast<AMOREADCConf *>(adc->GetConfig());

  INFO("software trigger for AMOREADC[%d] started.", conf->SID());

  auto & fifo = fFIFOs[n];

  const int nch = kNCHAMOREADC;
  std::vector<unsigned int> adcval(nch);
  unsigned long currenttime;
  unsigned long lasttime;
  bool isFirstSample = true;

  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }

    if (fStreamStatus == ENDED && fifo->Empty()) { break; }

    int stat = fifo->PopCurrent(adcval.data(), currenttime);
    if (stat == 0) {
      // realtime integrity check
      if (!isFirstSample) {
        unsigned long delta = currenttime - lasttime;
        if (delta != fTimeDelta) {
          if (delta < fTimeDelta) {
            WARNING("[NS ERROR] Overlap/Jitter! Last: %lu Now: %lu Gap: %lu ns", lasttime, currenttime, delta);
          }
          else {
            unsigned long lost = (delta / fTimeDelta) - 1;
            if (lost != 0) {
              WARNING("[NS ERROR] missing samples! Last: %lu Now: %lu Gap: %lu ns | Lost: %lu", lasttime, currenttime, delta, lost);
            }
          }
        }
      }
      else {
        isFirstSample = false;
      }
      lasttime = currenttime;

      // success poping a sample from fifo, triggering
    }
    else {
      // or waiting for new chunk
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  INFO("software trigger for AMOREADC[%d] ended.", conf->SID());
}
