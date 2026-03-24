#include "TH1D.h"

#include "RawObjs/FChannel.hh"

ClassImp(FChannel)

FChannel::FChannel()
  : AbsChannel(),
    ArrayS()
{
}

FChannel::FChannel(unsigned short id)
  : AbsChannel(id),
    ArrayS()
{
}

FChannel::FChannel(unsigned short id, int ndp)
  : AbsChannel(id),
    ArrayS(ndp)
{
}

FChannel::FChannel(unsigned short id, int ndp, const unsigned short * wave)
  : AbsChannel(id),
    ArrayS(ndp, wave)
{
}

FChannel::FChannel(const FChannel & ch)
  : AbsChannel(ch),
    ArrayS(ch)
{
  fWaveHis = nullptr;
  fPedestal = ch.GetPedestal();
}

FChannel::~FChannel()
{
  if (fWaveHis != nullptr) {
    delete fWaveHis;
    fWaveHis = nullptr;
  }
}

void FChannel::SetNdp(int ndp) { Set(ndp); }

void FChannel::SetWaveform(const unsigned short * wave)
{
  const int n = GetSize();
  if (n == 0) {
    Error("SetWaveform", "number of data points (ndp) is not set");
    return;
  }
  Set(n, wave);
}

void FChannel::SetWaveform(int ndp, const unsigned short * wave) { Set(ndp, wave); }

void FChannel::SetWaveform(int n, unsigned short count)
{
  if (GetSize() == 0) {
    Error("SetWaveform", "number of data points (ndp) is not set");
    return;
  }
  AddAt(count, n);
}

int FChannel::GetNdp() const { return GetSize(); }

const unsigned short * FChannel::GetWaveform() const { return GetArray(); }

TH1D * FChannel::GetWaveformHist(double pedm)
{
  const int ndp = GetSize();

  if (fWaveHis == nullptr) {
    fWaveHis = new TH1D(Form("Waveform_ch%03d", fID), "", ndp, 0.0, static_cast<double>(ndp));
  }

  if (pedm == 0.0) { pedm = static_cast<double>(fPedestal); }

  fWaveHis->Reset();
  for (int i = 1; i <= ndp; ++i) {
    fWaveHis->SetBinContent(i, static_cast<double>(At(i - 1)) - pedm);
  }

  return fWaveHis;
}
