#include <cstring>

#include "DAQConfig/IADCTConf.hh"
#include "DAQConfig/SADCTConf.hh"
#include "OnlObjs/SADCRawEvent.hh"

ClassImp(SADCRawEvent)

SADCRawEvent::SADCRawEvent()
  : AbsADCRaw(),
    fNCH(0),
    fADC(nullptr),
    fTime(nullptr)
{
  fMode = ADC::SMODE;
}

SADCRawEvent::SADCRawEvent(int s, ADC::TYPE type)
  : AbsADCRaw(s, type),
    fNCH(0),
    fADC(nullptr),
    fTime(nullptr)
{
  switch (type) {
    case ADC::SADC: fNCH = kNCHSADC; break;
    case ADC::IADC: fNCH = kNCHIADC; break;
    default: break;
  }

  if (fNCH > 0) {
    fADC = new unsigned int[fNCH];
    std::memset(fADC, 0, static_cast<size_t>(fNCH) * sizeof(unsigned int));
    fTime = new unsigned int[fNCH];
    std::memset(fTime, 0, static_cast<size_t>(fNCH) * sizeof(unsigned int));
  }

  fMode = ADC::SMODE;
}

SADCRawEvent::SADCRawEvent(const SADCRawEvent & raw)
  : AbsADCRaw(raw),
    fNCH(raw.GetNCH()),
    fADC(nullptr),
    fTime(nullptr)
{
  if (fNCH > 0) {
    fADC = new unsigned int[fNCH];
    fTime = new unsigned int[fNCH];
    std::memcpy(fADC, raw.GetADCs(), static_cast<size_t>(fNCH) * sizeof(unsigned int));
    std::memcpy(fTime, raw.GetTimes(), static_cast<size_t>(fNCH) * sizeof(unsigned int));
  }

  fMode = ADC::SMODE;
}

SADCRawEvent::~SADCRawEvent()
{
  delete[] fADC;
  delete[] fTime;
}

int SADCRawEvent::GetSize() const
{
  // AbsADCRaw base serialized size (TObject + ADCHeader + AbsADCRaw fields).
  int size = AbsADCRaw::GetSize();

  // SADCRawEvent class framing: version(2) + bytecount(4) = 6 bytes.
  // Own fields: fNCH(4) + fADC[fNCH](4*fNCH) + fTime[fNCH](4*fNCH).
  size += 6 + 4 + fNCH * 4 + fNCH * 4;

  return size;
}

void SADCRawEvent::Unpack(AbsConf * config, int verbose)
{
  switch (fType) {
    case ADC::SADC: Unpack_SADC(config, verbose); break;
    case ADC::IADC: Unpack_IADC(config, verbose); break;
    default: break;
  }
}

// ----------------------------------------------------------------------
//  SADC
// ----------------------------------------------------------------------

void SADCRawEvent::Unpack_SADC(AbsConf * config, int verbose)
{
  auto * conf = static_cast<SADCTConf *>(config);

  UnpackHeader_SADC(fHeader);

  if (verbose > 1) { fHeader->Print(); }

  if (fHeader->GetDataLength() != kBYTESPEREVENTSADC) {
    fHeader->SetError();
    return;
  }

  unsigned int itmp = 0;
  unsigned int peak = 0;
  unsigned int peaktime = 0;

  for (int i = 0; i < kNCHSADC; ++i) {
    if (conf->PID(i) == 0) {
      fHeader->SetZero(i);
      continue;
    }

    peak = static_cast<unsigned int>(fData[32 + i * 7] & 0xFF);
    itmp = static_cast<unsigned int>(fData[32 + i * 7 + 1] & 0xFF);
    peak += (itmp << 8);
    itmp = static_cast<unsigned int>(fData[32 + i * 7 + 2] & 0xFF);
    peak += (itmp << 16);
    fADC[i] = peak;

    peaktime = static_cast<unsigned int>(fData[32 + i * 7 + 3] & 0xFF);
    peaktime *= 8U;
    itmp = static_cast<unsigned int>(fData[32 + i * 7 + 4] & 0xFF);
    peaktime += itmp * 1000U;
    itmp = static_cast<unsigned int>(fData[32 + i * 7 + 5] & 0xFF) << 8;
    peaktime += itmp * 1000U;
    itmp = static_cast<unsigned int>(fData[32 + i * 7 + 6] & 0xFF) << 16;
    peaktime += itmp * 1000U;
    fTime[i] = peaktime;
  }
}

