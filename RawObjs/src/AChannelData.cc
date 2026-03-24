#include "RawObjs/AChannel.hh"
#include "RawObjs/AChannelData.hh"

ClassImp(AChannelData)

AChannelData::AChannelData()
  : TClonesArray("AChannel")
{
}

AChannelData::AChannelData(const AChannelData & data)
  : TClonesArray(data)
{
}

AChannel * AChannelData::Add() { return new ((*this)[fN++]) AChannel(); }

AChannel * AChannelData::Add(unsigned short id) { return new ((*this)[fN++]) AChannel(id); }

AChannel * AChannelData::Add(AChannel * ch) { return new ((*this)[fN++]) AChannel(*ch); }

AChannel * AChannelData::GetChannel(unsigned short id) const
{
  AChannel * channel = nullptr;

  const int nch = GetN();
  for (int i = 0; i < nch; ++i) {
    AChannel * rch = Get(i);
    if (rch->GetID() == id) {
      channel = rch;
      break;
    }
  }

  return channel;
}

void AChannelData::CopyFrom(const AChannelData * data)
{
  const int nch = data->GetN();
  for (int i = 0; i < nch; ++i) {
    AChannel * ch = data->Get(i);
    Add(ch);
  }
}

void AChannelData::Clear(const Option_t *)
{
  fN = 0;
  Delete();
}
