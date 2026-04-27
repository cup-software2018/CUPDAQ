#include <cstring>
#include <memory>

#include "DAQ/CupDAQManager.hh"
#include "DAQUtils/ELog.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"

void CupDAQManager::TF_BuildEvent()
{
  fBuildStatus = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("TF_BuildEvent exited by exit command");
    return;
  }
  INFO("TF_BuildEvent started");

  auto adctype = static_cast<ADC::TYPE>(fADCType % 10);
  auto * conf = (fConfigList != nullptr) ? fConfigList->GetSTRGConfig(adctype) : nullptr;

  if (fSoftTrigger != nullptr) {
    if (conf != nullptr) { fSoftTrigger->SetConfig(conf); }
    if (fSoftTrigger->IsEnabled()) {
      fSoftTrigger->SetMode(fADCMode);
      fSoftTrigger->InitTrigger();
    }
  }

  if (fTriggerMode == TRIGGER::GLOBAL) { BuildEvent_GLT(); }
  else if (fTriggerMode == TRIGGER::MODULE) {
    BuildEvent_MOD();
  }
  else {
    BuildEvent_SLF();
  }

  if (fBuildStatus != ERROR) { fBuildStatus = ENDED; }
  INFO("TF_BuildEvent ended");
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
    fBuildStatus = ERROR;
    return;
  }
  auto ** buffers = fADCRawBuffers.data();

  fBuildStatus = RUNNING;
  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }

    std::size_t nmod = 0;
    if (fSortStatus == ENDED) {
      for (std::size_t i = 0; i < nadc; i++) {
        auto * adceventbuffer = buffers[i];
        if (adceventbuffer == nullptr || adceventbuffer->empty()) { continue; }
        nmod += 1;
      }
      if (nmod < nadc) { break; }
    }

    int totalsize = 0;

    nmod = 0;
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
        fBuildStatus = ERROR;
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
        mlock.unlock();

        builtevent->SetEventNumber(fNBuiltEvent);

        fBuiltEventBuffer1.push_back(builtevent);

        if (fDoHistograming) { fBuiltEventBuffer2.push_back(builtevent); }
      }

      totalsize -= 1;
    }
    StopBenchmark("BuildEvent");

    ThreadSleep(fBuildSleep, perror, integral, totalsize);
  }
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

