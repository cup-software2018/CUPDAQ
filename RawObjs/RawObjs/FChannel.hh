/*
 *
 *  Module:  FChannel/RawObjs
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: A Class for storing digitzed waveform
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/03/30 23:35:19 $
 *  CVS/RCS Revision: $Revision: 1.2 $
 *  Status:           $State: Exp $
 *
 */

#ifndef FChannel_hh
#define FChannel_hh

#include <vector>

#include "RawObjs/AbsChannel.hh"
#include "RawObjs/ArrayS.hh"

class TH1D;
class FChannel : public AbsChannel, public ArrayS {
public:
  FChannel();
  FChannel(unsigned short id);
  FChannel(unsigned short id, int ndp);
  FChannel(unsigned short id, int ndp, const unsigned short * wave);
  FChannel(const FChannel & ch);
  virtual ~FChannel();

  void SetNdp(int ndp);
  void SetWaveform(const unsigned short * wave);
  void SetWaveform(int ndp, const unsigned short * wave);
  void SetWaveform(int n, unsigned short adc);
  void SetPedestal(unsigned short ped);

  int GetNdp() const;
  unsigned short GetPedestal() const;
  const unsigned short * GetWaveform() const;

  TH1D * GetWaveformHist(double pedm = 0);

protected:
  unsigned short fPedestal = 0; // pedestal from FADC
  TH1D * fWaveHis = nullptr;          //! just for drawing waveform

  ClassDef(FChannel, 1)
};

//
// Inline functions
//
inline void FChannel::SetPedestal(unsigned short ped) { fPedestal = ped; }
inline unsigned short FChannel::GetPedestal() const { return fPedestal; }

#endif
