#include "TRandom3.h"

#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5FADCEvent.hh"

#include "AMOREDAQ/AMOREDAQManager.hh"
#include "RawObjs/EventInfo.hh"
#include "RawObjs/AChannel.hh"
#include "RawObjs/AChannelData.hh"
#include "RawObjs/FChannel.hh"
#include "RawObjs/FChannelData.hh"
#include "AMOREAlgs/AMOREChunkFIFO.hh"

void AMOREDAQManager::TF_ReadData_AMORE()
{
  fReadStatus = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("Exited by exit command before starting");
    return;
  }

  const int nadc_int = GetEntries();
  if (nadc_int <= 0) {
    ERROR("no ADC modules in TF_ReadData_AMORE");
    fReadStatus = ERROR;
    RUNSTATE::SetError(fRunStatus);
    return;
  }
  const std::size_t nadc = static_cast<std::size_t>(nadc_int);

  INFO("Reading data from ADCs started.");

  auto * adc0 = static_cast<AbsADC *>(fCont[0]);
  std::vector<int> currentBCounts(nadc);

  double sleepError = 0.0;
  double sleepIntegral = 0.0;

  bool endsleep = false;

  fReadStatus = RUNNING;

  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }

    if (RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDED) && !endsleep) {
      INFO("waiting for reading remaining data in ADCs");
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      endsleep = true;
    }

    int bcount = ReadBCountMin(currentBCounts.data());
    if (bcount < 0) {
      ERROR("Failed to read buffer count [bcount=%d]", bcount);
      RUNSTATE::SetError(fRunStatus);
      fReadStatus = ERROR;
      break;
    }

    //if (endsleep && fDoEndRun && bcount < fMinimumBCount) {
    if (endsleep && bcount < fMinimumBCount) {
      INFO("no more data in ADCs [bcount=%d]", bcount);
      break;
    }

    int n = bcount / fMinimumBCount;
    int size = n;
    if (n > 16) { n = 16; }

    bcount = n * fMinimumBCount;

    if (bcount > 0) {
      for (int i = 0; i < nadc_int; ++i) {
        int readStat = ReadADCData(i, bcount);
        if (readStat < 0) {
          ERROR("Reading failed at ADC #%d [bcount=%d, ADCData=%d]", i + 1, bcount, readStat);
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
          fRemainingBCount[i] = currentBCounts[i] - bcount;
        }
      }

      fTotalReadDataSize += static_cast<double>(nadc) * bcount * kKILOBYTES;
    }

    ThreadSleep(fReadSleep, sleepError, sleepIntegral, size, 16);
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
  fTrigStatus[n] = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("Exited by exit command before starting");
    return;
  }

  auto * adc = static_cast<AbsADC *>(fCont[n]);
  auto * conf = static_cast<AMOREADCConf *>(adc->GetConfig());

  const int ndp = conf->RL();
  const int tail = ndp - conf->DLY();

  INFO("software trigger for AMOREADC[sid=%d] started.", conf->SID());
  fTrigStatus[n] = RUNNING;

  auto & fifo = fFIFOs[n];

  const int nch = kNCHAMOREADC;
  std::vector<unsigned short> adcval(nch);
  unsigned long currenttime;
  unsigned long lasttime;

  bool isFirstSample = true;

  std::vector<unsigned long> dumpTime(ndp);
  std::vector<unsigned short *> dumpADC(nch);
  for (int i = 0; i < nch; ++i)
    dumpADC[i] = new unsigned short[ndp];

  std::vector<std::uint16_t> phonon(ndp);
  std::vector<std::uint16_t> photon(ndp);


  std::vector<int> ndt(nch);
  std::vector<bool> istriggered(nch);

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
            WARNING("[NS ERROR] Overlap/Jitter! Last: %lu Now: %lu Gap: %lu ns", lasttime,
                    currenttime, delta);
          }
          else {
            unsigned long lost = (delta / fTimeDelta) - 1;
            if (lost != 0) {
              WARNING("[NS ERROR] missing samples! Last: %lu Now: %lu Gap: %lu ns | Lost: %lu",
                      lasttime, currenttime, delta, lost);
            }
          }
        }

        // success poping a sample from fifo, triggering
        for (int i = 0; i < nch; ++i) {
          if (!conf->TRGON(i)) continue;

          if (istriggered[i]) {
            ndt[i] += 1;
            if (ndt[i] > tail + conf->DT(i)) {
              istriggered[i] = false;
              ndt[i] = 0;
            }
            continue;
          }
          if (gRandom->Rndm() < 1e-06) {
            INFO("Channel %02d [sid=%d] is triggered", i, adc->GetSID());
            fifo->DumpCurrent(dumpADC.data(), dumpTime.data());
            //INFO("DumpCurrent size check: ndp=%d head=%d tail=%d",
            //      ndp, fifo->GetHead(), fifo->GetTail());
            int pid = conf->PID(i);

            Crystal_t xtal;
            xtal.ndp = static_cast<std::uint16_t>(ndp);
            xtal.id = pid/2;
            xtal.ttime = currenttime;
            
            for (int j = 0 ; j < ndp; ++j) {
              phonon[j] = static_cast<std::uint16_t>(dumpADC[i][j]);
              photon[j] = static_cast<std::uint16_t>(dumpADC[i+1][j]);
            }
            xtal.SetWaveforms(phonon.data(), photon.data(), ndp);

            fTriggeredCrystals.push_back(xtal);

            istriggered[i] = true;
          }
        }
      }
      else {
        isFirstSample = false;
      }
      lasttime = currenttime;
    }
    else {
      // or waiting for new chunk
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  INFO("software trigger for AMOREADC[sid=%d] ended.", conf->SID());
  fTrigStatus[n] = ENDED;

  fifo->DumpStat();

  for (int i = 0; i < nch; ++i) {
    delete[] dumpADC[i];
  }
}

