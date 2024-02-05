//
// Created by cupsoft on 7/24/19.
//
#include "DAQConfig/AmoreADCConf.hh"
#include "DAQConfig/FADCSConf.hh"
#include "DAQConfig/FADCTConf.hh"
#include "DAQConfig/GADCSConf.hh"
#include "DAQConfig/MADCSConf.hh"

#include "DAQ/CupDAQManager.hh"

using namespace std;

void CupDAQManager::TF_BuildEvent()
{
  fBuildStatus = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    fLog->Warning("CupDAQManager::TF_BuildEvent", "exited by exit command");
    return;
  }
  fLog->Info("CupDAQManager::TF_BuildEvent", "event building started");

  // prepare software trigger
  auto adctype = (ADC::TYPE)(fADCType % 10);
  auto * conf = fConfigList->GetSTRGConfig(adctype);
  if (conf) fSoftTrigger->SetConfig(conf);
  if (fSoftTrigger->IsEnabled()) { 
    fSoftTrigger->SetMode(fADCMode);
    fSoftTrigger->InitTrigger(); 
  }

  StartBenchmark("BuildEvent");
  if (fTriggerMode == TRIGGER::GLOBAL) { BuildEvent_GLT(); }
  else if (fTriggerMode == TRIGGER::MODULE) {
    BuildEvent_MOD();
  }
  else {
    BuildEvent_SLF();
  }
  StopBenchmark("BuildEvent");

  if (fBuildStatus != ERROR) fBuildStatus = ENDED;
  fLog->Info("CupDAQManager::TF_BuildEvent", "event building ended");
}

void CupDAQManager::BuildEvent_GLT()
{
  int nadc = GetEntries();

  auto ** header = new ADCHeader *[nadc];
  auto * sanity = new int[nadc];
  auto * trgnum = new unsigned int[nadc];
  auto * trgtime = new unsigned long[nadc];

  double perror = 0;
  double integral = 0;

  std::unique_lock<std::mutex> mlock(fMonitorMutex, std::defer_lock);

  fBuildStatus = RUNNING;
  while (true) {
    // emergent exit
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) break;

    int nmod = 0;
    if (fSortStatus == ENDED) {
      for (int i = 0; i < nadc; i++) {
        ConcurrentDeque<AbsADCRaw *> * adceventbuffer = fADCRawBuffers.at(i);
        if (adceventbuffer->empty()) continue;
        nmod += 1;
      }
      if (nmod < nadc) break;
    }

    int totalsize = 0;

    nmod = 0;
    for (int i = 0; i < nadc; i++) {
      ConcurrentDeque<AbsADCRaw *> * adceventbuffer = fADCRawBuffers.at(i);
      totalsize += adceventbuffer->size();

      AbsADCRaw * adcevent = adceventbuffer->front(false);
      if (!adcevent) continue;

      header[i] = adcevent->GetADCHeader();
      nmod += 1;
    }

    if (nmod == nadc) {
      totalsize = totalsize / nadc;

      CheckEventSanity(header, trgnum, trgtime, sanity);

      if (fADCType == ADC::SADCS || fADCType == ADC::SADCT) {
        int temp = 0;
        for (int i = 0; i < nadc; i++) {
          temp += sanity[i];
        }
        if (temp < 0) {
          fLog->Info("CupDAQManager::BuildEvent_GLT",
                     "SADC null event, will be skipped");
          for (int i = 0; i < nadc; i++) {
            while (true) {
              ConcurrentDeque<AbsADCRaw *> * adceventbuffer =
                  fADCRawBuffers.at(i);
              if (!adceventbuffer->empty()) {
                AbsADCRaw * adcevent = adceventbuffer->popfront();
                delete adcevent;
              }
              else {
                break;
              }
            }
          }
          continue;
        }
      }

      auto * builtevent = new BuiltEvent();
      builtevent->SetDAQID(fDAQID);

      int nerror = 0;
      for (int i = 0; i < nadc; i++) {
        auto * adc = (AbsADC *)fCont[i];

        ConcurrentDeque<AbsADCRaw *> * adceventbuffer = fADCRawBuffers.at(i);

        if (sanity[i] == 1) {
          fLog->Error("CupDAQManager::BuildEvent_GLT",
                      "ADC header is corrupted [mid=%d]", adc->GetSID());
          nerror += 1;
          continue;
        }
        else if (sanity[i] == 2) {
          fLog->Warning("CupDAQManager::BuildEvent_GLT",
                        "event missed in ADC [mid=%d]", adc->GetSID());
          continue;
        }
        else if (sanity[i] == 3) {
          fLog->Error("CupDAQManager::BuildEvent_GLT",
                      "local trigger time error in ADC [mid=%d]",
                      adc->GetSID());
          nerror += 1;
          continue;
        }

        AbsADCRaw * adcevent = adceventbuffer->popfront();
        builtevent->Add(adcevent);
      }

      if (nerror > 0) {
        RUNSTATE::SetError(fRunStatus);
        fBuildStatus = ERROR;
        break;
      }

      bool istriggered = true;

      // software trigger
      if (!fDoSendEvent && fSoftTrigger->IsEnabled() &&
          !fSoftTrigger->DoTrigger(builtevent)) {
        istriggered = false;
      }

      if (istriggered) {
        mlock.lock();
        fNBuiltEvent += 1;
        mlock.unlock();

        builtevent->SetEventNumber(fNBuiltEvent);
        fBuiltEventBuffer1.push_back(builtevent);
        if (fDoHistograming) {
          auto * builtevent2 = new BuiltEvent(*builtevent);
          fBuiltEventBuffer2.push_back(builtevent2);
        }
      }
      else {
        delete builtevent;
      }
      totalsize -= 1;
    }

    ThreadSleep(fBuildSleep, perror, integral, totalsize);
    //std::cout << Form("size=%5d, sleep=%8d, int=%f", totalsize, fBuildSleep,
    //                  integral)
    //          << std::endl;    
  }

  delete[] sanity;
  delete[] header;
  delete[] trgnum;
  delete[] trgtime;
}

