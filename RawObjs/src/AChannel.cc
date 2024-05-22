/*
 *
 *  Module:  AChannel/RawObjs
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: A Class for storing ADC information
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2022/12/19 00:43:47 $
 *  CVS/RCS Revision: $Revision: 1.2 $
 *  Status:           $State: Exp $
 *
 */

#include "RawObjs/AChannel.hh"

ClassImp(AChannel)

AChannel::AChannel()
    : AbsChannel()
{
  fADC = 0;
  fTime = 0;
}

AChannel::AChannel(unsigned short id)
    : AbsChannel(id)
{
  fADC = 0;
  fTime = 0;
}

AChannel::AChannel(const AChannel & ch)
    : AbsChannel(ch)
{
  fADC = ch.GetADC();
  fTime = ch.GetTime();
}

AChannel::~AChannel() {}
