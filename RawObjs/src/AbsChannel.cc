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
}

AbsChannel::AbsChannel(unsigned short id)
    : TObject()
{
  fID = id;
}

AbsChannel::AbsChannel(const AbsChannel & ch)
    : TObject(ch)
{
  fID = ch.GetID();
  fBit = ch.GetBit();
}

AbsChannel::~AbsChannel() {}
