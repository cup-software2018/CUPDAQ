/*
 *
 *  Module:  AbsChannel/RawObjs
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: A Class for storing basic channel information
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2022/12/19 00:43:47 $
 *  CVS/RCS Revision: $Revision: 1.2 $
 *  Status:           $State: Exp $
 *
 */

#include "RawObjs/AbsChannel.hh"

ClassImp(AbsChannel)

AbsChannel::AbsChannel()
    : TObject()
{
  fID = 0;
  fBit = 0;
}

AbsChannel::AbsChannel(unsigned short id)
    : TObject()
{
  fID = id;
  fBit = 0;
}

AbsChannel::AbsChannel(const AbsChannel & ch)
    : TObject(ch)
{
  fID = ch.GetID();
  fBit = ch.GetBit();
}

AbsChannel::~AbsChannel() {}

/**
$Log: AbsChannel.cc,v $
Revision 1.2  2022/12/19 00:43:47  cupsoft
add and clean up

Revision 1.1.1.1  2016/07/14 07:58:51  cupsoft
RawObjs

Revision 1.1.1.1  2016/02/29 08:25:13  cupsoft
RawObjs

**/