void CupDAQManager::TF_MergeEvent()
{
  fTotalRawDataSize = 0;

  int nmerge = 0;
  double perror = 0;
  double integral = 0;

  fMergeStatus = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("TF_MergeEvent: exited by exit command");
    return;
  }
  INFO("TF_MergeEvent: event merger started");

  auto adctype = static_cast<ADC::TYPE>(static_cast<int>(fADCType) % 10);
  auto * conf = fConfigList->GetSTRGConfig(adctype);
  if (conf) { fSoftTrigger->SetConfig(conf); }
  if (fSoftTrigger->IsEnabled()) {
    fSoftTrigger->SetMode(fADCMode);
    fSoftTrigger->InitTrigger();
  }

  std::unique_lock<std::mutex> mlock(fMonitorMutex, std::defer_lock);
  std::unique_lock<std::mutex> block(fRecvBufferMutex, std::defer_lock);

  int ndaq = static_cast<int>(fRecvEventBuffer.size());
  auto * size = new int[ndaq];
  auto * evtnum = new unsigned int[ndaq];
  auto * evttime = new unsigned long[ndaq];

  fMergeStatus = RUNNING;
  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }

    bool dobuild = true;
    int totalsize = 0;
    int nd = 0;
    std::string empty_daqs = "";
    for (auto & buf : fRecvEventBuffer) {
      size[nd] = static_cast<int>(buf.second->size());
      if (size[nd] == 0) { 
        dobuild = false; 
        if (fVerboseLevel > 1) { empty_daqs += std::to_string(buf.first) + " "; }
      }
      totalsize += size[nd];
      nd += 1;
    }

    if (!dobuild && fVerboseLevel > 1) {
      static int wait_count = 0;
      if (wait_count++ % 100 == 0) { // log periodically to avoid spam
        DEBUG("[DEBUG-TF_MergeEvent] Waiting for data from DAQs: %s (Wait count: %d)", empty_daqs.c_str(), wait_count);
      }
    }

    if (!dobuild) {
      totalsize = 0;
      if (fDoEndRun) {
        INFO("TF_MergeEvent: merger consumed all events");
        break;
      }
    }
    else {
      totalsize /= ndaq;

      nmerge += 1;
      auto builtevent = std::make_shared<BuiltEvent>();

      nd = 0;
      bool iserror = false;

      auto merge_start = std::chrono::steady_clock::now();
      block.lock();
      for (auto & buf : fRecvEventBuffer) {
        auto bevent_opt = buf.second->pop_front();
        if (!bevent_opt.has_value()) {
          iserror = true;
          break;
        }

        std::shared_ptr<BuiltEvent> & bevent_ptr = bevent_opt.value();
        BuiltEvent * bevent = bevent_ptr.get();

        evtnum[nd] = bevent->GetEventNumber();
        // NOTE: fRecvBufferMutex is already held by block.lock() above.
        // Do NOT re-acquire it here (std::mutex is non-recursive → deadlock).
        evttime[nd] = bevent->GetTriggerTime();

        int nadc = bevent->GetEntries();
        for (int i = 0; i < nadc; i++) {
          switch (fADCMode) {
            case ADC::SMODE: {
              auto * adc = static_cast<SADCRawEvent *>(bevent->At(i));
              auto * newadc = new SADCRawEvent(*adc);
              builtevent->Add(newadc);
              fTotalRawDataSize += adc->GetRawDataSize();
              break;
            }
            case ADC::FMODE: {
              auto * adc = static_cast<FADCRawEvent *>(bevent->At(i));
              auto * newadc = new FADCRawEvent(*adc);
              builtevent->Add(newadc);
              fTotalRawDataSize += adc->GetRawDataSize();
              break;
            }
            default: break;
          }
        }

        if (nd > 0) {
          if (evtnum[nd] != evtnum[0]) { iserror = true; }
          if (evttime[nd] != evttime[0]) { iserror = true; }
        }
        nd += 1;
      }
      block.unlock();
      auto merge_end = std::chrono::steady_clock::now();
      long long merge_dur_us = std::chrono::duration_cast<std::chrono::microseconds>(merge_end - merge_start).count();

      if (nmerge % 100 == 0 || merge_dur_us > 5000) {
        INFO("[DEBUG-TF_MergeEvent] Built %d-th event from %d DAQs. Lock+Copy took %lld us. (totalsize before merge=%d)", 
             nmerge, ndaq, merge_dur_us, totalsize * ndaq);
      }

      if (iserror) {
        RUNSTATE::SetError(fRunStatus);
        fMergeStatus = ERROR;
        ERROR("TF_MergeEvent: events from DAQs are different");
        break;
      }

      mlock.lock();
      fTriggerNumber = nmerge;
      fTriggerTime = builtevent->GetTriggerTime();
      mlock.unlock();

      bool istriggered = true;

      if (fSoftTrigger->IsEnabled() && !fSoftTrigger->DoTrigger(builtevent.get())) {
        istriggered = false;
      }

      if (istriggered) {
        mlock.lock();
        fNBuiltEvent += 1;
        mlock.unlock();

        builtevent->SetEventNumber(fNBuiltEvent);

        fBuiltEventBuffer1.push_back(builtevent);

        if (fDoHistograming) { fBuiltEventBuffer2.push_back(builtevent); }
      }

      totalsize -= 1;
      if (totalsize < 0) { totalsize = 0; }
    }

    ThreadSleep(fMergeSleep, perror, integral, totalsize, 1, 10.0);
  }

  fMergeStatus = ENDED;

  for (auto & buf : fRecvEventBuffer) {
    while (!buf.second->empty()) {
      auto ev = buf.second->pop_front();
    }
  }

  delete[] size;
  delete[] evtnum;
  delete[] evttime;

  INFO("TF_MergeEvent: event merger ended");
}
