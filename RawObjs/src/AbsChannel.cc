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
