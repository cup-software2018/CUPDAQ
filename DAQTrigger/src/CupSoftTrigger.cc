//
// Created by cupsoft on 6/30/20.
//
#include <fstream>
#include <sstream>

#include "TRandom3.h"

#include "DAQConfig/STRGConf.hh"
#include "DAQTrigger/CupSoftTrigger.hh"
#include "OnlObjs/FADCRawChannel.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"

using namespace std;

CupSoftTrigger::CupSoftTrigger()
    : AbsSoftTrigger()
{
  fDoZSUTBit = false;
  fDoZSUQSum = false;
  fDoPrescale = false;
  fDoZSUPHeight = false;
  fPrescale = 1;

  fQsumArray = new TObjArray();
  gRandom->SetSeed(0);
}

CupSoftTrigger::CupSoftTrigger(AbsConf * config)
    : AbsSoftTrigger(config)
{
  fDoZSUTBit = false;
  fDoZSUQSum = false;
  fDoPrescale = false;
  fDoZSUPHeight = false;
  fPrescale = 1;

  fQsumArray = new TObjArray();
  gRandom->SetSeed(0);
}

CupSoftTrigger::~CupSoftTrigger() {}

void CupSoftTrigger::InitTrigger()
{
  auto * config = (STRGConf *)fConfig;
  int mode = config->GetZSUMode();

  fDoZSUTBit = mode & (1 << 0);    // 1: tbit zero suppression
  fDoZSUQSum = mode & (1 << 1);    // 2: qsum zero suppression
  fDoZSUPHeight = mode & (1 << 2); // 4: pulse height
  fPrescale = config->GetPrescale();

  if (fPrescale > 1) fDoPrescale = true;

  string line;
  ifstream file(config->GetInputCard());
  if (file.is_open()) {
    while (file.good()) {
      getline(file, line);
      if (line.empty()) continue;

      istringstream iss(line);
      string key;

      iss >> key;
      if (iss.fail() || key.length() == 0 || (key.data())[0] == '#') continue;

      if (key == "mid" || key == "MID") {
        int mid, n, m, val;
        iss >> mid >> n >> m;
        Qsum * qsum = new Qsum(mid, n, m);
        for (int i = 0; i < n; i++) {
          getline(file, line);
          istringstream iss1(line);
          for (int j = 0; j < m; j++) {
            iss1 >> val;
            qsum->fCut[m * i + j] = val;
          }
        }
        // qsum->Print();
        fQsumArray->Add(qsum);
      }
    }
  }
  else {
    fLog->Warning("InitTrigger", "no input file %s", config->GetInputCard());
  }

  fLog->Info("InitTrigger", "NADC=%d, TBIT=%d, QSUM=%d, PSC=%d",
             fQsumArray->GetEntries(), fDoZSUTBit, fDoZSUQSum, fDoPrescale);
}

bool CupSoftTrigger::DoTrigger(BuiltEvent * bevent)
{
  fTotalInputEvent += 1;
  bool state = false;

  if (fPrescale > 1) {
    if (gRandom->Rndm() > 1 / (double)fPrescale) { return false; }
  }

  switch (fMode) {
    case ADC::FMODE: state = DoTriggerFADC(bevent); break;
    case ADC::SMODE: state = DoTriggerSADC(bevent); break;
    default: break;
  }

  if (state) { fNTriggeredEvent += 1; }

  return state;
}

bool CupSoftTrigger::DoTriggerFADC(BuiltEvent * bevent)
{
  int noverqsum = 0;
  int nbit = 0;
  int zero;

  ADCHeader * header = nullptr;
  int nadc = bevent->GetEntries();
  for (int k = 0; k < nadc; k++) {
    auto * adcraw = (FADCRawEvent *)bevent->At(k);

    header = adcraw->GetADCHeader();
    if (header->GetTriggerType() == 1) { return true; }

    int mid = header->GetMID();
    Qsum * qsumthr = FindQsum(mid);

    for (int i = 0; i < adcraw->GetNCH(); i++) {
      zero = 0;
      if (header->GetZero(i)) continue; // unused channel

      if (fDoZSUTBit) {
        if (header->GetTriggerBit(i) == 0) { zero += 1; }
        else nbit += 1;
      }

      int ped = header->GetPedestal(i);
      FADCRawChannel * ch = adcraw->GetChannel(i);
      unsigned short * adc = ch->GetADC();

      if (fDoZSUQSum && qsumthr) {
        int qsum = 0;
        for (int j = 0; j < ch->GetNDP(); j++) {
          qsum += int(adc[j]) - ped;
        }

        if (qsum < qsumthr->fCut[i]) header->SetZero(i);
        else noverqsum += 1;
      }

      if (fDoZSUPHeight) {
        int j;
        for (j = 0; j < ch->GetNDP(); j++) {
          if (int(adc[j]) - ped > qsumthr->fCut[i]) {
            nbit += 1;
            break;
          }
        }
        if (j == ch->GetNDP() - 1) zero += 1;
      }

      if (zero > 0) header->SetZero(i);
    }
  }

  if (fDoZSUQSum && noverqsum == 0) return false;
  if ((fDoZSUTBit || fDoZSUPHeight) && nbit == 0) return false;
  //if (fDoZSUPHeight && zero != 0) return false;
  return true;
}

bool CupSoftTrigger::DoTriggerSADC(BuiltEvent * bevent)
{
  ADCHeader * header = nullptr;
  int nadc = bevent->GetEntries();
  for (int k = 0; k < nadc; k++) {
    auto * adcraw = (SADCRawEvent *)bevent->At(k);

    header = adcraw->GetADCHeader();
    if (header->GetTriggerType() == 1) continue; // pedestal trigger

    for (int i = 0; i < adcraw->GetNCH(); i++) {
      if (header->GetZero()) continue; // unused channel

      if (fDoZSUTBit) {
        if (header->GetTriggerBit(i) == 0) { header->SetZero(i); }
      }
    }
  }

  return true;
}

Qsum * CupSoftTrigger::FindQsum(int mid) const
{
  int n = fQsumArray->GetEntries();
  for (int i = 0; i < n; i++) {
    auto * qsum = (Qsum *)fQsumArray->At(i);
    if (qsum->fMID == mid) return qsum;
  }
  return nullptr;
}
