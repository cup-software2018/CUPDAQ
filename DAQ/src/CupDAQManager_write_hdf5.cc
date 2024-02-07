#ifdef ENABLE_HDF5
#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5Event.hh"
#endif
#include "OnlObjs/ADCHeader.hh"
#include "OnlObjs/FADCRawChannel.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"
#include "TObjString.h"

#include "DAQ/CupDAQManager.hh"

void CupDAQManager::WriteFADC_MOD_HDF5()
{
#ifdef ENABLE_HDF5
  fH5Event = new H5Event<FChannel_t>;
  fHDF5File->SetEvent(fH5Event);
  if (!fHDF5File->Open()) {
    fLog->Error("CupDAQManager::WriteFADC_MOD_HDF5",
                "can\'t open hdf5 output file");
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  auto * h5event = (H5Event<FChannel_t> *)fH5Event;

  EventInfo_t eventinfo;
  std::vector<FChannel_t> chdata;

  double perror = 0;
  double integral = 0;

  std::unique_lock<std::mutex> wlock(fWriteFileMutex, std::defer_lock);

  fWriteStatus = RUNNING;
  while (true) {
    // for emergent exit
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) break;
    if (fBuiltEventBuffer1.empty()) {
      if (fBuildStatus == ENDED || fMergeStatus == ENDED) break;
    }
    else {
      chdata.clear();

      BuiltEvent * bevent = fBuiltEventBuffer1.popfront();

      unsigned long fastttime = UINT64_MAX;
      unsigned int tnum = 0;
      unsigned int ttype = 0;
      unsigned short nhit = 0;

      int nadc = bevent->GetEntries();
      for (int j = 0; j < nadc; j++) {
        auto * adcraw = (FADCRawEvent *)bevent->At(j);
        auto * header = adcraw->GetADCHeader();

        if (header->GetLocalTriggerTime() < fastttime) {
          fastttime = header->GetLocalTriggerTime();
          tnum = header->GetTriggerNumber();
          ttype = header->GetTriggerType();
        }

        AbsConf * conf = fConfigList->FindConfig(fADCType, header->GetMID());

        for (int i = 0; i < kNCHFADC; i++) {
          if (header->GetZero(i)) continue;

          FChannel_t channel;
          channel.id = conf->PID(i);
          channel.tbit = header->GetTriggerBit(i);
          channel.ped = header->GetPedestal(i);
          auto * rawchannel = adcraw->GetChannel(i);
          channel.SetWaveform(rawchannel->GetADC());

          chdata.push_back(channel);
          nhit += 1;
        }
      }
      eventinfo.tnum = tnum;
      eventinfo.ttime = fastttime;
      eventinfo.ttype = ttype;
      eventinfo.nhit = nhit;

      wlock.lock();
      h5event->WriteEvent(eventinfo, chdata);
      wlock.unlock();

      delete bevent;
    }

    int size = fBuiltEventBuffer1.size();
    ThreadSleep(fWriteSleep, perror, integral, size);
  }
#endif
}

void CupDAQManager::WriteSADC_MOD_HDF5()
{
#ifdef ENABLE_HDF5
  fH5Event = new H5Event<AChannel_t>;
  fHDF5File->SetEvent(fH5Event);
  if (!fHDF5File->Open()) {
    fLog->Error("CupDAQManager::WriteFADC_MOD_HDF5",
                "can\'t open hdf5 output file");
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  auto * h5event = (H5Event<AChannel_t> *)fH5Event;

  EventInfo_t eventinfo;
  std::vector<AChannel_t> chdata;

  double perror = 0;
  double integral = 0;

  int nadcch = 32;
  ADC::TYPE adctype = static_cast<ADC::TYPE>(static_cast<int>(fADCType) % 10);
  switch (adctype) {
    case ADC::SADC: nadcch = 32; break;
    case ADC::IADC: nadcch = 40; break;
    default: break;
  }

  std::unique_lock<std::mutex> wlock(fWriteFileMutex, std::defer_lock);

  fWriteStatus = RUNNING;
  while (true) {
    // for emergent exit
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) break;
    if (fBuiltEventBuffer1.empty()) {
      if (fBuildStatus == ENDED || fMergeStatus == ENDED) break;
    }
    else {
      chdata.clear();

      BuiltEvent * bevent = fBuiltEventBuffer1.popfront();

      unsigned long fastttime = UINT64_MAX;
      unsigned int tnum = 0;
      unsigned int ttype = 0;
      unsigned short nhit = 0;

      int nadc = bevent->GetEntries();
      for (int j = 0; j < nadc; j++) {
        auto * adcraw = (SADCRawEvent *)bevent->At(j);
        auto * header = adcraw->GetADCHeader();

        if (header->GetLocalTriggerTime() < fastttime) {
          fastttime = header->GetLocalTriggerTime();
          tnum = header->GetTriggerNumber();
          ttype = header->GetTriggerType();
        }

        AbsConf * conf = fConfigList->FindConfig(fADCType, header->GetMID());

        for (int i = 0; i < nadcch; i++) {
          if (header->GetZero(i)) continue;

          AChannel_t channel;
          channel.id = conf->PID(i);
          channel.tbit = header->GetTriggerBit(i);
          channel.adc = adcraw->GetADC(i);
          channel.time = adcraw->GetTime(i);

          chdata.push_back(channel);
          nhit += 1;
        }
      }
      eventinfo.tnum = tnum;
      eventinfo.ttime = fastttime;
      eventinfo.ttype = ttype;
      eventinfo.nhit = nhit;

      wlock.lock();
      h5event->WriteEvent(eventinfo, chdata);
      wlock.unlock();

      delete bevent;
    }

    int size = fBuiltEventBuffer1.size();
    ThreadSleep(fWriteSleep, perror, integral, size);
  }
#endif
}

long CupDAQManager::OpenNewHDF5File(const char * filename)
{
  long retval = 0;

#ifdef ENABLE_HDF5
  TString bname = gSystem->BaseName(filename);
  TObjArray * objs = bname.Tokenize(".");
  int subnum =
      TString(((TObjString *)objs->At(objs->GetEntries() - 1))->GetName())
          .Atoi();

  if (subnum == 0) {
    fHDF5File = new H5DataWriter(filename, fCompressionLevel);
    fHDF5File->SetSubrun(0);
  }
  else {
    retval = fHDF5File->GetFileSize();
    fHDF5File->Close();
    delete fHDF5File;
    fHDF5File = new H5DataWriter(filename, fCompressionLevel);
    fHDF5File->SetSubrun(subnum);
    fHDF5File->SetEvent(fH5Event);
    if (!fHDF5File->Open()) {
      fLog->Error("CupDAQManager::OpenNewHDF5File",
                  "can\'t open output file %s", filename);
      return -1;
    }
  }

  fLog->Info("CupDAQManager::OpenNewHDF5File", "%s opened", filename);
  return retval;
#else
  fLog->Error("CupDAQManager::OpenNewHDF5File",
                  "HDF5 not supported");
  return -1;
#endif
}
