/*
 *
 *  Module:  FChannelData/RawObjs
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: TClonesArray for FChannels
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/03/30 23:35:19 $
 *  CVS/RCS Revision: $Revision: 1.3 $
 *  Status:           $State: Exp $
 *
 */

#include "RawObjs/FChannelData.hh"
#include "RawObjs/FChannel.hh"

ClassImp(FChannelData)

FChannelData::FChannelData()
    : TClonesArray("FChannel")
{
  fN = 0;
}

FChannelData::FChannelData(const FChannelData & data)
    : TClonesArray(data)
{
}

FChannelData::~FChannelData() {}

FChannel * FChannelData::Add() { return new ((*this)[fN++]) FChannel(); }

FChannel * FChannelData::Add(unsigned short id, int ndp)
{
  return new ((*this)[fN++]) FChannel(id, ndp);
}

FChannel * FChannelData::Add(unsigned short id, int ndp, const unsigned short * wave)
{
  return new ((*this)[fN++]) FChannel(id, ndp, wave);
}

FChannel * FChannelData::Add(FChannel * ch)
{
  return new ((*this)[fN++]) FChannel(*ch);
}

FChannel * FChannelData::GetChannel(unsigned short id) const
{
  FChannel * channel = nullptr;

  int nch = GetN();
  for (int i = 0; i < nch; i++) {
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
  int nch = data->GetN();
  for (int i = 0; i < nch; i++) {
    FChannel * ch = data->Get(i);
    this->Add(ch);
  }
}

void FChannelData::Clear(const Option_t *)
{
  fN = 0;
  Delete();
}