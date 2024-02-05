#include "OnlObjs/FADCRawEvent.hh"
#include "OnlObjs/FADCRawChannel.hh"
#include "DAQConfig/FADCTConf.hh"
#include "DAQConfig/GADCTConf.hh"
#include "DAQConfig/IADCTConf.hh"
#include "DAQConfig/MADCSConf.hh"

#include <iostream>
using namespace std;

ClassImp(FADCRawEvent)

FADCRawEvent::FADCRawEvent()
    : AbsADCRaw()
{
  fNCH = 0;
  fNDP = 0;
  fChannel = nullptr;
  fMode = ADC::FMODE;  
}

FADCRawEvent::FADCRawEvent(int n, int s, ADC::TYPE type)
    : AbsADCRaw(s, type)
{
  switch (type) {
    case ADC::FADC: fNCH = kNCHFADC; break;
    case ADC::GADC: fNCH = kNCHGADC; break;
    case ADC::MADC: fNCH = kNCHMADC; break;
    case ADC::IADC: fNCH = kNCHIADC; break;
    default: break;
  }

  fNDP = n;
  fChannel = new FADCRawChannel*[fNCH];
  for (int i = 0; i < fNCH; i++)
    fChannel[i] = new FADCRawChannel(fNDP);
  fMode = ADC::FMODE;
}

FADCRawEvent::FADCRawEvent(const FADCRawEvent & raw)
    : AbsADCRaw(raw)
{
  fNCH = raw.GetNCH();
  fNDP = raw.GetNDP();
  fChannel = new FADCRawChannel*[fNCH];
  for (int i = 0; i < fNCH; i++) {
    FADCRawChannel * ch = raw.GetChannel(i);
    fChannel[i] = new FADCRawChannel(*ch);
  }
  fMode = ADC::FMODE;
}

FADCRawEvent::~FADCRawEvent()
{
  if (fChannel) {
    for (int i = 0; i < fNCH; i++)
      delete fChannel[i];
    delete[] fChannel;
  }
}

void FADCRawEvent::Unpack(AbsConf * config, int verbose)
{
  switch (fType) {
    case ADC::FADC: Unpack_FADC(config, verbose); break;
    case ADC::GADC: Unpack_GADC(config, verbose); break;
    case ADC::MADC: Unpack_MADC(config, verbose); break;
    case ADC::IADC: Unpack_IADC(config, verbose); break;
    default: break;
  }
}


void FADCRawEvent::Unpack_FADC(AbsConf * config, int verbose)
{
  auto * conf = (FADCTConf *)config;

  // record length is the same for all modules
  unsigned int dlength = 128 * conf->RL();
  // int ndp = (dlength-32)/2;

  unsigned short itmp;
  for (int i = 0; i < kNCHFADC; i++) {
    UnpackHeader_FADC(i, fHeader);

    if (verbose > 1) { fHeader->Print(); }

    // check USB endpoint garbage fData for ending run safely
    if (fHeader->GetDataLength() != dlength) {
      fHeader->SetError();
      return;
    }

    // skip unused channel
    if (conf->PID(i) == 0) {
      fHeader->SetZero(i);
      continue;
    }

    int pol = conf->POL(fHeader->GetCID() - 1);
    if (pol == 0) { fHeader->SetPedestal(i, 4096 - fHeader->GetPedestal(i)); }

    //
    // get ADC value
    //
    for (int j = 0; j < fNDP; j++) {
      unsigned short adc  = (unsigned short)(fData[4 * (32 + j * 2) + i] & 0xFF);
      itmp = (unsigned short)(fData[4 * (32 + j * 2 + 1) + i] & 0xFF);
      adc += itmp << 8;
      if (pol == 0) { adc = 4096 - adc; }
      fChannel[i]->SetADC(j, adc);      
    }
  }
}

