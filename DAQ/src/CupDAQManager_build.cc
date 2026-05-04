#include <cstring>
#include <memory>

#include "DAQ/CupDAQManager.hh"
#include "DAQUtils/ELog.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"

void CupDAQManager::TF_BuildEvent()
{
  fBuildStatus.store(READY);

  if (!WaitRunState(fRunStatus, RUNSTATE::kRUNNING, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }

  auto adctype = static_cast<ADC::TYPE>(fADCType % 10);
  auto * conf = (fConfigList != nullptr) ? fConfigList->GetSTRGConfig(adctype) : nullptr;

  if (fSoftTrigger != nullptr) {
    if (conf != nullptr) { fSoftTrigger->SetConfig(conf); }
    if (fSoftTrigger->IsEnabled()) {
      fSoftTrigger->SetMode(fADCMode);
      fSoftTrigger->InitTrigger();
    }
  }

  INFO("started");

  if (fDAQType == DAQ::MERGER) { MergeEvent(); }
  else {
    if (fTriggerMode == TRIGGER::GLOBAL) { BuildEvent_GLT(); }
    else if (fTriggerMode == TRIGGER::MODULE) {
      BuildEvent_MOD();
    }
    else {
      BuildEvent_SLF();
    }
  }

  fBuildStatus.store(ENDED);

  // sort already ended, clear fADCRawBuffers
  for (auto * buffer : fADCRawBuffers) {
    if (buffer) { buffer->clear(); }
  }

  INFO("ended");
}

void CupDAQManager::BuildEvent_GLT()
{
  std::size_t nadc = static_cast<std::size_t>(GetEntries());

  std::vector<ADCHeader *> header(nadc);
  std::vector<int> sanity(nadc);
  std::vector<unsigned int> trgnum(nadc);
  std::vector<unsigned long> trgtime(nadc);

  double perror = 0;
  double integral = 0;

  std::unique_lock<std::mutex> mlock(fMonitorMutex, std::defer_lock);

  if (fADCRawBuffers.empty()) {
    ERROR("BuildEvent_GLT called with empty fADCRawBuffers");
    return;
  }

  auto ** buffers = fADCRawBuffers.data();

  fBuildStatus.store(RUNNING);

  while (true) {
    if (fDoExit.load() || RUNSTATE::CheckError(fRunStatus)) { break; }

    if (fSortStatus.load() == ENDED) {
      std::size_t nmod = 0;
      for (std::size_t i = 0; i < nadc; i++) {
        auto * adceventbuffer = buffers[i];
        if (adceventbuffer == nullptr || adceventbuffer->empty()) { continue; }
        nmod += 1;
      }
      if (nmod < nadc) {
        INFO("all ADC event buffers are empty, exit");
        break;
      }
    }

    int totalsize = 0;
    std::size_t nmod = 0;

    for (std::size_t i = 0; i < nadc; i++) {
      auto * adceventbuffer = buffers[i];
      if (adceventbuffer == nullptr) { continue; }

      totalsize += static_cast<int>(adceventbuffer->size());

      auto * adcevent = adceventbuffer->front_ptr();
      if (adcevent == nullptr) { continue; }

      header[i] = adcevent->GetADCHeader();
      nmod += 1;
    }

    StartBenchmark("BuildEvent");

    if (nmod == nadc) {
      if (nadc > 0) { totalsize /= static_cast<int>(nadc); }

      CheckEventSanity(header.data(), trgnum.data(), trgtime.data(), sanity.data());

      if (fADCType == ADC::SADCS || fADCType == ADC::SADCT) {
        bool isnull = false;
        for (std::size_t i = 0; i < nadc; i++) {
          if (sanity[i] == -1) {
            isnull = true;
            break;
          }
        }
        if (isnull) {
          INFO("SADC null event, will be skipped");
          for (std::size_t i = 0; i < nadc; i++) {
            auto * adceventbuffer = buffers[i];
            if (adceventbuffer == nullptr) { continue; }
            while (!adceventbuffer->empty()) {
              adceventbuffer->pop_front();
            }
          }
          continue;
        }
      }

      auto builtevent = std::make_shared<BuiltEvent>();
      builtevent->SetDAQID(fDAQID);

      int nerror = 0;
      for (std::size_t i = 0; i < nadc; i++) {
        auto * adc = static_cast<AbsADC *>(fCont[i]);
        auto * adceventbuffer = buffers[i];
        if (adceventbuffer == nullptr) { continue; }

        int s = sanity[i];
        if (s == 1) {
          ERROR("ADC header is corrupted [mid=%d]", adc->GetSID());
          nerror += 1;
        }
        else if (s == 2) {
          ERROR("trigger number mismatch in ADC [mid=%d]", adc->GetSID());
          nerror += 1;
        }
        else if (s == 3) {
          ERROR("local trigger time mismatch in ADC [mid=%d]", adc->GetSID());
          nerror += 1;
        }

        auto adcrawOpt = adceventbuffer->pop_front();
        if (!adcrawOpt) {
          ERROR("ADC buffer empty while building event [mid=%d]", adc->GetSID());
          nerror += 1;
          continue;
        }

        AbsADCRaw * adcraw_ptr = adcrawOpt->release();
        builtevent->Add(adcraw_ptr);
      }

      if (nerror > 0) {
        RUNSTATE::SetError(fRunStatus);
        break;
      }

      bool istriggered = true;
      if (!fDoSendEvent && fSoftTrigger != nullptr && fSoftTrigger->IsEnabled() &&
          !fSoftTrigger->DoTrigger(builtevent.get())) {
        istriggered = false;
      }

      if (istriggered) {
        mlock.lock();
        fNBuiltEvent += 1;
        builtevent->SetEventNumber(fNBuiltEvent);
        mlock.unlock();

        fBuiltEventBuffer1.push_back(builtevent);
        if (fDoHistograming) { fBuiltEventBuffer2.push_back(builtevent); }
      }

      totalsize -= 1;
    }

    StopBenchmark("BuildEvent");
    ThreadSleep(fBuildSleep, perror, integral, totalsize);
  }
}

void CupDAQManager::MergeEvent()
{
  double perror = 0;
  double integral = 0;
  bool recvEnded = false;
  std::size_t ndaq = fRecvEventBuffers.size();

  if (ndaq == 0) {
    ERROR("MergeEvent called with empty fRecvEventBuffers");
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  std::unique_lock<std::mutex> mlock(fMonitorMutex, std::defer_lock);

  fBuildStatus.store(RUNNING);

  while (true) {
    if (fDoExit.load() || RUNSTATE::CheckError(fRunStatus)) { break; }

    int totalsize = 0;
    std::size_t nready = 0;
    int nerror = 0;
    bool isFirst = true;
    unsigned int refTrigNum = 0;
    unsigned long refTrigTime = 0;

    for (auto & [daqId, buf] : fRecvEventBuffers) {
      totalsize += static_cast<int>(buf->size());
      auto * ev = buf->front_ptr();
      if (ev != nullptr) {
        nready += 1;
        if (isFirst) {
          refTrigNum = ev->GetTriggerNumber();
          refTrigTime = ev->GetTriggerTime();
          isFirst = false;
        }
        else {
          if (ev->GetTriggerNumber() != refTrigNum) {
            ERROR("MergeEvent: TriggerNumber mismatch [daqId=%d] got=%u expected=%u", daqId,
                  ev->GetTriggerNumber(), refTrigNum);
            nerror += 1;
          }
          if (ev->GetTriggerTime() != refTrigTime) {
            ERROR("MergeEvent: TriggerTime mismatch [daqId=%d] got=%lu expected=%lu", daqId,
                  ev->GetTriggerTime(), refTrigTime);
            nerror += 1;
          }
        }
      }
    }

    if (nerror > 0) {
      RUNSTATE::SetError(fRunStatus);
      break;
    }

    if (!recvEnded && fRecvStatus.load() == ENDED) { recvEnded = true; }
    if (recvEnded && nready < ndaq) { 
      INFO("all Received event buffers are empty, exit");
      break; 
    }

    StartBenchmark("BuildEvent");

    if (nready == ndaq) {
      totalsize /= static_cast<int>(ndaq);

      mlock.lock();
      fTriggerNumber = refTrigNum;
      fTriggerTime = refTrigTime;
      mlock.unlock();

      auto builtevent = std::make_shared<BuiltEvent>();
      for (auto & [daqId, buf] : fRecvEventBuffers) {
        auto ev = *buf->pop_front();
        int nadc = ev->GetEntries();
        for (int i = 0; i < nadc; i++) {
          switch (fADCMode) {
            case ADC::SMODE: {
              auto * adc = static_cast<SADCRawEvent *>(ev->RemoveAt(i));
              if (adc) {
                fTotalRawDataSize += adc->GetRawDataSize();
                builtevent->Add(adc);
              }
              break;
            }
            case ADC::FMODE: {
              auto * adc = static_cast<FADCRawEvent *>(ev->RemoveAt(i));
              if (adc) {
                fTotalRawDataSize += adc->GetRawDataSize();
                builtevent->Add(adc);
              }
              break;
            }
            default: break;
          }
        }
      }

      bool istriggered = true;
      if (!fDoSendEvent && fSoftTrigger != nullptr && fSoftTrigger->IsEnabled() &&
          !fSoftTrigger->DoTrigger(builtevent.get())) {
        istriggered = false;
      }

      if (istriggered) {
        mlock.lock();
        fNBuiltEvent += 1;
        builtevent->SetEventNumber(fNBuiltEvent);
        mlock.unlock();

        fBuiltEventBuffer1.push_back(builtevent);
        if (fDoHistograming) { fBuiltEventBuffer2.push_back(builtevent); }
      }

      totalsize -= 1;
    }

    StopBenchmark("BuildEvent");
    ThreadSleep(fBuildSleep, perror, integral, totalsize);
  }

  // data server already ended, clear fRecvEventBuffers
  for (auto & [daqId, buf] : fRecvEventBuffers) {
    if (buf) { buf->clear(); }
  }

  fBuildStatus.store(ENDED);
}

void CupDAQManager::BuildEvent_MOD() {}

void CupDAQManager::BuildEvent_SLF() {}

void CupDAQManager::CheckEventSanity(ADCHeader ** header, unsigned int * tn, unsigned long * tt,
                                     int * error)
{
  std::size_t nadc = static_cast<std::size_t>(GetEntries());
  std::memset(error, 0, nadc * sizeof(int));

  if (nadc == 1) {
    if (header[0]->GetError()) { error[0] = 1; }
    else if (header[0]->GetMID() == 0) {
      error[0] = -1;
    }
    return;
  }

  for (std::size_t i = 0; i < nadc; i++) {
    tn[i] = header[i]->GetLocalTriggerNumber();
    tt[i] = header[i]->GetLocalTriggerTime();

    if (header[i]->GetMID() == 0) { error[i] = -1; }
    else if (header[i]->GetError()) {
      error[i] = 1;
    }
    else if (i > 0) {
      if (tn[i] != tn[0]) {
        DEBUG("trgnum mismatch: ref=%u vs [mid=%d]=%u", tn[0], header[i]->GetMID(), tn[i]);
        error[i] = 2;
      }
      else if (tt[i] != tt[0]) {
        DEBUG("trgtime mismatch: ref=%lu vs [mid=%d]=%lu", tt[0], header[i]->GetMID(), tt[i]);
        error[i] = 3;
      }
    }
  }
}