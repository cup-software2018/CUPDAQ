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

int AbsADCRaw::GetSize() const
{
  // Analytical estimate of ROOT-serialized size. Avoids expensive
  // TMessage::WriteObject() on every call.
  //
  // ROOT framing per class level: version(2) + bytecount(4) = 6 bytes.
  // Levels: TObject + ADCHeader + AbsADCRaw = 3 * 6 = 18 bytes framing.
  // TObject data: uniqueid(4) + bits(4) = 8 bytes.
  //
  // ADCHeader serialized fields (kNMAXADCCH = 40):
  //   kNMAXADCCH(4) + error(1) + zero[40](40) + tbit[40](40)
  //   + mid(4) + cid(4) + ttype(4) + dlen(4)
  //   + ped[40](160) + tnum(4) + loctnum(4) + loctptn(4)
  //   + ttime(8) + locttime(8) = 289 bytes
  //
  // AbsADCRaw fields: fType(4) + fMode(4) = 8 bytes.

  constexpr int kROOTFraming   = 18;  // 3 class levels × 6 bytes
  constexpr int kTObjectData   = 8;   // uniqueid + bits
  constexpr int kADCHeaderData = 289; // all non-transient fields
  constexpr int kAbsADCRawData = 8;   // fType + fMode

  return kROOTFraming + kTObjectData + kADCHeaderData + kAbsADCRawData;
}