void FADCRawEvent::UnpackHeader_FADC(int n, ADCHeader * header)
{
  unsigned int itmp;
  unsigned long ltmp, finetime, coarsetime;

  // get fData length
  unsigned int dlen = fData[0 + n] & 0xFF;
  itmp = fData[4 + n] & 0xFF;
  dlen += (unsigned int)(itmp << 8);
  itmp = fData[8 + n] & 0xFF;
  dlen += (unsigned int)(itmp << 16);
  itmp = fData[12 + n] & 0xFF;
  dlen += (unsigned int)(itmp << 24);
  header->SetDataLength(dlen);

  // get ped
  unsigned int ped = fData[16 + n] & 0xFF;
  itmp = fData[20 + n] & 0x0F;
  ped += (unsigned int)(itmp << 8);
  header->SetPedestal(n, ped);

  // get run number
  // itmp = fData[20 + n] & 0xF0;
  // unsigned int runnum = (unsigned int)(itmp >> 4);

  // get trigger type
  unsigned int ttype = fData[24 + n] & 0x0F;
  header->SetTriggerType(ttype);

  // get trigger destination
  // itmp = fData[24 + n] & 0xF0;
  // unsigned int tdest = itmp >> 4;

  // get trigger number
  unsigned int tnum = fData[28 + n] & 0xFF;
  itmp = fData[32 + n] & 0xFF;
  tnum += (unsigned int)(itmp << 8);
  itmp = fData[36 + n] & 0xFF;
  tnum += (unsigned int)(itmp << 16);
  itmp = fData[40 + n] & 0xFF;
  tnum += (unsigned int)(itmp << 24);
  tnum += 1;
  header->SetTriggerNumber(tnum);

  // get trigger fine time
  finetime = fData[44 + n] & 0xFF;
  finetime = finetime * 8;

  // get trigger coarse time
  ltmp = fData[48 + n] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = fData[52 + n] & 0xFF;
  ltmp = ltmp << 8;
  coarsetime += ltmp * 1000;
  ltmp = fData[56 + n] & 0xFF;
  ltmp = ltmp << 16;
  coarsetime += ltmp * 1000;

  header->SetTriggerTime(coarsetime + finetime);

  // get module id
  int mid = fData[60 + n] & 0xFF;
  header->SetMID(mid);

  // get channel id
  int cid = fData[64 + n] & 0xFF;
  header->SetCID(cid);

  // get local trigger number
  unsigned int ctnum = fData[68 + n] & 0xFF;
  itmp = fData[72 + n] & 0xFF;
  ctnum += (unsigned int)(itmp << 8);
  itmp = fData[76 + n] & 0xFF;
  ctnum += (unsigned int)(itmp << 16);
  itmp = fData[80 + n] & 0xFF;
  ctnum += (unsigned int)(itmp << 24);
  ctnum += 1;
  header->SetLocalTriggerNumber(ctnum);

  // get loc trigger pattern
  unsigned int ctptn = fData[84 + n] & 0xFF;
  itmp = fData[88 + n] & 0xFF;
  ctptn += (unsigned int)(itmp << 8);
  itmp = fData[92 + n] & 0xFF;
  ctptn += (unsigned int)(itmp << 16);
  itmp = fData[96 + n] & 0xFF;
  ctptn += (unsigned int)(itmp << 24);
  header->SetLocalTriggerPattern(ctptn);

  // get loc starting fine time
  finetime = fData[100 + n] & 0xFF;
  finetime = finetime * 8;

  // get loc starting coarse time
  ltmp = fData[104 + n] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = (unsigned long)(fData[108 + n] & 0xFF) << 8;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[112 + n] & 0xFF) << 16;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[116 + n] & 0xFF) << 24;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[120 + n] & 0xFF) << 32;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[124 + n] & 0xFF) << 40;
  coarsetime += ltmp * 1000;

  header->SetLocalTriggerTime(coarsetime + finetime);

  unsigned int tbit = (ctptn >> 4 * n) & 0x0F;
  if (tbit > 0) header->SetTriggerBit(n);
}

