#include "DAQConfig/AbsConf.hh"

ClassImp(AbsConf)

AbsConf::AbsConf()
    : TNamed(),
      fIsEnabled(false),
      fIsLinked(false),
      fSID(0),
      fDAQID(0)
{
  fMID = 0;
}

AbsConf::AbsConf(int sid, ADC::TYPE type)
    : TNamed(),
      fIsEnabled(false),
      fIsLinked(false),
      fSID(sid),
      fDAQID(0)
{
  int tag = 0;
  switch (type) {
    case ADC::FADCT: tag = 0; break;
    case ADC::SADCT: tag = 64; break;
    case ADC::IADCT: tag = 192; break;
    default: break;
  }

  fMID = fSID + tag;
  fADCType = type;
}

AbsConf::~AbsConf() {}

int AbsConf::Compare(const TObject * object) const
{
  // sort using MID, not SID
  auto * comp = (AbsConf *)object;
  if (this->MID() > comp->MID()) return 1;
  else if (this->MID() < comp->MID()) return -1;
  return 0;
}

const char * AbsConf::InfoStr() const
{
  return Form("%5s [sid=%d mid=%d]", fName.Data(), fSID, fMID);
}