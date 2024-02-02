#include <iostream>

#include "OnlObjs/FADCRawChannel.hh"

using namespace std;

ClassImp(FADCRawChannel)

FADCRawChannel::FADCRawChannel()
    : TObject()
{
  fNDP = 0;
  fADC = nullptr;
}

FADCRawChannel::FADCRawChannel(int ndp)
    : TObject()
{
  fNDP = ndp;
  fADC = new unsigned short[fNDP];
  memset(fADC, 0, fNDP * sizeof(unsigned short));
}

FADCRawChannel::FADCRawChannel(const FADCRawChannel & ch)
    : TObject(ch)
{
  fNDP = ch.GetNDP();
  fADC = new unsigned short[fNDP];
  unsigned short * adc = ch.GetADC();
  memcpy(fADC, adc, fNDP * sizeof(unsigned short));
}

FADCRawChannel::~FADCRawChannel()
{
  if (fADC) delete[] fADC;
}

void FADCRawChannel::Print(Option_t * opt) const
{
  for (int i = 0; i < fNDP; i++)
    cout << fADC[i] << " ";
  cout << endl;
}