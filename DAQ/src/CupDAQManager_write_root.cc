#include "TBranchRef.h"
#include "TObjString.h"

#include "DAQ/CupDAQManager.hh"
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
    // for emergent exit
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) break;
    if (fBuiltEventBuffer1.empty()) {
      if (fBuildStatus == ENDED || fMergeStatus == ENDED) break;
    }
    else {    
      chdata->Clear();

      BuiltEvent * bevent = fBuiltEventBuffer1.popfront();

      unsigned long fastttime = UINT64_MAX;
      unsigned int tnum = 0;
      unsigned int ttype = 0;

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
        if (!conf) {
          fLog->Error("CupDAQManager::WriteFADC_MOD_ROOT", "no config for mid=%d", header->GetMID());
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
      eventinfo->SetTriggerNumber(tnum);
      eventinfo->SetTriggerTime(fastttime);
      eventinfo->SetTriggerType(ttype);

      delete bevent;

      wlock.lock();
      fROOTTree->Fill();
      wlock.unlock();
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
    // for emergent exit
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) break;
    if (fBuiltEventBuffer1.empty()) {
      if (fBuildStatus == ENDED || fMergeStatus == ENDED) break;
    }
    else {
      chdata->Clear();

      BuiltEvent * bevent = fBuiltEventBuffer1.popfront();

      unsigned long fastttime = UINT64_MAX;
      unsigned int tnum = 0;
      unsigned int ttype = 0;

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

          AChannel * channel = chdata->Add(conf->PID(i));
          channel->SetBit(header->GetTriggerBit(i));
          channel->SetADC(adcraw->GetADC(i));
          channel->SetTime(adcraw->GetTime(i));
        }
      }
      eventinfo->SetTriggerNumber(tnum);
      eventinfo->SetTriggerTime(fastttime);
      eventinfo->SetTriggerType(ttype);

      delete bevent;

      wlock.lock();
      fROOTTree->Fill();
      wlock.unlock();
    }

    int size = fBuiltEventBuffer1.size();
    ThreadSleep(fWriteSleep, perror, integral, size);
  }
}

long CupDAQManager::OpenNewROOTFile(const char * filename)
{
  long retval = 0;

  TString bname = gSystem->BaseName(filename);
  TObjArray * objs = bname.Tokenize(".");
  int subnum =
      TString(((TObjString *)objs->At(objs->GetEntries() - 1))->GetName())
          .Atoi();

  if (subnum == 0) {
    fROOTFile = new TFile(filename, "recreate", "", fCompressionLevel);
    if (!fROOTFile->IsOpen()) {
      fLog->Error("CupDAQManager::OpenNewROOTFile",
                  "can\'t open output file %s", filename);
      return -1;
    }
    fROOTTree = new TTree("AbsEvent", "AbsEvent");
  }
  else {
    fROOTFile->cd();
    fROOTTree->Write();
    fROOTTree->Reset();

    auto * newfile = new TFile(filename, "recreate", "", fCompressionLevel);
    if (!fROOTFile->IsOpen()) {
      fLog->Error("CupDAQManager::OpenNewROOTFile",
                  "can\'t open output file %s", filename);
      return -1;
    }

    retval = SwitchRootFile(fROOTFile, newfile);
  }

  fLog->Info("CupDAQManager::OpenNewROOTFile", "%s opened", filename);
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
      auto * t = (TTree *)obj;
      t->SetDirectory(newfile);

      TIter nextb(t->GetListOfBranches());
      while ((branch = (TBranch *)nextb())) {
        branch->SetFile(newfile);
      }

      if (t->GetBranchRef()) { t->GetBranchRef()->SetFile(newfile); }
      continue;
    }

    if (newfile) newfile->Append(obj);
    oldfile->Remove(obj);
  }

  delete oldfile;
  oldfile = nullptr;
  oldfile = newfile;

  return fsize;
}
