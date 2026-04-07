#include <algorithm>
#include <cstring>
#include <memory>

#include "DAQ/CupDAQManager.hh"
#include "DAQUtils/ELog.hh"

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

  double perror = 0.0;
  double integral = 0.0;

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
        int temp = 0;
        for (std::size_t i = 0; i < nadc; i++) {
          temp += sanity[i];
        }
        if (temp < 0) {
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

      auto builtevent = std::make_unique<BuiltEvent>();
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
          continue;
        }
        else if (s == 2) {
          WARNING("event missed in ADC [mid=%d]", adc->GetSID());
          continue;
        }
        else if (s == 3) {
          ERROR("local trigger time error in ADC [mid=%d]", adc->GetSID());
          nerror += 1;
          continue;
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

        std::unique_ptr<BuiltEvent> hist_clone;
        if (fDoHistograming) { hist_clone = std::make_unique<BuiltEvent>(*builtevent); }

        fBuiltEventBuffer1.push_back(std::move(builtevent));

        if (fDoHistograming) { fBuiltEventBuffer2.push_back(std::move(hist_clone)); }
      }

      totalsize -= 1;
    }
    StopBenchmark("BuildEvent");

    ThreadSleep(fBuildSleep, perror, integral, totalsize);
  }
}

void CupDAQManager::BuildEvent_MOD() {}

void CupDAQManager::BuildEvent_SLF() {}

void CupDAQManager::CheckEventSanity(ADCHeader ** header, unsigned int * tn, unsigned long * tt, int * error)
{
  std::size_t nadc = static_cast<std::size_t>(GetEntries());
  std::memset(error, 0, nadc * sizeof(int));

  if (nadc == 1) {
    if (header[0]->GetError()) { error[0] = 1; }
    return;
  }

  bool iserror = false;
  for (std::size_t i = 0; i < nadc; i++) {
    if (header[i]->GetMID() == 0) { error[i] = -1; }
    if (header[i]->GetError()) { error[0] = 1; }
    tn[i] = header[i]->GetLocalTriggerNumber();
    tt[i] = header[i]->GetLocalTriggerTime();

    if (i > 0) {
      if (tn[i] != tn[0] || tt[i] != tt[0]) { iserror = true; }
    }
  }

  if (iserror) {
    unsigned int nmin = *std::min_element(tn, tn + nadc);
    unsigned long tmin = *std::min_element(tt, tt + nadc);
    for (std::size_t i = 0; i < nadc; i++) {
      if (header[i]->GetLocalTriggerNumber() > nmin) {
        DEBUG("trgnum = %u [mid=%d, %u]", nmin, header[i]->GetMID(), header[i]->GetLocalTriggerNumber());
        error[i] = 2;
      }
      if (header[i]->GetLocalTriggerTime() > tmin) {
        DEBUG("trgtime = %lu [mid=%d, %lu]", tmin, header[i]->GetMID(), header[i]->GetLocalTriggerTime());
        error[i] = 3;
      }
    }
  }
}
