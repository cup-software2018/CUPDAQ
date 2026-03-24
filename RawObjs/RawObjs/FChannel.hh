#pragma once

#include "RawObjs/AbsChannel.hh"
#include "RawObjs/ArrayS.hh"

class TH1D;

class FChannel : public AbsChannel, public ArrayS {
public:
  FChannel();
  explicit FChannel(unsigned short id);
  FChannel(unsigned short id, int ndp);
  FChannel(unsigned short id, int ndp, const unsigned short * wave);
  FChannel(const FChannel & ch);
  ~FChannel() override;

  void SetNdp(int ndp);
  void SetWaveform(const unsigned short * wave);
  void SetWaveform(int ndp, const unsigned short * wave);
  void SetWaveform(int n, unsigned short adc);
  void SetPedestal(unsigned short ped);

  int GetNdp() const;
  unsigned short GetPedestal() const;
  const unsigned short * GetWaveform() const;

  TH1D * GetWaveformHist(double pedm = 0.0);

protected:
  unsigned short fPedestal = 0; // pedestal from FADC
  TH1D * fWaveHis = nullptr;    //! just for drawing waveform

  ClassDef(FChannel, 1)
};

// Inline functions

inline void FChannel::SetPedestal(unsigned short ped) { fPedestal = ped; }
inline unsigned short FChannel::GetPedestal() const { return fPedestal; }
