#include "RawObjs/FChannel.hh"
#include "RawObjs/FChannelData.hh"

ClassImp(FChannelData)

FChannelData::FChannelData()
  : TClonesArray("FChannel")
{
}

FChannelData::FChannelData(const FChannelData & data)
  : TClonesArray(data)
{
}

FChannel * FChannelData::Add() { return new ((*this)[fN++]) FChannel(); }

FChannel * FChannelData::Add(unsigned short id, int ndp) { return new ((*this)[fN++]) FChannel(id, ndp); }

FChannel * FChannelData::Add(unsigned short id, int ndp, const unsigned short * wave)
{
  return new ((*this)[fN++]) FChannel(id, ndp, wave);
}

FChannel * FChannelData::Add(FChannel * ch) { return new ((*this)[fN++]) FChannel(*ch); }

FChannel * FChannelData::GetChannel(unsigned short id) const
{
  FChannel * channel = nullptr;

  const int nch = GetN();
  for (int i = 0; i < nch; ++i) {
    FChannel * rch = Get(i);
    if (rch->GetID() == id) {
      channel = rch;
      break;
    }
  }

  return channel;
}

void FChannelData::CopyFrom(const FChannelData * data)
{
  const int nch = data->GetN();
  for (int i = 0; i < nch; ++i) {
    FChannel * ch = data->Get(i);
    Add(ch);
  }
}

void FChannelData::Clear(const Option_t *)
{
  fN = 0;
  Delete();
}
