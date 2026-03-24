#include <cstring>

#include "OnlObjs/AbsADCRaw.hh"

ClassImp(AbsADCRaw)

AbsADCRaw::AbsADCRaw()
  : TObject(),
    fType(),
    fMode(),
    fHeader(nullptr),
    fSize(0),
    fData(nullptr)
{
}

AbsADCRaw::AbsADCRaw(int s, ADC::TYPE t)
  : TObject(),
    fType(t),
    fMode(),
    fHeader(new ADCHeader()),
    fSize(s),
    fData(new unsigned char[fSize])
{
  std::memset(fData, 0, static_cast<std::size_t>(fSize));
}

AbsADCRaw::AbsADCRaw(const AbsADCRaw & raw)
  : TObject(raw),
    fType(raw.GetADCType()),
    fMode(raw.GetADCMode()),
    fHeader(nullptr),
    fSize(raw.GetRawDataSize()),
    fData(nullptr)
{
  if (raw.GetADCHeader()) { fHeader = new ADCHeader(*raw.GetADCHeader()); }

  if (raw.GetRawData() && fSize > 0) {
    fData = new unsigned char[fSize];
    std::memcpy(fData, raw.GetRawData(), static_cast<std::size_t>(fSize));
  }
}

AbsADCRaw::~AbsADCRaw()
{
  delete[] fData;
  delete fHeader;
}

void AbsADCRaw::CopyDataFrom(unsigned char * raw) { std::memcpy(fData, raw, static_cast<std::size_t>(fSize)); }

void AbsADCRaw::PrintHeader() const
{
  if (fHeader) { fHeader->Print(); }
}
