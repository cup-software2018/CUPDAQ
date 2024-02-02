#include "OnlObjs/SADCRawEvent.hh"
#include "DAQConfig/IADCTConf.hh"
#include "DAQConfig/SADCTConf.hh"

ClassImp(SADCRawEvent)

SADCRawEvent::SADCRawEvent()
    : AbsADCRaw()
{
  fNCH = 0;
  fADC = nullptr;
  fTime = nullptr;
  fMode = ADC::SMODE;
}

SADCRawEvent::SADCRawEvent(int s, ADC::TYPE type)
    : AbsADCRaw(s, type)
{
  switch (type) {
    case ADC::SADC: fNCH = kNCHSADC; break;
    case ADC::IADC: fNCH = kNCHIADC; break;
    default: break;
  }

  fADC = new unsigned int[fNCH];
  memset(fADC, 0, fNCH*sizeof(unsigned int));
  fTime = new unsigned int[fNCH];
  memset(fTime, 0, fNCH*sizeof(unsigned int));

  fMode = ADC::SMODE;
}

SADCRawEvent::SADCRawEvent(const SADCRawEvent & raw)
    : AbsADCRaw(raw)
{
  fNCH = raw.GetNCH();
  fADC = new unsigned int[fNCH];
  fTime = new unsigned int[fNCH];
  memcpy(fADC, raw.GetADCs(), fNCH * sizeof(unsigned int));
  memcpy(fTime, raw.GetTimes(), fNCH * sizeof(unsigned int));

  fMode = ADC::SMODE;
}

SADCRawEvent::~SADCRawEvent()
{
  if (fADC) delete[] fADC;
  if (fTime) delete[] fTime;
}

void SADCRawEvent::Unpack(AbsConf * config, int verbose)
{
  switch (fType) {
    case ADC::SADC: Unpack_SADC(config, verbose); break;
    case ADC::IADC: Unpack_IADC(config, verbose); break;
    default: break;
  }
}

void SADCRawEvent::Unpack_SADC(AbsConf * config, int verbose)
{
  auto * conf = (SADCTConf *)config;

  UnpackHeader_SADC(fHeader);

  if (verbose > 1) { fHeader->Print(); }

  // check USB endpoint garbage data for ending run safely
  if (fHeader->GetDataLength() != kBYTESPEREVENTSADC) {
    fHeader->SetError();
    return;
  }

  unsigned int itmp, peak, peaktime;
  for (int i = 0; i < kNCHSADC; i++) {
    if (conf->PID(i) == 0) {
      fHeader->SetZero(i);
      continue;
    }

    peak = fData[32 + i * 7] & 0xFF;
    itmp = fData[32 + i * 7 + 1] & 0xFF;
    peak = peak + (int)(itmp << 8);
    itmp = fData[32 + i * 7 + 2] & 0xFF;
    peak = peak + (int)(itmp << 16);
    fADC[i] = peak;

    peaktime = fData[32 + i * 7 + 3] & 0xFF;
    peaktime = peaktime * 8;
    itmp = fData[32 + i * 7 + 4] & 0xFF;
    peaktime = peaktime + itmp * 1000;
    itmp = (int)(fData[32 + i * 7 + 5] & 0xFF) << 8;
    peaktime = peaktime + itmp * 1000;
    itmp = (int)(fData[32 + i * 7 + 6] & 0xFF) << 16;
    peaktime = peaktime + itmp * 1000;
    fTime[i] = peaktime;
  }
}

void SADCRawEvent::UnpackHeader_SADC(ADCHeader * header)
{
  unsigned int itmp;
  unsigned long ltmp, finetime, coarsetime;

  // get data length
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

  // get channel id
  // int cid = fData[16] & 0xFF;

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

  for (int i = 0; i < kNCHSADC; i++) {
    unsigned int tbit = ctptn & (1 << i);
    if (tbit > 0) header->SetTriggerBit(i);
  }
}

void SADCRawEvent::Unpack_IADC(AbsConf * config, int verbose)
{
  auto * conf = (IADCTConf *)config;

  UnpackHeader_IADC(fHeader);

  for (int i = 0; i < kNCHIADC; i++) {
    if (conf->PID(i) == 0) { fHeader->SetZero(i); }
  }

  if (verbose > 1) { fHeader->Print(); }

  unsigned int itmp;
  for (int k = 0; k < kNCHIADC; k++) {
    fADC[k] = fData[5 * k + 32] & 0xFF;
    itmp = fData[5 * k + 33] & 0xFF;
    itmp = itmp << 8;
    fADC[k] = fADC[k] + itmp;

    fTime[k] = fData[5 * k + 34] & 0xFF;
    itmp = fData[5 * k + 35] & 0xFF;
    itmp = itmp << 8;
    fTime[k] = fTime[k] + itmp;
    itmp = fData[5 * k + 36] & 0xFF;
    itmp = itmp << 16;
    fTime[k] = fTime[k] + itmp;
  }
}

void SADCRawEvent::UnpackHeader_IADC(ADCHeader * header)
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
    int k = i % 4;
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
