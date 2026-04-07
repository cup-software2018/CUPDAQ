//
// Created by cupsoft on 7/24/19.
//
#include <memory>

#include "DAQ/CupDAQManager.hh"
#include "DAQConfig/IADCTConf.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"
#include "DAQUtils/ELog.hh"

void CupDAQManager::TF_SortEvent()
{
  fSortStatus = READY;

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }
  INFO("sorting data started");

  if (fTriggerMode == TRIGGER::SELF) { SortEvent_CHA(); }
  else {
    SortEvent_MOD();
  }

  fSortStatus = ENDED;
  INFO("sorting data ended");
}

void CupDAQManager::SortEvent_MOD()
{
  const int nadc_int = GetEntries();
  if (nadc_int <= 0) {
    WARNING("no ADC modules in SortEvent_MOD");
    return;
  }

  double perror = 0.0;
  double integral = 0.0;

  fSortStatus = RUNNING;
  while (true) {
    // emergent exit
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }

    if (fReadStatus == ENDED) {
      int remain = 0;
      for (int i = 0; i < nadc_int; ++i) {
        auto * adc = static_cast<AbsADC *>(fCont[i]);
        remain += adc->Bsize();
      }
      if (remain == 0) { break; }
    }

    StartBenchmark("SortEvent");
    int totalsize = 0;
    for (int i = 0; i < nadc_int; ++i) {
      auto * adc = static_cast<AbsADC *>(fCont[i]);
      totalsize += adc->Bsize();

      if (adc->Bempty()) { continue; }

      auto chunkdata = adc->Bpop_front();
      if (!chunkdata) { continue; }

      unsigned char * data = chunkdata->data;
      const int nevent = kKILOBYTES * chunkdata->size / fADCEventDataSize;

      for (int j = 0; j < nevent; ++j) {
        std::unique_ptr<AbsADCRaw> adcevent;

        switch (fADCType) {
          case ADC::FADCS:
          case ADC::FADCT: adcevent = std::make_unique<FADCRawEvent>(fNDP, fADCEventDataSize, ADC::FADC); break;
          case ADC::GADCS:
          case ADC::GADCT: adcevent = std::make_unique<FADCRawEvent>(fNDP, fADCEventDataSize, ADC::GADC); break;
          case ADC::MADCS: adcevent = std::make_unique<FADCRawEvent>(fNDP, fADCEventDataSize, ADC::MADC); break;
          case ADC::SADCS:
          case ADC::SADCT: adcevent = std::make_unique<SADCRawEvent>(fADCEventDataSize, ADC::SADC); break;
          case ADC::IADCT: {
            auto * conf = static_cast<IADCTConf *>(adc->GetConfig());
            if (conf->MODE() > 0) { adcevent = std::make_unique<FADCRawEvent>(fNDP, fADCEventDataSize, ADC::IADC); }
            else {
              adcevent = std::make_unique<SADCRawEvent>(fADCEventDataSize, ADC::IADC);
            }
            break;
          }
          default: break;
        }

        if (!adcevent) { continue; }

        adcevent->CopyDataFrom(data + j * fADCEventDataSize);
        adcevent->Unpack(adc->GetConfig(), fVerboseLevel);

        auto * buffer = fADCRawBuffers.at(static_cast<std::size_t>(i));
        buffer->push_back(std::move(adcevent));
      }
    }
    StopBenchmark("SortEvent");

    const int denom = (nadc_int > 0) ? nadc_int : 1;
    totalsize /= denom;

    ThreadSleep(fSortSleep, perror, integral, totalsize);
  }
}

void CupDAQManager::SortEvent_CHA() {}