void FADCRawEvent::Unpack_GADC(AbsConf * config, int verbose)
{
  auto * conf = (GADCTConf *)config;

  // record length is the same for all modules
  unsigned int dlength = 512 * conf->RL();
  // int ndp = (dlength-32)/2;

  UnpackHeader_GADC(fHeader);
  if (verbose > 1) { fHeader->Print(); }

  // check USB endpoint garbage data for ending run safely
  if (fHeader->GetDataLength() != dlength) {
    fHeader->SetError();
    return;
  }

  unsigned short itmp;
  for (int i = 0; i < kNCHGADC; i++) {
    // skip unused channel
    if (conf->PID(i) == 0) {
      fHeader->SetZero(i);
      continue;
    }

    int pol = conf->POL(i);
    if (pol == 0) { fHeader->SetPedestal(i, 16348 - fHeader->GetPedestal(i)); }

    //
    // get ADC value
    //
    for (int j = 0; j < fNDP; j++) {
      unsigned short adc = (unsigned short)(fData[64 + j * 32 + i * 2] & 0xFF);
      itmp = (unsigned short)(fData[64 + j * 32 + i * 2 + 1] & 0xFF);
      adc += itmp << 8;
      if (pol == 0) { adc = 16384 - adc; }
      fChannel[i]->SetADC(j, adc);
    }
  }
}

void FADCRawEvent::UnpackHeader_GADC(ADCHeader * header)
{
  unsigned int itmp;
  unsigned long ltmp, finetime, coarsetime;

  // get fData length
  unsigned int dlen = fData[0] & 0xFF;
  itmp = fData[1] & 0xFF;
  dlen += (unsigned int)(itmp << 8);
  itmp = fData[2] & 0xFF;
  dlen += (unsigned int)(itmp << 16);
  itmp = fData[3] & 0xFF;
  dlen += (unsigned int)(itmp << 24);
  header->SetDataLength(dlen);

  // get trigger type
  unsigned int ttype = fData[4] & 0x0F;
  header->SetTriggerType(ttype);

  // get trigger number
  unsigned int tnum = fData[7] & 0xFF;
  itmp = fData[8] & 0xFF;
  tnum += (unsigned int)(itmp << 8);
  itmp = fData[9] & 0xFF;
  tnum += (unsigned int)(itmp << 16);
  itmp = fData[10] & 0xFF;
  tnum += (unsigned int)(itmp << 24);
  tnum += 1;
  header->SetTriggerNumber(tnum);

  // get trigger fine time
  finetime = fData[11] & 0xFF;
  finetime = finetime * 8;

  // get trigger coarse time
  ltmp = fData[12] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = fData[13] & 0xFF;
  ltmp = ltmp << 8;
  coarsetime += ltmp * 1000;
  ltmp = fData[14] & 0xFF;
  ltmp = ltmp << 16;
  coarsetime += ltmp * 1000;

  header->SetTriggerTime(coarsetime + finetime);

  // get module id
  int mid = fData[15] & 0xFF;
  header->SetMID(mid);

  // get channel id
  // int cid = fData[64] & 0xFF;memset(fADC, 0, fNDP*sizeof(unsigned short));
  // header->SetCId(cid);

  // get local trigger number
  unsigned int ctnum = fData[17] & 0xFF;
  itmp = fData[18] & 0xFF;
  ctnum += (unsigned int)(itmp << 8);
  itmp = fData[19] & 0xFF;
  ctnum += (unsigned int)(itmp << 16);
  itmp = fData[20] & 0xFF;
  ctnum += (unsigned int)(itmp << 24);
  ctnum += 1;
  header->SetLocalTriggerNumber(ctnum);

  // get loc trigger pattern
  unsigned int ctptn = fData[21] & 0xFF;
  itmp = fData[22] & 0xFF;
  ctptn += (unsigned int)(itmp << 8);
  itmp = fData[23] & 0xFF;
  ctptn += (unsigned int)(itmp << 16);
  itmp = fData[24] & 0xFF;
  ctptn += (unsigned int)(itmp << 24);
  header->SetLocalTriggerPattern(ctptn);

  // get loc starting fine time
  finetime = fData[25] & 0xFF;
  finetime = finetime * 8;

  // get loc starting coarse time
  ltmp = fData[26] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = (unsigned long)(fData[27] & 0xFF) << 8;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[28] & 0xFF) << 16;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[29] & 0xFF) << 24;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[30] & 0xFF) << 32;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[31] & 0xFF) << 40;
  coarsetime += ltmp * 1000;

  header->SetLocalTriggerTime(coarsetime + finetime);

  // get pedestal & trigger bit
  for (int i = 0; i < kNCHGADC; i++) {
    unsigned int ped = fData[32 + i * 2] & 0xFF;
    itmp = fData[32 + i * 2 + 1] & 0xFF;
    ped += (unsigned int)(itmp << 8);
    header->SetPedestal(i, ped);

    unsigned int tbit = (ctptn >> 4 * i) & 0x0F;
    if (tbit > 0) header->SetTriggerBit(i);
  }
}

