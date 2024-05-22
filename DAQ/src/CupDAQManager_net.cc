#include "TMessage.h"
#include "TMonitor.h"
#include "TServerSocket.h"
#include "TSocket.h"

#include "DAQ/CupDAQManager.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"

void CupDAQManager::TF_SendEvent()
{
  double perror = 0;
  double integral = 0;

  // char tmpbuf[4];
  char data[kMESSLEN];
  EncodeMsg(data, kRECVEVENT);

  if (!ThreadWait(fRunStatus, fDoExit)) {
    fLog->Warning("CupDAQManager::TF_SendEvent", "exited by exit command");
    return;
  }
  fLog->Info("CupDAQManager::TF_SendEvent", "sending event to merger started");

  auto * socket = new TSocket(fMergeServerIPAddr, fMergeServerPort);
  if (socket->GetErrorCode() < 0) {
    RUNSTATE::SetError(fRunStatus);
    delete socket;
    fLog->Error("CupDAQManager::TF_SendEvent", "connection to merger failed");
    return;
  }
  else
    fLog->Info("CupDAQManager::TF_SendEvent", "connection to merger succeeded");

  fSendStatus = RUNNING;
  while (true) {
    // emergent exit
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) break;
    if (fBuiltEventBuffer1.empty()) {
      if (fBuildStatus == ENDED) break;
    }
    else {
      BuiltEvent * bevent = fBuiltEventBuffer1.popfront();

      socket->SendRaw(data, kMESSLEN);
      int state = socket->SendObject(bevent);
      if (state < 0) {
        RUNSTATE::SetError(fRunStatus);
        fLog->Error("CupDAQManager::TF_SendEvent", "sending event failed (s)");
        break;
      }
      delete bevent;
    }

    int size = fBuiltEventBuffer1.size();
    ThreadSleep(fSendSleep, perror, integral, size);
  }
  fSendStatus = ENDED;
  fLog->Info("CupDAQManager::TF_SendEvent", "sending event ended");
}

void CupDAQManager::TF_MergeEvent()
{
  fTotalRawDataSize = 0;

  int nmerge = 0;
  double perror = 0;
  double integral = 0;

  fMergeStatus = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    fLog->Warning("CupDAQManager::TF_MergeEvent", "exited by exit command");
    return;
  }
  fLog->Info("CupDAQManager::TF_MergeEvent", "event merger started");

  // prepare software trigger
  auto adctype = (ADC::TYPE)(fADCType % 10);
  auto * conf = fConfigList->GetSTRGConfig(adctype);
  if (conf) fSoftTrigger->SetConfig(conf);
  if (fSoftTrigger->IsEnabled()) {
    fSoftTrigger->SetMode(fADCMode);
    fSoftTrigger->InitTrigger();
  }

  std::unique_lock<std::mutex> mlock(fMonitorMutex, std::defer_lock);

  int ndaq = fRecvEventBuffer.size();
  int * size = new int[ndaq];
  unsigned int * evtnum = new unsigned int[ndaq];
  unsigned long * evttime = new unsigned long[ndaq];

  fMergeStatus = RUNNING;
  while (true) {
    // emergent exit
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) break;

    bool dobuild = true;
    int totalsize = 0, nd = 0;
    for (auto buf : fRecvEventBuffer) {
      size[nd] = buf.second->size();
      if (size[nd] == 0) dobuild = false;
      totalsize += size[nd];
      nd += 1;
    }

    if (!dobuild) {
      totalsize = 0;
      if (fDoEndRun) {
        fLog->Info("CupDAQManager::TF_MergeEvent",
                   "merger consumed all events");
        break;
      }
    }
    else {
      totalsize /= ndaq;

      nmerge += 1;
      auto * builtevent = new BuiltEvent();

      nd = 0;
      bool iserror = false;
      for (auto buf : fRecvEventBuffer) {
        BuiltEvent * bevent = buf.second->popfront();

        evtnum[nd] = bevent->GetEventNumber();
        evttime[nd] = bevent->GetTriggerTime();

        int nadc = bevent->GetEntries();
        for (int i = 0; i < nadc; i++) {
          switch (fADCMode) {
            case ADC::SMODE: {
              auto * adc = (SADCRawEvent *)bevent->At(i);
              auto * newadc = new SADCRawEvent(*adc);
              builtevent->Add(newadc);
              fTotalRawDataSize += adc->GetRawDataSize();
              break;
            }
            case ADC::FMODE: {
              auto * adc = (FADCRawEvent *)bevent->At(i);
              auto * newadc = new FADCRawEvent(*adc);
              builtevent->Add(newadc);
              fTotalRawDataSize += adc->GetRawDataSize();
              break;
            }
            default: break;
          }
        }
        delete bevent;

        if (nd > 0) {
          if (evtnum[nd] != evtnum[0]) iserror = true;
          if (evttime[nd] != evttime[0]) iserror = true;
        }
        nd += 1;
      }

      if (iserror) {
        RUNSTATE::SetError(fRunStatus);
        fMergeStatus = ERROR;
        fLog->Error("CupDAQManager::TF_MergeEvent",
                    "events from DAQs are different");
        break;
      }

      mlock.lock();
      fTriggerNumber = nmerge;
      fTriggerTime = builtevent->GetTriggerTime();
      mlock.unlock();

      bool istriggered = true;

      // software trigger
      if (fSoftTrigger->IsEnabled() && !fSoftTrigger->DoTrigger(builtevent)) {
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
      if (totalsize < 0) totalsize = 0;
    }

    ThreadSleep(fMergeSleep, perror, integral, totalsize, 1, 10);
    //std::cout << Form("size=%5d, sleep=%8d, int=%f", totalsize, fMergeSleep,
    //                  integral)
    //          << std::endl;
  }
  fMergeStatus = ENDED;

  for (auto buf : fRecvEventBuffer) {
    while (!buf.second->empty()) {
      auto * event = buf.second->popfront(false);
      delete event;
    }
  }

  delete[] evtnum;
  delete[] evttime;

  fLog->Info("CupDAQManager::TF_MergeEvent", "event merger ended");
}
