#include <cstdlib>
#include <string>

#include "TBranchRef.h"

#include "DAQ/CupDAQManager.hh"
#include "DAQUtils/ELog.hh"
#include "OnlObjs/ADCHeader.hh"
#include "OnlObjs/FADCRawChannel.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"
#include "RawObjs/AChannel.hh"
#include "RawObjs/AChannelData.hh"
#include "RawObjs/EventInfo.hh"
#include "RawObjs/FChannel.hh"
#include "RawObjs/FChannelData.hh"

void CupDAQManager::WriteFADC_MOD_ROOT()
{
  fROOTFile->cd();
  auto * eventinfo = new EventInfo();
  fROOTTree->Branch("EventInfo", &eventinfo);
  auto * chdata = new FChannelData();
  fROOTTree->Branch("FChannelData", &chdata);

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
      chdata->Clear();

      auto bevent_opt = fBuiltEventBuffer1.pop_front();
      if (!bevent_opt.has_value()) {
        int size_empty = fBuiltEventBuffer1.size();
        ThreadSleep(fWriteSleep, perror, integral, size_empty);
        continue;
      }
      std::shared_ptr<BuiltEvent> bevent_ptr = bevent_opt.value();
      BuiltEvent * ev = bevent_ptr.get();

      eventinfo->SetTriggerNumber(ev->GetTriggerNumber());
      eventinfo->SetTriggerTime(ev->GetTriggerTime());
      eventinfo->SetTriggerType(ev->GetTriggerType());
      eventinfo->SetEventNumber(fNBuiltEvent);

      int nadc = ev->GetEntries();
      for (int j = 0; j < nadc; j++) {
        auto * adcraw = static_cast<FADCRawEvent *>(ev->At(j));
        auto * header = adcraw->GetADCHeader();

        AbsConf * conf = fConfigList->FindConfig(fADCType, header->GetMID());
        if (!conf) {
          ERROR("no config for mid=%d", header->GetMID());
          RUNSTATE::SetError(fRunStatus);
          break;
        }

        for (int i = 0; i < nadcch; i++) {
          if (header->GetZero(i)) continue;

          FChannel * channel = chdata->Add(conf->PID(i), adcraw->GetNDP());
          channel->SetPedestal(header->GetPedestal(i));
          auto * rawchannel = adcraw->GetChannel(i);
          channel->SetWaveform(rawchannel->GetADC());
          channel->SetBit(header->GetTriggerBit(i));
        }
      }

      wlock.lock();
      fROOTTree->Fill();
      wlock.unlock();

      StopBenchmark("WriteEvent");
    }

    int size = fBuiltEventBuffer1.size();
    ThreadSleep(fWriteSleep, perror, integral, size);
  }
}

void CupDAQManager::WriteSADC_MOD_ROOT()
{
  fROOTFile->cd();
  auto * eventinfo = new EventInfo();
  fROOTTree->Branch("EventInfo", &eventinfo);
  auto * chdata = new AChannelData();
  fROOTTree->Branch("AChannelData", &chdata);

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
      StartBenchmark("WriteEvent");
      chdata->Clear();

      auto bevent_opt = fBuiltEventBuffer1.pop_front();
      if (!bevent_opt.has_value()) {
        int size_empty = fBuiltEventBuffer1.size();
        ThreadSleep(fWriteSleep, perror, integral, size_empty);
        continue;
      }
      std::shared_ptr<BuiltEvent> bevent_ptr = bevent_opt.value();
      BuiltEvent * ev = bevent_ptr.get();

      eventinfo->SetTriggerNumber(ev->GetTriggerNumber());
      eventinfo->SetTriggerTime(ev->GetTriggerTime());
      eventinfo->SetTriggerType(ev->GetTriggerType());
      eventinfo->SetEventNumber(fNBuiltEvent);

      int nadc = ev->GetEntries();
      for (int j = 0; j < nadc; j++) {
        auto * adcraw = static_cast<SADCRawEvent *>(ev->At(j));
        auto * header = adcraw->GetADCHeader();

        AbsConf * conf = fConfigList->FindConfig(fADCType, header->GetMID());
        if (!conf) {
          ERROR("no config for mid=%d", header->GetMID());
          RUNSTATE::SetError(fRunStatus);
          break;
        }

        for (int i = 0; i < nadcch; i++) {
          if (header->GetZero(i)) continue;

          AChannel * channel = chdata->Add(conf->PID(i));
          channel->SetBit(header->GetTriggerBit(i));
          channel->SetADC(adcraw->GetADC(i));
          channel->SetTime(adcraw->GetTime(i));
        }
      }

      wlock.lock();
      fROOTTree->Fill();
      wlock.unlock();

      StopBenchmark("WriteEvent");
    }

    int size = fBuiltEventBuffer1.size();
    ThreadSleep(fWriteSleep, perror, integral, size);
  }
}

long CupDAQManager::OpenNewROOTFile(const char * filename)
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
    fROOTFile = new TFile(filename, "recreate", "", fCompressionLevel);
    if (!fROOTFile->IsOpen()) {
      ERROR("can't open output file %s", filename);
      return -1;
    }
    fROOTTree = new TTree("AbsEvent", "AbsEvent");
  }
  else {

    fROOTFile->cd();
    fROOTTree->Write();
    fROOTTree->Reset();

    auto * newfile = new TFile(filename, "recreate", "", fCompressionLevel);
    if (!newfile->IsOpen()) {
      ERROR("can't open output file %s", filename);
      return -1;
    }

    retval = SwitchRootFile(fROOTFile, newfile);
  }

  INFO("%s opened", filename);
  return retval;
}

long CupDAQManager::SwitchRootFile(TFile *& oldfile, TFile * newfile)
{
  long fsize = oldfile->GetEND();

  TBranch * branch = nullptr;
  TObject * obj = nullptr;

  while ((obj = oldfile->GetList()->First())) {
    oldfile->Remove(obj);

    if (obj->InheritsFrom(TTree::Class())) {
      auto * t = static_cast<TTree *>(obj);
      t->SetDirectory(newfile);

      TIter nextb(t->GetListOfBranches());
      while ((branch = static_cast<TBranch *>(nextb()))) {
        branch->SetFile(newfile);
      }

      if (t->GetBranchRef()) t->GetBranchRef()->SetFile(newfile);
      continue;
    }

    if (newfile) newfile->Append(obj);
    oldfile->Remove(obj);
  }

  delete oldfile;
  oldfile = newfile;

  return fsize;
}