void FADCRawEvent::Unpack_MADC(AbsConf * config, int verbose)
{
  auto * conf = (MADCSConf *)config;

  // record length is the same for all modules
  unsigned int dlength = 32 * conf->RL();

  unsigned short itmp;
  for (int i = 0; i < kNCHMADC; i++) {
    UnpackHeader_MADC(i, fHeader);

    if (verbose > 1) { fHeader->Print(); }

    // check USB endpoint garbage fData for ending run safely
    if (fHeader->GetDataLength() != dlength) {
      fHeader->SetError();
      return;
    }

    // skip unused channel
    if (conf->PID(i) == 0) {
      fHeader->SetZero(i);
      continue;
    }

    int pol = conf->POL(fHeader->GetCID() - 1);
    if (pol == 0) { fHeader->SetPedestal(i, 16384 - fHeader->GetPedestal(i)); }

    //
    // get ADC value
    //
    for (int j = 0; j < fNDP; j++) {
      unsigned short adc = (unsigned short)(fData[4 * (32 + j * 2) + i] & 0xFF);
      itmp = (unsigned short)(fData[4 * (32 + j * 2 + 1) + i] & 0xFF);
      adc += itmp << 8;
      if (pol == 0) { adc = 16384 - adc; }
      fChannel[j]->SetADC(j, adc);
    }
  }
}

