/*
 *
 *  Module:  FChannel/RawObjs
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: A Class for storing FADC digitzed waveform
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/03/30 23:35:19 $
 *  CVS/RCS Revision: $Revision: 1.2 $
 *  Status:           $State: Exp $
 *
 */

#include <iostream>

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
  if (fWaveHis != nullptr) delete fWaveHis;
}

void FChannel::SetNdp(int ndp) { Set(ndp); }

void FChannel::SetWaveform(const unsigned short * wave)
{
  int n = GetSize();
  if (n == 0) {
    Error("SetWaveform", "Number of data point (ndp) is not set");
    return;
  }
  Set(n, wave);
}

void FChannel::SetWaveform(int ndp, const unsigned short * wave)
{
  Set(ndp, wave);
}

void FChannel::SetWaveform(int n, unsigned short count)
{
  if (GetSize() == 0) {
    Error("SetWaveform", "Number of data point (ndp) is not set");
    return;
  }
  AddAt(count, n);
}

int FChannel::GetNdp() const { return GetSize(); }
const unsigned short * FChannel::GetWaveform() const { return GetArray(); }

TH1D * FChannel::GetWaveformHist(Double_t pedm)
{
  int ndp = GetSize();

  if (!fWaveHis) {
    fWaveHis = new TH1D(Form("Waveform_ch%03d", fID), "", ndp, 0, ndp);
  }

  if (pedm == 0) { pedm = fPedestal; }

  fWaveHis->Reset();
  for (int i = 1; i <= ndp; i++) {
    fWaveHis->SetBinContent(i, At(i - 1) - pedm);
  }

  return fWaveHis;
}

