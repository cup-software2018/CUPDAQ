#include <iostream>

#include "OnlObjs/FADCRawChannel.hh"

ClassImp(FADCRawChannel)

FADCRawChannel::FADCRawChannel()
  : TObject(),
    fNDP(0),
    fADC(nullptr)
{
}

FADCRawChannel::FADCRawChannel(int ndp)
  : TObject(),
    fNDP(ndp),
    fADC(new unsigned short[fNDP])
{
  std::memset(fADC, 0, static_cast<std::size_t>(fNDP) * sizeof(unsigned short));
}

FADCRawChannel::FADCRawChannel(const FADCRawChannel & ch)
  : TObject(ch),
    fNDP(ch.GetNDP()),
    fADC(new unsigned short[fNDP])
{
  std::memcpy(fADC, ch.GetADC(), static_cast<std::size_t>(fNDP) * sizeof(unsigned short));
}

FADCRawChannel::~FADCRawChannel() { delete[] fADC; }

void FADCRawChannel::Print(Option_t * opt) const
{
  for (int i = 0; i < fNDP; ++i) {
    std::cout << fADC[i] << " ";
  }
  std::cout << std::endl;
}