void FADCRawEvent::UnpackHeader_MADC(int n, ADCHeader * header)
{
  unsigned int itmp;
  unsigned long ltmp, finetime, coarsetime;

  // get fData length
  unsigned int dlen = fData[0 + n] & 0xFF;
  itmp = fData[4 + n] & 0xFF;
  dlen += (unsigned int)(itmp << 8);
  itmp = fData[8 + n] & 0xFF;
  dlen += (unsigned int)(itmp << 16);
  itmp = fData[12 + n] & 0xFF;
  dlen += (unsigned int)(itmp << 24);
  header->SetDataLength(dlen);

  // get ped
  unsigned int ped = fData[16 + n] & 0xFF;
  itmp = fData[20 + n] & 0x0F;
  ped += (unsigned int)(itmp << 8);
  header->SetPedestal(n, ped);

  // get run number
  // itmp = fData[20 + n] & 0xF0;
  // unsigned int runnum = (unsigned int)(itmp >> 4);

  // get trigger type
  unsigned int ttype = fData[24 + n] & 0x0F;
  header->SetTriggerType(ttype);

  // get trigger destination
  // itmp = fData[24 + n] & 0xF0;
  // unsigned int tdest = itmp >> 4;

  // get trigger number
  unsigned int tnum = fData[28 + n] & 0xFF;
  itmp = fData[32 + n] & 0xFF;
  tnum += (unsigned int)(itmp << 8);
  itmp = fData[36 + n] & 0xFF;
  tnum += (unsigned int)(itmp << 16);
  itmp = fData[40 + n] & 0xFF;
  tnum += (unsigned int)(itmp << 24);
  tnum += 1;
  header->SetTriggerNumber(tnum);

  // get trigger fine time
  finetime = fData[44 + n] & 0xFF;
  finetime = finetime * 8;

  // get trigger coarse time
  ltmp = fData[48 + n] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = fData[52 + n] & 0xFF;
  ltmp = ltmp << 8;
  coarsetime += ltmp * 1000;
  ltmp = fData[56 + n] & 0xFF;
  ltmp = ltmp << 16;
  coarsetime += ltmp * 1000;

  header->SetTriggerTime(coarsetime + finetime);

  // get module id
  int mid = fData[60 + n] & 0xFF;
  header->SetMID(mid);

  // get channel id
  int cid = fData[64 + n] & 0xFF;
  header->SetCID(cid);

  // get local trigger number
  unsigned int ctnum = fData[68 + n] & 0xFF;
  itmp = fData[72 + n] & 0xFF;
  ctnum += (unsigned int)(itmp << 8);
  itmp = fData[76 + n] & 0xFF;
  ctnum += (unsigned int)(itmp << 16);
  itmp = fData[80 + n] & 0xFF;
  ctnum += (unsigned int)(itmp << 24);
  ctnum += 1;
  header->SetLocalTriggerNumber(ctnum);

  // get loc trigger pattern
  unsigned int ctptn = fData[84 + n] & 0xFF;
  itmp = fData[88 + n] & 0xFF;
  ctptn += (unsigned int)(itmp << 8);
  itmp = fData[92 + n] & 0xFF;
  ctptn += (unsigned int)(itmp << 16);
  itmp = fData[96 + n] & 0xFF;
  ctptn += (unsigned int)(itmp << 24);
  header->SetLocalTriggerPattern(ctptn);

  // get loc starting fine time
  finetime = fData[100 + n] & 0xFF;
  finetime = finetime * 8;

  // get loc starting coarse time
  ltmp = fData[104 + n] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = (unsigned long)(fData[108 + n] & 0xFF) << 8;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[112 + n] & 0xFF) << 16;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[116 + n] & 0xFF) << 24;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[120 + n] & 0xFF) << 32;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[124 + n] & 0xFF) << 40;
  coarsetime += ltmp * 1000;

  header->SetLocalTriggerTime(coarsetime + finetime);

  unsigned int tbit = (ctptn >> 4 * n) & 0x0F;
  if (tbit > 0) header->SetTriggerBit(n);
}

void FADCRawEvent::Unpack_IADC(AbsConf * config, int verbose)
{
  auto * conf = (IADCTConf *)config;

  UnpackHeader_IADC(fHeader);

  for (int i = 0; i < kNCHIADC; i++) {
    if (conf->PID(i) == 0) { fHeader->SetZero(i); }
  }

  if (verbose > 1) { fHeader->Print(); }

  unsigned short stmp;
  for (int j = 0; j < fNDP; j++) {
    for (int k = 0; k < kNCHIADC / 2; k++) {
      unsigned short adc0 = fData[128 + j * 64 + 3 * k] & 0xFF;
      stmp = fData[128 + j * 64 + 3 * k + 1] & 0x0F;
      stmp = stmp << 8;
      adc0 += stmp;
      stmp = fData[128 + j * 64 + 3 * k + 1] & 0xF0;
      unsigned short adc1 = stmp >> 4;
      stmp = fData[128 + j * 64 + 3 * k + 2] & 0xFF;
      stmp = stmp << 4;
      adc1 += stmp;
      fChannel[2 * k]->SetADC(j, adc0);
      fChannel[2 * k + 1]->SetADC(j, adc1);
    }
  }
}

