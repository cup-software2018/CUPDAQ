#include "OnlObjs/ADCHeader.hh"
#include "TString.h"
#include <iostream>

using namespace std;

ClassImp(ADCHeader)

ADCHeader::ADCHeader()
    : TObject(),
      kNMAXADCCH(40)
{
  mid = 0;
  cid = 0;
  error = false;

  zero = new bool[kNMAXADCCH];
  tbit = new bool[kNMAXADCCH];
  ped = new unsigned int[kNMAXADCCH];

  memset(zero, false, kNMAXADCCH * sizeof(bool));
  memset(tbit, false, kNMAXADCCH * sizeof(bool));
  memset(ped, 0, kNMAXADCCH * sizeof(unsigned int));

  ttype = 0;
  dlen = 0;
  tnum = 0;
  ttime = 0;
  loctptn = 0;
  loctnum = 0;
  locttime = 0;

  fCalConsts = nullptr;
}

ADCHeader::ADCHeader(const ADCHeader & header)
    : TObject(header),
      kNMAXADCCH(40)
{
  mid = header.GetMID();
  cid = header.GetCID();
  error = header.GetError();

  zero = new bool[kNMAXADCCH];
  tbit = new bool[kNMAXADCCH];
  ped = new unsigned int[kNMAXADCCH];

  memcpy(zero, header.GetZero(), kNMAXADCCH * sizeof(bool));
  memcpy(tbit, header.GetTriggerBit(), kNMAXADCCH * sizeof(bool));
  memcpy(ped, header.GetPedestal(), kNMAXADCCH * sizeof(unsigned int));

  ttype = header.GetTriggerType();
  dlen = header.GetDataLength();
  tnum = header.GetTriggerNumber();
  ttime = header.GetTriggerTime();
  loctptn = header.GetLocalTriggerPattern();
  loctnum = header.GetLocalTriggerNumber();
  locttime = header.GetLocalTriggerTime();

  fCalConsts = header.GetTimeCalConsts();
}

ADCHeader::~ADCHeader()
{
  delete[] zero;
  delete[] tbit;
  delete[] ped;
}

void ADCHeader::Print(Option_t * opt) const
{
  cout << Form("===> mid=%d cid=%d", mid, cid) << endl;
  cout << Form(" dlength=%u, ttype=%u", dlen, ttype) << endl;
  cout << Form(" tnum =%u, ttime =%lu", tnum, ttime) << endl;
  cout << Form(" ctnum=%u, cttime=%lu", loctnum, locttime) << endl;
}