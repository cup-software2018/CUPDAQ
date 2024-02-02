//
// Created by cupsoft on 7/24/19.
//
#include "DAQ/CupDAQManager.hh"
#include "DAQConfig/IADCTConf.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"

void CupDAQManager::TF_SortEvent()
{
  fSortStatus = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    fLog->Warning("CupDAQManager::TF_SortEvent", "exited by exit command");    
    return;
  }
  fLog->Info("CupDAQManager::TF_SortEvent", "sorting data started");

  StartBenchmark("SortEvent");
  if (fTriggerMode == TRIGGER::SELF) { SortEvent_CHA(); }
  else {
    SortEvent_MOD();
  }
  StopBenchmark("SortEvent");

  fSortStatus = ENDED;
  fLog->Info("CupDAQManager::TF_SortEvent", "sorting data ended");
}

void CupDAQManager::SortEvent_MOD()
{
  int nadc = GetEntries();

  AbsADCRaw * adcevent = nullptr;

  double perror = 0;
  double integral = 0;

  fSortStatus = RUNNING;
  while (true) {
    // for emergent exit
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) break;

    if (fReadStatus == ENDED) {
      int remain = 0;
      for (int i = 0; i < nadc; i++) {
        auto * adc = (AbsADC *)fCont[i];
        remain += adc->Bsize();
      }
      if (remain == 0) { break; }
    }

    int totalsize = 0;
    for (int i = 0; i < nadc; i++) {
      auto * adc = (AbsADC *)fCont[i];
      totalsize += adc->Bsize();

      if (adc->Bempty()) continue;

      ChunkData * chunkdata = adc->Bpopfront(false);
      unsigned char * data = chunkdata->data;

      int nevent = kKILOBYTES * chunkdata->size / fADCEventDataSize;
      for (int j = 0; j < nevent; j++) {
        switch (fADCType) {
          case ADC::FADCS:
            adcevent = new FADCRawEvent(fNDP, fADCEventDataSize, ADC::FADC);
            break;
          case ADC::FADCT:
            adcevent = new FADCRawEvent(fNDP, fADCEventDataSize, ADC::FADC);
            break;
          case ADC::GADCS:
            adcevent = new FADCRawEvent(fNDP, fADCEventDataSize, ADC::GADC);
            break;
          case ADC::GADCT:
            adcevent = new FADCRawEvent(fNDP, fADCEventDataSize, ADC::GADC);
            break;
          case ADC::MADCS:
            adcevent = new FADCRawEvent(fNDP, fADCEventDataSize, ADC::MADC);
            break;
          case ADC::SADCS:
            adcevent = new SADCRawEvent(fADCEventDataSize, ADC::SADC);
            break;
          case ADC::SADCT:
            adcevent = new SADCRawEvent(fADCEventDataSize, ADC::SADC);
            break;
          case ADC::IADCT: {
            auto * conf = (IADCTConf *)adc->GetConfig();
            if (conf->MODE() > 0)
              adcevent = new FADCRawEvent(fNDP, fADCEventDataSize, ADC::IADC);
            else adcevent = new SADCRawEvent(fADCEventDataSize, ADC::IADC);
            break;
          }
          default: break;
        }
        adcevent->CopyDataFrom(data + j * fADCEventDataSize);
        adcevent->Unpack(adc->GetConfig(), fVerboseLevel);

        ConcurrentDeque<AbsADCRaw *> * buffer = fADCRawBuffers.at(i);
        buffer->push_back(adcevent);
      }

      delete chunkdata;
    }

    totalsize = totalsize / nadc;
    ThreadSleep(fSortSleep, perror, integral, totalsize);
  }
}

void CupDAQManager::SortEvent_CHA() {}
