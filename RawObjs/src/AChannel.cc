#include "RawObjs/AChannel.hh"

ClassImp(AChannel)

AChannel::AChannel()
  : AbsChannel()
{
}

AChannel::AChannel(unsigned short id)
  : AbsChannel(id)
{
}

AChannel::AChannel(const AChannel & ch)
  : AbsChannel(ch)
{
  fADC = ch.GetADC();
  fTime = ch.GetTime();
}