void SADCRawEvent::UnpackHeader_SADC(ADCHeader * header)
{
  unsigned int itmp = 0;
  unsigned long ltmp = 0;
  unsigned long finetime = 0;
  unsigned long coarsetime = 0;

  unsigned int dlen = static_cast<unsigned int>(fData[0] & 0xFF);
  itmp = static_cast<unsigned int>(fData[1] & 0xFF);
  dlen += (itmp << 8);
  itmp = static_cast<unsigned int>(fData[2] & 0xFF);
  dlen += (itmp << 16);
  itmp = static_cast<unsigned int>(fData[3] & 0xFF);
  dlen += (itmp << 24);
  header->SetDataLength(dlen);

  unsigned int ttype = static_cast<unsigned int>(fData[6] & 0x0F);
  header->SetTriggerType(ttype);

  unsigned int tnum = static_cast<unsigned int>(fData[7] & 0xFF);
  itmp = static_cast<unsigned int>(fData[8] & 0xFF);
  tnum += (itmp << 8);
  itmp = static_cast<unsigned int>(fData[9] & 0xFF);
  tnum += (itmp << 16);
  itmp = static_cast<unsigned int>(fData[10] & 0xFF);
  tnum += (itmp << 24);
  tnum += 1U;
  header->SetTriggerNumber(tnum);

  finetime = static_cast<unsigned long>(fData[11] & 0xFF);
  finetime *= 8UL;

  ltmp = static_cast<unsigned long>(fData[12] & 0xFF);
  coarsetime = ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[13] & 0xFF) << 8;
  coarsetime += ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[14] & 0xFF) << 16;
  coarsetime += ltmp * 1000UL;

  header->SetTriggerTime(coarsetime + finetime);

  int mid = static_cast<int>(fData[15] & 0xFF);
  header->SetMID(mid);

  unsigned int ctnum = static_cast<unsigned int>(fData[17] & 0xFF);
  itmp = static_cast<unsigned int>(fData[18] & 0xFF);
  ctnum += (itmp << 8);
  itmp = static_cast<unsigned int>(fData[19] & 0xFF);
  ctnum += (itmp << 16);
  itmp = static_cast<unsigned int>(fData[20] & 0xFF);
  ctnum += (itmp << 24);
  ctnum += 1U;
  header->SetLocalTriggerNumber(ctnum);

  unsigned int ctptn = static_cast<unsigned int>(fData[21] & 0xFF);
  itmp = static_cast<unsigned int>(fData[22] & 0xFF);
  ctptn += (itmp << 8);
  itmp = static_cast<unsigned int>(fData[23] & 0xFF);
  ctptn += (itmp << 16);
  itmp = static_cast<unsigned int>(fData[24] & 0xFF);
  ctptn += (itmp << 24);
  header->SetLocalTriggerPattern(ctptn);

  finetime = static_cast<unsigned long>(fData[25] & 0xFF);
  finetime *= 8UL;

  ltmp = static_cast<unsigned long>(fData[26] & 0xFF);
  coarsetime = ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[27] & 0xFF) << 8;
  coarsetime += ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[28] & 0xFF) << 16;
  coarsetime += ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[29] & 0xFF) << 24;
  coarsetime += ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[30] & 0xFF) << 32;
  coarsetime += ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[31] & 0xFF) << 40;
  coarsetime += ltmp * 1000UL;

  header->SetLocalTriggerTime(coarsetime + finetime);

  for (int i = 0; i < kNCHSADC; ++i) {
    unsigned int tbit = ctptn & (1U << i);
    if (tbit > 0U) { header->SetTriggerBit(i); }
  }
}

// ----------------------------------------------------------------------
//  IADC (scalar mode)
// ----------------------------------------------------------------------

void SADCRawEvent::Unpack_IADC(AbsConf * config, int verbose)
{
  auto * conf = static_cast<IADCTConf *>(config);

  UnpackHeader_IADC(fHeader);

  for (int i = 0; i < kNCHIADC; ++i) {
    if (conf->PID(i) == 0) { fHeader->SetZero(i); }
  }

  if (verbose > 1) { fHeader->Print(); }

  unsigned int itmp = 0;
  for (int k = 0; k < kNCHIADC; ++k) {
    fADC[k] = static_cast<unsigned int>(fData[5 * k + 32] & 0xFF);
    itmp = static_cast<unsigned int>(fData[5 * k + 33] & 0xFF);
    itmp <<= 8;
    fADC[k] += itmp;

    fTime[k] = static_cast<unsigned int>(fData[5 * k + 34] & 0xFF);
    itmp = static_cast<unsigned int>(fData[5 * k + 35] & 0xFF);
    itmp <<= 8;
    fTime[k] += itmp;
    itmp = static_cast<unsigned int>(fData[5 * k + 36] & 0xFF);
    itmp <<= 16;
    fTime[k] += itmp;
  }
}

