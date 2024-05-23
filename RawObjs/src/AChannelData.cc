/*
 *
 *  Module:  AChannelData/RawObjs
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: TClonesArray for AChannels
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/03/30 23:35:19 $
 *  CVS/RCS Revision: $Revision: 1.3 $
 *  Status:           $State: Exp $
 *
 */

#include "RawObjs/AChannelData.hh"
#include "RawObjs/AChannel.hh"

ClassImp(AChannelData)

AChannelData::AChannelData()
    : TClonesArray("AChannel")
{
}

AChannelData::AChannelData(const AChannelData & data)
    : TClonesArray(data)
{
}

AChannelData::~AChannelData() {}

AChannel * AChannelData::Add() { return new ((*this)[fN++]) AChannel(); }

AChannel * AChannelData::Add(unsigned short id)
{
  return new ((*this)[fN++]) AChannel(id);
}

AChannel * AChannelData::Add(AChannel * ch)
{
  return new ((*this)[fN++]) AChannel(*ch);
}

AChannel * AChannelData::GetChannel(unsigned short id) const
{
  AChannel * channel = nullptr;

  int nch = GetN();
  for (int i = 0; i < nch; i++) {
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
  int nch = data->GetN();
  for (int i = 0; i < nch; i++) {
    AChannel * ch = data->Get(i);
    this->Add(ch);
  }
}

void AChannelData::Clear(const Option_t *)
{
  fN = 0;
  Delete();
}