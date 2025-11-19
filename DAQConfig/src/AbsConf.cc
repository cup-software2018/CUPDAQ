#include "DAQConfig/AbsConf.hh"

ClassImp(AbsConf)

AbsConf::AbsConf()
  : TNamed()
{
}

AbsConf::AbsConf(int sid, ADC::TYPE type)
  : TNamed(),
    fSID(sid),
    fADCType(type)
{
  int tag = 0;
  switch (type) {
    case ADC::FADCT: tag = 0; break;
    case ADC::SADCT: tag = 64; break;
    case ADC::IADCT: tag = 192; break;
    default: break;
  }
  fMID = fSID + tag;
}

int AbsConf::Compare(const TObject * object) const
{
  auto * comp = static_cast<const AbsConf *>(object);
  if (fMID > comp->MID()) return 1;
  if (fMID < comp->MID()) return -1;
  return 0;
}

const char * AbsConf::InfoStr() const { return Form("%5s [sid=%d mid=%d]", fName.Data(), fSID, fMID); }