bool AMOREDAQManager::HasRunningTrigger() const
{
  for (const auto &st : fTrigStatus) {
    if (st == READY || st == RUNNING) return true;
  }
  return false;
}

void AMOREDAQManager::TF_WriteEvent_AMORE()
{
  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("Exited by exit command before starting");
    return;
  }

  if (!fHDF5File) {
      fHDF5File = new H5DataWriter(fOutputFilename.Data(), 1);
  }
  if (!fHDF5File) {
    ERROR("failed to create H5DataWriter");
    RUNSTATE::SetError(fRunStatus);
    fWriteStatus = ERROR;
    return;
  }

  auto * adc0 = static_cast<AbsADC *>(fCont[0]);
  auto * conf0 = static_cast<AMOREADCConf *>(adc0->GetConfig());
  int ndp = conf0->RL();

  auto *h5event = new H5FADCEvent;
  INFO("writer: after new H5FADCEvent");
  h5event->SetNDP(ndp);
  INFO("writer: after SetNDP");
  INFO("writer: fHDF5File=%p", fHDF5File);
 
  fH5Event = h5event;
 
  INFO("writer: before SetEvent");
  fHDF5File->SetEvent(h5event);
 
  INFO("writer: before Open");
  if (!fHDF5File->Open()) {
  ERROR("can't open hdf5 output file");
  RUNSTATE::SetError(fRunStatus);
  fWriteStatus = ERROR;
  return;
  }
  INFO("writer: after Open");

  EventInfo_t eventinfo{};
  std::vector<FChannel_t> chdata;
  chdata.reserve(2);

  double perror = 0;
  double integral = 0;

  fWriteStatus = RUNNING;
  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) break;

    if (fTriggeredCrystals.empty()) {
      if (!HasRunningTrigger()) break;

      int size_empty = fTriggeredCrystals.size();
      ThreadSleep(fWriteSleep, perror, integral, size_empty);
      continue;
    }

    chdata.clear();

    auto popped = fTriggeredCrystals.pop_front();
    if (!popped) {
      int size_empty = fTriggeredCrystals.size();
      ThreadSleep(fWriteSleep, perror, integral, size_empty);
      continue;
    }

    Crystal_t crystal = std::move(popped.value());
    int ndp_xtal = crystal.ndp;
    INFO("writer: popped crystal");
    INFO("writer: crystal id=%u ndp=%u ttime=%llu",
        crystal.id, crystal.ndp, (unsigned long long)crystal.ttime);
    INFO("writer: before GetWaveforms");
    INFO("writer: before AppendEvent");
 
    if (ndp_xtal > kH5FADCNDPMAX) {
      ERROR("ndp overflow: %d > %d", ndp, kH5AMORENDPMAX);
      RUNSTATE::SetError(fRunStatus);
      fWriteStatus = ERROR;
      break;
    }

    std::uint16_t pn[kH5AMORENDPMAX];
    std::uint16_t pt[kH5AMORENDPMAX];
    crystal.GetWaveforms(pn, pt, ndp_xtal);

    unsigned short phonon_id = static_cast<unsigned short>(2 * crystal.id);
    unsigned short photon_id = static_cast<unsigned short>(2 * crystal.id + 1);

    FChannel_t ch_phonon{};
    ch_phonon.id = phonon_id;
    ch_phonon.SetWaveform(pn, ndp_xtal);
    chdata.push_back(ch_phonon);

    FChannel_t ch_photon{};
    ch_photon.id = photon_id;
    ch_photon.SetWaveform(pt, ndp_xtal);
    chdata.push_back(ch_photon);

    eventinfo.tnum = 0;
    eventinfo.ttime = crystal.ttime;
    eventinfo.ttype = 0;
    eventinfo.nhit = 2;

    INFO("writer: append nhit=%zu ndp=%d", chdata.size(), ndp_xtal);
    herr_t status = h5event->AppendEvent(eventinfo, chdata);
    if (status < 0) {
      ERROR("H5FADCEvent::AppendEvent failed");
      RUNSTATE::SetError(fRunStatus);
      fWriteStatus = ERROR;
      break;
    }

    ++fNBuiltEvent;

    int size = fTriggeredCrystals.size();
    ThreadSleep(fWriteSleep, perror, integral, size);
  }

  if (fHDF5File) {
    fHDF5File->Close();
  }
  if (fWriteStatus != ERROR) { fWriteStatus = ENDED; }
  INFO("Writing events ended");
}