void FADCRawEvent::UnpackHeader_IADC(ADCHeader * header)
{
  unsigned int itmp;
  unsigned long ltmp, finetime, coarsetime;

  // get fData length
  unsigned int dlen = fData[0] & 0xFF;
  itmp = fData[1] & 0xFF;
  dlen += (unsigned int)(itmp << 8);
  itmp = fData[2] & 0xFF;
  dlen += (unsigned int)(itmp << 16);
  itmp = fData[3] & 0xFF;
  dlen += (unsigned int)(itmp << 24);
  header->SetDataLength(dlen);

  // get trigger type
  unsigned int ttype = fData[6] & 0x0F;
  header->SetTriggerType(ttype);

  // get trigger number
  unsigned int tnum = fData[7] & 0xFF;
  itmp = fData[8] & 0xFF;
  tnum += (unsigned int)(itmp << 8);
  itmp = fData[9] & 0xFF;
  tnum += (unsigned int)(itmp << 16);
  itmp = fData[10] & 0xFF;
  tnum += (unsigned int)(itmp << 24);
  tnum += 1;
  header->SetTriggerNumber(tnum);

  // get trigger fine time
  finetime = fData[11] & 0xFF;
  finetime = finetime * 8;

  // get trigger coarse time
  ltmp = fData[12] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = (unsigned long)(fData[13] & 0xFF) << 8;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[14] & 0xFF) << 16;
  coarsetime += ltmp * 1000;
  header->SetTriggerTime(coarsetime + finetime);

  // get module id
  int mid = fData[15] & 0xFF;
  header->SetMID(mid);

  // get local trigger number
  unsigned int ctnum = fData[17] & 0xFF;
  itmp = fData[18] & 0xFF;
  ctnum += (unsigned int)(itmp << 8);
  itmp = fData[19] & 0xFF;
  ctnum += (unsigned int)(itmp << 16);
  itmp = fData[20] & 0xFF;
  ctnum += (unsigned int)(itmp << 24);
  ctnum += 1;
  header->SetLocalTriggerNumber(ctnum);

  // get local trigger pattern
  unsigned int ctptn = fData[21] & 0xFF;
  itmp = fData[22] & 0xFF;
  ctptn += (unsigned int)(itmp << 8);
  itmp = fData[23] & 0xFF;
  ctptn += (unsigned int)(itmp << 16);
  itmp = fData[24] & 0xFF;
  ctptn += (unsigned int)(itmp << 24);
  header->SetLocalTriggerPattern(ctptn);

  for (int i = 0; i < kNCHIADC; i++) {
    int k = i / 4;
    unsigned int tbit = (unsigned int)TESTBIT(ctptn, k);
    if (tbit > 0) header->SetTriggerBit(i);
  }

  // get local starting fine time
  finetime = fData[25] & 0xFF;
  finetime = finetime * 8;

  // get local starting coarse time
  ltmp = fData[26] & 0xFF;
  coarsetime = ltmp * 1000;
  ltmp = (unsigned long)(fData[27] & 0xFF) << 8;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[28] & 0xFF) << 16;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[29] & 0xFF) << 24;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[30] & 0xFF) << 32;
  coarsetime += ltmp * 1000;
  ltmp = (unsigned long)(fData[31] & 0xFF) << 40;
  coarsetime += ltmp * 1000;
  header->SetLocalTriggerTime(coarsetime + finetime);

  for (int k = 0; k < 20; k++) {
    unsigned short ped0 = fData[64 + k * 3 + 0] & 0xFF;
    itmp = fData[64 + k * 3 + 1] & 0x0F;
    itmp = itmp << 8;
    ped0 = ped0 + itmp;
    itmp = fData[64 + k * 3 + 1] & 0xF0;
    header->SetPedestal(2 * k, ped0);
    unsigned short ped1 = itmp >> 4;
    itmp = fData[64 + k * 3 + 2] & 0xFF;
    itmp = itmp << 4;
    ped1 = ped1 + itmp;
    header->SetPedestal(2 * k + 1, ped1);
  }
}
