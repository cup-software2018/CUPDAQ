#include "TMessage.h"
#include "TMonitor.h"
#include "TSocket.h"

#include "DAQ/CupDAQManager.hh"
#include "DAQUtils/ELog.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"

void CupDAQManager::TF_SendEvent()
{
  double perror = 0;
  double integral = 0;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("TF_SendEvent: exited by exit command");
    return;
  }
  INFO("TF_SendEvent: Data Client started, connecting to Data Server...");

  auto socket = std::make_unique<TSocket>(fMergeServerIPAddr.c_str(), fMergeServerPort);

  if (socket->GetErrorCode() < 0) {
    RUNSTATE::SetError(fRunStatus);
    ERROR("TF_SendEvent: connection to Data Server failed at %s:%d", fMergeServerIPAddr.c_str(),
          fMergeServerPort);
    return;
  }

  INFO("TF_SendEvent: connection to Data Server succeeded");

  fSendStatus = RUNNING;

  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }

    if (fBuiltEventBuffer1.empty()) {
      if (fBuildStatus == ENDED) { break; }
    }
    else {
      auto bevent_opt = fBuiltEventBuffer1.pop_front();
      if (!bevent_opt.has_value()) { continue; }

      std::shared_ptr<BuiltEvent> bevent_ptr = bevent_opt.value();
      BuiltEvent * bevent = bevent_ptr.get();

      int state = socket->SendObject(bevent);

      if (state < 0) {
        RUNSTATE::SetError(fRunStatus);
        ERROR("TF_SendEvent: sending BuiltEvent to Data Server failed");
        break;
      }
    }

    int size = static_cast<int>(fBuiltEventBuffer1.size());
    ThreadSleep(fSendSleep, perror, integral, size);
  }

  fSendStatus = ENDED;
  INFO("TF_SendEvent: Data Client disconnected and ended");
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
    for (auto & buf : fRecvEventBuffer) {
      size[nd] = static_cast<int>(buf.second->size());
      if (size[nd] == 0) { dobuild = false; }
      totalsize += size[nd];
      nd += 1;
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
        std::lock_guard<std::mutex> lock(fRecvBufferMutex);
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

        if (fDoHistograming) {
          if (fBuiltEventBuffer2.size() < 1000) {
            fBuiltEventBuffer2.push_back(builtevent);
          }
        }
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
