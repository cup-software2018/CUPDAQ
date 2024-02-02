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
    : TObject()
{
  fNCh = 0;
  fColl = new AChannelColl("AChannel");
}

AChannelData::AChannelData(const AChannelData & data)
    : TObject(data),
      fColl(data.GetColl())
{
}

AChannelData::~AChannelData() { delete fColl; }

AChannel * AChannelData::Add() { return new ((*fColl)[fNCh++]) AChannel(); }

AChannel * AChannelData::Add(unsigned short id)
{
  return new ((*fColl)[fNCh++]) AChannel(id);
}

AChannel * AChannelData::Add(AChannel * ch)
{
  return new ((*fColl)[fNCh++]) AChannel(*ch);
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
  fColl->Delete();
  fNCh = 0;
}

void AChannelData::Dump() const {}

/**
$Log: AChannelData.cc,v $
Revision 1.3  2023/03/30 23:35:19  cupsoft
*** empty log message ***

Revision 1.2  2022/12/19 00:43:47  cupsoft
add and clean up

Revision 1.1.1.1  2016/07/14 07:58:51  cupsoft
RawObjs

Revision 1.1.1.1  2016/02/29 08:25:13  cupsoft
RawObjs

**/
