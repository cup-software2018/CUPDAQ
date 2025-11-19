#include <cstring>
#include <iostream>

#include "TString.h"

#include "OnlObjs/ADCHeader.hh"

ClassImp(ADCHeader)

ADCHeader::ADCHeader()
  : TObject(),
    kNMAXADCCH(40),
    error(false),
    zero(new bool[kNMAXADCCH]),
    tbit(new bool[kNMAXADCCH]),
    mid(0),
    cid(0),
    ttype(0),
    dlen(0),
    ped(new unsigned int[kNMAXADCCH]),
    tnum(0),
    loctnum(0),
    loctptn(0),
    ttime(0),
    locttime(0),
    fCalConsts(nullptr)
{
  std::memset(zero, 0, static_cast<std::size_t>(kNMAXADCCH) * sizeof(bool));
  std::memset(tbit, 0, static_cast<std::size_t>(kNMAXADCCH) * sizeof(bool));
  std::memset(ped, 0, static_cast<std::size_t>(kNMAXADCCH) * sizeof(unsigned int));
}

ADCHeader::ADCHeader(const ADCHeader & header)
  : TObject(header),
    kNMAXADCCH(40),
    error(header.GetError()),
    zero(new bool[kNMAXADCCH]),
    tbit(new bool[kNMAXADCCH]),
    mid(header.GetMID()),
    cid(header.GetCID()),
    ttype(header.GetTriggerType()),
    dlen(header.GetDataLength()),
    ped(new unsigned int[kNMAXADCCH]),
    tnum(header.GetTriggerNumber()),
    loctnum(header.GetLocalTriggerNumber()),
    loctptn(header.GetLocalTriggerPattern()),
    ttime(header.GetTriggerTime()),
    locttime(header.GetLocalTriggerTime()),
    fCalConsts(header.GetTimeCalConsts())
{
  std::memcpy(zero, header.GetZero(), static_cast<std::size_t>(kNMAXADCCH) * sizeof(bool));
  std::memcpy(tbit, header.GetTriggerBit(), static_cast<std::size_t>(kNMAXADCCH) * sizeof(bool));
  std::memcpy(ped, header.GetPedestal(), static_cast<std::size_t>(kNMAXADCCH) * sizeof(unsigned int));
}

ADCHeader::~ADCHeader()
{
  delete[] zero;
  delete[] tbit;
  delete[] ped;
}

void ADCHeader::Print(Option_t * opt) const
{
  std::cout << Form("===> mid=%d cid=%d", mid, cid) << std::endl;
  std::cout << Form(" dlength=%u, ttype=%u", dlen, ttype) << std::endl;
  std::cout << Form(" tnum =%u, ttime =%lu", tnum, ttime) << std::endl;
  std::cout << Form(" ctnum=%u, cttime=%lu", loctnum, locttime) << std::endl;
}
