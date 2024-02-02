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
  unsigned short fPedestal; // pedestal from FADC
  TH1D * fWaveHis;          //! just for drawing waveform

  ClassDef(FChannel, 1)
};

//
// Inline functions
//
inline void FChannel::SetPedestal(unsigned short ped) { fPedestal = ped; }
inline unsigned short FChannel::GetPedestal() const { return fPedestal; }

#endif

/**
$Log: FChannel.hh,v $
Revision 1.2  2023/03/30 23:35:19  cupsoft
*** empty log message ***

Revision 1.1  2022/12/19 00:43:47  cupsoft
add and clean up

Revision 1.2  2020/01/14 02:26:08  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/07/14 07:58:51  cupsoft
RawObjs

Revision 1.3  2016/03/17 08:12:11  jslee
add inheritance from TArray class
remove member array for waveform

Revision 1.2  2016/03/08 04:40:33  amore
*** empty log message ***

Revision 1.1.1.1  2016/02/29 08:25:13  cupsoft
RawObjs

**/
