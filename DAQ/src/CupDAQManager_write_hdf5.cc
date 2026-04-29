#ifdef ENABLE_HDF5
#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5FADCEvent.hh"
#include "HDF5Utils/H5SADCEvent.hh"
#endif

#include <cstdlib>
#include <string>

#include "DAQ/CupDAQManager.hh"
#include "DAQUtils/ELog.hh"
#include "OnlObjs/ADCHeader.hh"
#include "OnlObjs/FADCRawChannel.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"

#ifdef ENABLE_HDF5

void CupDAQManager::WriteFADC_MOD_HDF5()
{
  auto * h5event = new H5FADCEvent;
  h5event->SetNDP(fNDP);
  fH5Event = h5event;

  h5event->SetBufferCapacity(100);
  h5event->SetBufferMaxBytes(32 * 1024 * 1024);

  fHDF5File->SetData(h5event);
  if (!fHDF5File->Open()) {
    ERROR("can't open hdf5 output file");
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  EventInfo_t eventinfo{};
  std::vector<FChannel_t> chdata;
  chdata.reserve(kNCHFADC);

  double perror = 0;
  double integral = 0;

  int nadcch = 0;
  ADC::TYPE adctype = static_cast<ADC::TYPE>(static_cast<int>(fADCType) % 10);
  switch (adctype) {
    case ADC::FADC: nadcch = kNCHFADC; break;
    case ADC::GADC: nadcch = kNCHGADC; break;
    case ADC::MADC: nadcch = kNCHMADC; break;
    case ADC::IADC: nadcch = kNCHIADC; break;
    default: break;
  }

  if (nadcch == 0) {
    ERROR("invalid adc type");
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  std::unique_lock<std::mutex> wlock(fWriteFileMutex, std::defer_lock);

  fWriteStatus.store(RUNNING);

  while (true) {
    if (fDoExit.load() || RUNSTATE::CheckError(fRunStatus)) { break; }

    if (fBuiltEventBuffer1.empty()) {
      if (fBuildStatus.load() == ENDED) { break; }
    }
    else {
      StartBenchmark("WriteEvent");

      chdata.clear();

      auto bevent_opt = fBuiltEventBuffer1.pop_front();
      if (!bevent_opt.has_value()) {
        int size_empty = static_cast<int>(fBuiltEventBuffer1.size());
        ThreadSleep(fWriteSleep, perror, integral, size_empty);
        continue;
      }

      std::shared_ptr<BuiltEvent> bevent_ptr = bevent_opt.value();
      BuiltEvent * ev = bevent_ptr.get();

      eventinfo.tnum = ev->GetTriggerNumber();
      eventinfo.ttime = ev->GetTriggerTime();
      eventinfo.ttype = ev->GetTriggerType();

      unsigned short nhit = 0;

      const int nadc = ev->GetEntries();
      for (int j = 0; j < nadc; ++j) {
        auto * adcraw = static_cast<FADCRawEvent *>(ev->At(j));
        auto * header = adcraw->GetADCHeader();

        AbsConf * conf = fConfigList->FindConfig(fADCType, header->GetMID());
        if (!conf) {
          ERROR("no config for mid=%d", header->GetMID());
          RUNSTATE::SetError(fRunStatus);
          break;
        }

        for (int i = 0; i < nadcch; ++i) {
          if (header->GetZero(i)) { continue; }

          FChannel_t channel{};
          channel.id = conf->PID(i);
          channel.tbit = header->GetTriggerBit(i);
          channel.ped = header->GetPedestal(i);

          auto * rawchannel = adcraw->GetChannel(i);
          channel.SetWaveform(rawchannel->GetADC(), fNDP);

          chdata.push_back(channel);
          nhit += 1;
        }
      }

      eventinfo.nhit = nhit;

      wlock.lock();
      herr_t status = h5event->AppendEvent(eventinfo, chdata);
      wlock.unlock();

      if (status < 0) {
        ERROR("H5FADCEvent::AppendEvent failed (tnum=%u)", eventinfo.tnum);
        RUNSTATE::SetError(fRunStatus);
        break;
      }

      StopBenchmark("WriteEvent");
    }

    int size = static_cast<int>(fBuiltEventBuffer1.size());
    ThreadSleep(fWriteSleep, perror, integral, size);
  }
}

void CupDAQManager::WriteSADC_MOD_HDF5()
{
  auto * h5event = new H5SADCEvent;
  fH5Event = h5event;

  h5event->SetBufferCapacity(1000);
  h5event->SetBufferMaxBytes(32 * 1024 * 1024);

  fHDF5File->SetData(h5event);
  if (!fHDF5File->Open()) {
    ERROR("can't open hdf5 output file");
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  EventInfo_t eventinfo{};
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

  fWriteStatus.store(RUNNING);

  while (true) {
    if (fDoExit.load() || RUNSTATE::CheckError(fRunStatus)) { break; }

    if (fBuiltEventBuffer1.empty()) {
      if (fBuildStatus.load() == ENDED) { break; }
    }
    else {
      StartBenchmark("WriteEvent");

      chdata.clear();

      auto bevent_opt = fBuiltEventBuffer1.pop_front();
      if (!bevent_opt.has_value()) {
        int size_empty = static_cast<int>(fBuiltEventBuffer1.size());
        ThreadSleep(fWriteSleep, perror, integral, size_empty);
        continue;
      }

      std::shared_ptr<BuiltEvent> bevent_ptr = bevent_opt.value();
      BuiltEvent * ev = bevent_ptr.get();

      unsigned long fastttime = UINT64_MAX;
      unsigned int tnum = 0;
      unsigned int ttype = 0;
      unsigned short nhit = 0;

      const int nadc = ev->GetEntries();
      for (int j = 0; j < nadc; ++j) {
        auto * adcraw = static_cast<SADCRawEvent *>(ev->At(j));
        auto * header = adcraw->GetADCHeader();

        if (header->GetLocalTriggerTime() < fastttime) {
          fastttime = header->GetLocalTriggerTime();
          tnum = header->GetTriggerNumber();
          ttype = header->GetTriggerType();
        }

        AbsConf * conf = fConfigList->FindConfig(fADCType, header->GetMID());

        for (int i = 0; i < nadcch; ++i) {
          if (header->GetZero(i)) { continue; }

          AChannel_t channel{};
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
      herr_t status = h5event->AppendEvent(eventinfo, chdata);
      wlock.unlock();

      if (status < 0) {
        ERROR("H5SADCEvent::AppendEvent failed (tnum=%u)", tnum);
        RUNSTATE::SetError(fRunStatus);
        break;
      }

      StopBenchmark("WriteEvent");
    }

    int size = static_cast<int>(fBuiltEventBuffer1.size());
    ThreadSleep(fWriteSleep, perror, integral, size);
  }
}

long CupDAQManager::OpenNewHDF5File(const char * filename)
{
  long retval = 0;

  std::string filepath(filename);

  std::size_t slash_pos = filepath.find_last_of("/\\");
  std::string bname = (slash_pos == std::string::npos) ? filepath : filepath.substr(slash_pos + 1);

  int subnum = 0;
  std::size_t dot_pos = bname.find_last_of('.');

  if (dot_pos != std::string::npos && dot_pos + 1 < bname.length()) {

    subnum = std::atoi(bname.substr(dot_pos + 1).c_str());
  }

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
    fHDF5File->SetData(fH5Event);

    if (!fHDF5File->Open()) {
      ERROR("can't open output file %s", filename);
      return -1;
    }
  }

  INFO("%s opened", filename);
  return retval;
}

#else

void CupDAQManager::WriteFADC_MOD_HDF5()
{
  ERROR("HDF5 is not enabled in this build. Cannot write FADC data.");
  RUNSTATE::SetError(fRunStatus);
}

void CupDAQManager::WriteSADC_MOD_HDF5()
{
  ERROR("HDF5 is not enabled in this build. Cannot write SADC data.");
  RUNSTATE::SetError(fRunStatus);
}

long CupDAQManager::OpenNewHDF5File(const char * filename)
{
  ERROR("HDF5 not supported. Cannot open %s", filename);
  return -1;
}

#endif