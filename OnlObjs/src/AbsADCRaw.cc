#include "OnlObjs/AbsADCRaw.hh"

ClassImp(AbsADCRaw)

AbsADCRaw::AbsADCRaw()
    : TObject()
{
  fSize = 0;
  fData = nullptr;
  fHeader = nullptr;
}

AbsADCRaw::AbsADCRaw(int s, ADC::TYPE t)
    : TObject()
{
  fType = t;
  fSize = s;
  fData = new unsigned char[fSize];
  memset(fData, 0, fSize * sizeof(unsigned char));
  fHeader = new ADCHeader();
}

AbsADCRaw::AbsADCRaw(const AbsADCRaw & raw)
    : TObject(raw)
{
  fType = raw.GetADCType();
  fMode = raw.GetADCMode();
  fHeader = new ADCHeader(*(raw.GetADCHeader()));
  fSize = raw.GetRawDataSize();
  if (raw.GetRawData()) {
    fData = new unsigned char[fSize];
    memcpy(fData, raw.GetRawData(), fSize * sizeof(unsigned char));
  }
}

AbsADCRaw::~AbsADCRaw()
{
  if (fData) delete[] fData;
  if (fHeader) delete fHeader;
}

void AbsADCRaw::CopyDataFrom(unsigned char * raw)
{
  memcpy(fData, raw, fSize * sizeof(unsigned char));
}

void AbsADCRaw::PrintHeader() const { fHeader->Print(); }