void CupDAQManager::BuildEvent_MOD() {}

void CupDAQManager::BuildEvent_SLF() {}

void CupDAQManager::CheckEventSanity(ADCHeader ** header, unsigned int * tn,
                                     unsigned long * tt, int * error)
{
  int nadc = GetEntries();
  memset(error, 0, nadc * sizeof(int));

  if (nadc == 1) {
    if (header[0]->GetError()) error[0] = 1;
    return;
  }

  bool iserror = false;
  for (int i = 0; i < nadc; i++) {
    if (header[i]->GetMID() == 0) error[i] = -1;
    if (header[i]->GetError()) error[0] = 1;
    tn[i] = header[i]->GetLocalTriggerNumber();
    tt[i] = header[i]->GetLocalTriggerTime();

    if (i > 0) {
      if (tn[i] != tn[0] || tt[i] != tt[0]) { iserror = true; }
    }
  }

  if (iserror) {
    unsigned int nmin = *std::min_element(tn, tn + nadc);
    unsigned long tmin = *std::min_element(tt, tt + nadc);
    for (int i = 0; i < nadc; i++) {
      if (header[i]->GetLocalTriggerNumber() > nmin) {
        fLog->Debug("CupDAQManager::CheckEventSanity",
                    "trgnum = %d [mid=%d, %d]", nmin, header[i]->GetMID(),
                    header[i]->GetLocalTriggerNumber());
        error[i] = 2;
      }
      if (header[i]->GetLocalTriggerTime() > tmin) {
        fLog->Debug("CupDAQManager::CheckEventSanity",
                    "trgtime = %ld [mid=%d, %ld]", tmin,
                    header[i]->GetMID(), header[i]->GetLocalTriggerTime());
        error[i] = 3;
      }
    }
  }
}
