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
 *  CVS/RCS Revision: $Revision: 1.2 $
 *  Status:           $State: Exp $
 *
 */

#include "RawObjs/FChannelData.hh"
#include "RawObjs/FChannel.hh"

ClassImp(FChannelData)

FChannelData::FChannelData()
    : TObject()
{
  fNCh = 0;
  fColl = new FChannelColl("FChannel");
}

FChannelData::FChannelData(const FChannelData & data)
    : TObject(),
      fColl(data.GetColl())
{
}

FChannelData::~FChannelData() { delete fColl; }

FChannel * FChannelData::Add() { return new ((*fColl)[fNCh++]) FChannel(); }

FChannel * FChannelData::Add(unsigned short id, int ndp)
{
  return new ((*fColl)[fNCh++]) FChannel(id, ndp);
}

FChannel * FChannelData::Add(unsigned short id, int ndp,
                             const unsigned short * wave)
{
  return new ((*fColl)[fNCh++]) FChannel(id, ndp, wave);
}

FChannel * FChannelData::Add(FChannel * ch)
{
  return new ((*fColl)[fNCh++]) FChannel(*ch);
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
  fColl->Delete();
  fNCh = 0;
}

void FChannelData::Dump() const {}

/**
$Log: FChannelData.cc,v $
Revision 1.2  2023/03/30 23:35:19  cupsoft
*** empty log message ***

Revision 1.1  2022/12/19 00:43:47  cupsoft
add and clean up

Revision 1.2  2020/01/14 02:26:08  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/07/14 07:58:51  cupsoft
RawObjs

Revision 1.2  2016/03/17 08:12:41  jslee
add inheritance from TArray class
remove member array for waveform

Revision 1.1.1.1  2016/02/29 08:25:13  cupsoft
RawObjs

**/