void SADCRawEvent::UnpackHeader_IADC(ADCHeader * header)
{
  unsigned int itmp = 0;
  unsigned long ltmp = 0;
  unsigned long finetime = 0;
  unsigned long coarsetime = 0;

  unsigned int dlen = static_cast<unsigned int>(fData[0] & 0xFF);
  itmp = static_cast<unsigned int>(fData[1] & 0xFF);
  dlen += (itmp << 8);
  itmp = static_cast<unsigned int>(fData[2] & 0xFF);
  dlen += (itmp << 16);
  itmp = static_cast<unsigned int>(fData[3] & 0xFF);
  dlen += (itmp << 24);
  header->SetDataLength(dlen);

  unsigned int ttype = static_cast<unsigned int>(fData[6] & 0x0F);
  header->SetTriggerType(ttype);

  unsigned int tnum = static_cast<unsigned int>(fData[7] & 0xFF);
  itmp = static_cast<unsigned int>(fData[8] & 0xFF);
  tnum += (itmp << 8);
  itmp = static_cast<unsigned int>(fData[9] & 0xFF);
  tnum += (itmp << 16);
  itmp = static_cast<unsigned int>(fData[10] & 0xFF);
  tnum += (itmp << 24);
  tnum += 1U;
  header->SetTriggerNumber(tnum);

  finetime = static_cast<unsigned long>(fData[11] & 0xFF);
  finetime *= 8UL;

  ltmp = static_cast<unsigned long>(fData[12] & 0xFF);
  coarsetime = ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[13] & 0xFF) << 8;
  coarsetime += ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[14] & 0xFF) << 16;
  coarsetime += ltmp * 1000UL;
  header->SetTriggerTime(coarsetime + finetime);

  int mid = static_cast<int>(fData[15] & 0xFF);
  header->SetMID(mid);

  unsigned int ctnum = static_cast<unsigned int>(fData[17] & 0xFF);
  itmp = static_cast<unsigned int>(fData[18] & 0xFF);
  ctnum += (itmp << 8);
  itmp = static_cast<unsigned int>(fData[19] & 0xFF);
  ctnum += (itmp << 16);
  itmp = static_cast<unsigned int>(fData[20] & 0xFF);
  ctnum += (itmp << 24);
  ctnum += 1U;
  header->SetLocalTriggerNumber(ctnum);

  unsigned int ctptn = static_cast<unsigned int>(fData[21] & 0xFF);
  itmp = static_cast<unsigned int>(fData[22] & 0xFF);
  ctptn += (itmp << 8);
  itmp = static_cast<unsigned int>(fData[23] & 0xFF);
  ctptn += (itmp << 16);
  itmp = static_cast<unsigned int>(fData[24] & 0xFF);
  ctptn += (itmp << 24);
  header->SetLocalTriggerPattern(ctptn);

  // same pattern logic as FADCRawEvent::UnpackHeader_IADC (4 ch per bit)
  for (int i = 0; i < kNCHIADC; ++i) {
    int k = i / 4;
    unsigned int tbit = static_cast<unsigned int>(TESTBIT(ctptn, k));
    if (tbit > 0U) { header->SetTriggerBit(i); }
  }

  finetime = static_cast<unsigned long>(fData[25] & 0xFF);
  finetime *= 8UL;

  ltmp = static_cast<unsigned long>(fData[26] & 0xFF);
  coarsetime = ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[27] & 0xFF) << 8;
  coarsetime += ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[28] & 0xFF) << 16;
  coarsetime += ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[29] & 0xFF) << 24;
  coarsetime += ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[30] & 0xFF) << 32;
  coarsetime += ltmp * 1000UL;
  ltmp = static_cast<unsigned long>(fData[31] & 0xFF) << 40;
  coarsetime += ltmp * 1000UL;
  header->SetLocalTriggerTime(coarsetime + finetime);

  for (int k = 0; k < 20; ++k) {
    unsigned int ped0 = static_cast<unsigned int>(fData[64 + k * 3 + 0] & 0xFF);
    itmp = static_cast<unsigned int>(fData[64 + k * 3 + 1] & 0x0F);
    itmp <<= 8;
    ped0 += itmp;
    itmp = static_cast<unsigned int>(fData[64 + k * 3 + 1] & 0xF0);
    header->SetPedestal(2 * k, static_cast<int>(ped0));

    unsigned int ped1 = itmp >> 4;
    itmp = static_cast<unsigned int>(fData[64 + k * 3 + 2] & 0xFF);
    itmp <<= 4;
    ped1 += itmp;
    header->SetPedestal(2 * k + 1, static_cast<int>(ped1));
  }
}
