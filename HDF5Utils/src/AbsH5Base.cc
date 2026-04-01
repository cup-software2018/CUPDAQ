#include "HDF5Utils/AbsH5Base.hh"

ClassImp(AbsH5Base)

void AbsH5Base::InitSubRun()
{
  if (fSubType < 0) {
    fSubType = SubRun_t::BuildType();
  }
  fSubRun.nevent = 0;
  fSubRun.first = 0;
  fSubRun.last = 0;
}

void AbsH5Base::UpdateSubRun(std::uint32_t tnum)
{
  if (fSubRun.nevent == 0) { 
    fSubRun.first = tnum; 
  }
  fSubRun.last = tnum;
  fSubRun.nevent += 1;
}

void AbsH5Base::CloseSubRun()
{
  if (fSubType >= 0) {
    H5Tclose(fSubType);
    fSubType = H5I_INVALID_HID;
  }
}