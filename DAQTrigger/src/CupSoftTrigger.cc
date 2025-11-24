// CupSoftTrigger.cc
#include <fstream>
#include <sstream>

#include "TRandom3.h"

#include "DAQConfig/STRGConf.hh"
#include "DAQTrigger/CupSoftTrigger.hh"
#include "DAQUtils/ELog.hh"
#include "OnlObjs/FADCRawChannel.hh"
#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/SADCRawEvent.hh"

Qsum::Qsum(int mid, int row, int col)
  : fMID(mid),
    fRow(row),
    fCol(col),
    fNCH(row * col),
    fCut(new double[fNCH])
{
}

Qsum::~Qsum() { delete[] fCut; }

void Qsum::Print(Option_t * opt) const
{
  std::cout << Form("mid %d %d %d", fMID, fRow, fCol) << std::endl;
  for (int i = 0; i < fRow; i++) {
    for (int j = 0; j < fCol; j++) {
      std::cout << fCut[fCol * i + j] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

CupSoftTrigger::CupSoftTrigger()
  : AbsSoftTrigger(),
    fDoZSUTBit(false),
    fDoZSUQSum(false),
    fDoZSUPHeight(false),
    fDoPrescale(false),
    fPHeight(0),
    fPrescale(1),
    fQsumArray(new TObjArray())
{
  gRandom->SetSeed(0);
}

CupSoftTrigger::CupSoftTrigger(AbsConf * config)
  : AbsSoftTrigger(config),
    fDoZSUTBit(false),
    fDoZSUQSum(false),
    fDoZSUPHeight(false),
    fDoPrescale(false),
    fPHeight(0),
    fPrescale(1),
    fQsumArray(new TObjArray())
{
  gRandom->SetSeed(0);
}

CupSoftTrigger::~CupSoftTrigger() = default;

void CupSoftTrigger::InitTrigger()
{
  auto * config = static_cast<STRGConf *>(fConfig);
  int mode = config->GetZSUMode();

  fDoZSUTBit = (mode & (1 << 0)) != 0;
  fDoZSUQSum = (mode & (1 << 1)) != 0;
  fDoZSUPHeight = (mode & (1 << 2)) != 0;
  fPrescale = config->GetPrescale();
  if (fPrescale > 1) fDoPrescale = true;

  std::string line;
  std::ifstream file(config->GetInputCard());
  if (file.is_open()) {
    while (std::getline(file, line)) {
      if (line.empty()) continue;

      std::istringstream iss(line);
      std::string key;
      iss >> key;
      if (iss.fail() || key.empty() || key[0] == '#') continue;

      if (key == "mid" || key == "MID") {
        int mid, n, m, val;
        iss >> mid >> n >> m;
        auto * qsum = new Qsum(mid, n, m);

        for (int i = 0; i < n; i++) {
          std::getline(file, line);
          std::istringstream iss1(line);
          for (int j = 0; j < m; j++) {
            iss1 >> val;
            qsum->fCut[m * i + j] = val;
          }
        }
        fQsumArray->Add(qsum);
      }
    }
  }
  else {
    WARNING("InitTrigger: no input file %s", config->GetInputCard());
  }

  INFO("InitTrigger: NADC=%d, TBIT=%d, QSUM=%d, PSC=%d", fQsumArray->GetEntries(), fDoZSUTBit, fDoZSUQSum, fDoPrescale);
}

bool CupSoftTrigger::DoTrigger(BuiltEvent * bevent)
{
  fTotalInputEvent += 1;

  if (fPrescale > 1) {
    if (gRandom->Rndm() > 1.0 / static_cast<double>(fPrescale)) return false;
  }

  bool state = false;
  switch (fMode) {
    case ADC::FMODE: state = DoTriggerFADC(bevent); break;
    case ADC::SMODE: state = DoTriggerSADC(bevent); break;
    default: break;
  }

  if (state) fNTriggeredEvent += 1;
  return state;
}

bool CupSoftTrigger::DoTriggerFADC(BuiltEvent * bevent)
{
  int noverqsum = 0;
  int nbit = 0;

  int nadc = bevent->GetEntries();
  for (int k = 0; k < nadc; k++) {
    auto * adcraw = static_cast<FADCRawEvent *>(bevent->At(k));
    auto * header = adcraw->GetADCHeader();

    if (header->GetTriggerType() == 1) return true;

    int mid = header->GetMID();
    Qsum * qsumthr = FindQsum(mid);

    for (int i = 0; i < adcraw->GetNCH(); i++) {
      int zero = 0;
      if (header->GetZero(i)) continue;

      if (fDoZSUTBit) {
        if (header->GetTriggerBit(i) == 0) zero += 1;
        else nbit += 1;
      }

      int ped = header->GetPedestal(i);
      auto * ch = adcraw->GetChannel(i);
      unsigned short * adc = ch->GetADC();

      if (fDoZSUQSum && qsumthr) {
        int qsum = 0;
        for (int j = 0; j < ch->GetNDP(); j++) {
          qsum += static_cast<int>(adc[j]) - ped;
        }

        if (qsum < qsumthr->fCut[i]) { header->SetZero(i); }
        else {
          noverqsum += 1;
        }
      }

      if (fDoZSUPHeight && qsumthr) {
        bool hit = false;
        for (int j = 0; j < ch->GetNDP(); j++) {
          if (static_cast<int>(adc[j]) - ped > qsumthr->fCut[i]) {
            nbit += 1;
            hit = true;
            break;
          }
        }
        if (!hit) zero += 1;
      }

      if (zero > 0) header->SetZero(i);
    }
  }

  if (fDoZSUQSum && noverqsum == 0) return false;
  if ((fDoZSUTBit || fDoZSUPHeight) && nbit == 0) return false;
  return true;
}

bool CupSoftTrigger::DoTriggerSADC(BuiltEvent * bevent)
{
  int nadc = bevent->GetEntries();
  for (int k = 0; k < nadc; k++) {
    auto * adcraw = static_cast<SADCRawEvent *>(bevent->At(k));
    auto * header = adcraw->GetADCHeader();

    if (header->GetTriggerType() == 1) continue;

    for (int i = 0; i < adcraw->GetNCH(); i++) {
      if (header->GetZero()) continue;

      if (fDoZSUTBit) {
        if (header->GetTriggerBit(i) == 0) header->SetZero(i);
      }
    }
  }

  return true;
}

Qsum * CupSoftTrigger::FindQsum(int mid) const
{
  int n = fQsumArray->GetEntries();
  for (int i = 0; i < n; i++) {
    auto * qsum = static_cast<Qsum *>(fQsumArray->At(i));
    if (qsum->fMID == mid) return qsum;
  }
  return nullptr;
}
