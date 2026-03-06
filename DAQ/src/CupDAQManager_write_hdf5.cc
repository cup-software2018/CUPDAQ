#ifdef ENABLE_HDF5
#include "HDF5Utils/EDM.hh"
#include "HDF5Utils/H5FADCEvent.hh"
#include "HDF5Utils/H5SADCEvent.hh"
#endif

#include <cstdlib>
#include <string>

#include "DAQ/CupDAQManager.hh"
#include "OnlObjs/ADCHeader.hh"
#include "OnlObjs/FADCRawChannel.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"

// =====================================================================
// HDF5 Enabled Implementation
// =====================================================================
#ifdef ENABLE_HDF5

void CupDAQManager::WriteFADC_MOD_HDF5()
{
  auto * h5event = new H5FADCEvent;
  h5event->SetNDP(fNDP);
  fH5Event = h5event;

  fHDF5File->SetEvent(h5event);
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

  int nadcch = 4;
  ADC::TYPE adctype = static_cast<ADC::TYPE>(static_cast<int>(fADCType) % 10);
  switch (adctype) {
    case ADC::FADC: nadcch = 4; break;
    case ADC::GADC: nadcch = 16; break;
    case ADC::MADC: nadcch = 4; break;
    case ADC::IADC: nadcch = 40; break;
    default: break;
  }

  std::unique_lock<std::mutex> wlock(fWriteFileMutex, std::defer_lock);

  fWriteStatus = RUNNING;
  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) break;

    if (fBuiltEventBuffer1.empty()) {
      if (fBuildStatus == ENDED || fMergeStatus == ENDED) break;
    }
    else {
      StartBenchmark("WriteEvent");
      chdata.clear();

      auto bevent_opt = fBuiltEventBuffer1.pop_front();
      if (!bevent_opt.has_value()) {
        int size_empty = fBuiltEventBuffer1.size();
        ThreadSleep(fWriteSleep, perror, integral, size_empty);
        continue;
      }

      std::unique_ptr<BuiltEvent> bevent = std::move(bevent_opt.value());
      BuiltEvent * ev = bevent.get();

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
          if (header->GetZero(i)) continue;

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

    int size = fBuiltEventBuffer1.size();
    ThreadSleep(fWriteSleep, perror, integral, size);
  }
}

void CupDAQManager::WriteSADC_MOD_HDF5()
{
  auto * h5event = new H5SADCEvent;
  fH5Event = h5event;

  h5event->SetBufferEventCapacity(1000);
  h5event->SetBufferMaxBytes(32 * 1024 * 1024);

  fHDF5File->SetEvent(h5event);
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

  fWriteStatus = RUNNING;
  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) break;

    if (fBuiltEventBuffer1.empty()) {
      if (fBuildStatus == ENDED || fMergeStatus == ENDED) break;
    }
    else {
      chdata.clear();

      auto bevent_opt = fBuiltEventBuffer1.pop_front();
      if (!bevent_opt.has_value()) {
        int size_empty = fBuiltEventBuffer1.size();
        ThreadSleep(fWriteSleep, perror, integral, size_empty);
        continue;
      }

      std::unique_ptr<BuiltEvent> bevent = std::move(bevent_opt.value());
      BuiltEvent * ev = bevent.get();

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
          if (header->GetZero(i)) continue;

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
    }

    int size = fBuiltEventBuffer1.size();
    ThreadSleep(fWriteSleep, perror, integral, size);
  }
}

long CupDAQManager::OpenNewHDF5File(const char * filename)
{
  long retval = 0;

  // 1. Convert to std::string for easy manipulation
  std::string filepath(filename);

  // 2. Extract BaseName (equivalent to gSystem->BaseName)
  // Find the last slash to separate directory and filename
  std::size_t slash_pos = filepath.find_last_of("/\\");
  std::string bname = (slash_pos == std::string::npos) ? filepath : filepath.substr(slash_pos + 1);

  // 3. Extract the subrun number (the last token after '.')
  int subnum = 0;
  std::size_t dot_pos = bname.find_last_of('.');

  if (dot_pos != std::string::npos && dot_pos + 1 < bname.length()) {
    // Convert substring to integer (e.g., "00001" -> 1)
    subnum = std::atoi(bname.substr(dot_pos + 1).c_str());
  }

  // 4. Proceed with HDF5 file creation
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
      ERROR("can't open output file %s", filename);
      return -1;
    }
  }

  INFO("%s opened", filename);
  return retval;
}

// =====================================================================
// HDF5 Disabled Implementation (Stub Functions)
// =====================================================================
#else

void CupDAQManager::WriteFADC_MOD_HDF5()
{
  ERROR("HDF5 is not enabled in this build. Cannot write FADC data.");
  RUNSTATE::SetError(fRunStatus); // RunStatus 에러 처리 추가 권장
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