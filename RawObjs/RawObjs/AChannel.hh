/*
 *
 *  Module:  AChannel/RawObjs
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: A Class for storing ADC information
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/03/30 23:35:19 $
 *  CVS/RCS Revision: $Revision: 1.3 $
 *  Status:           $State: Exp $
 *
 */

#ifndef AChannel_hh
#define AChannel_hh

#include "RawObjs/AbsChannel.hh"

class AChannel : public AbsChannel {
public:
  AChannel();
  AChannel(unsigned short id);
  AChannel(const AChannel & ch);

  virtual ~AChannel();

  virtual void SetADC(unsigned int count);
  virtual void SetTime(unsigned int time);

  virtual unsigned int GetADC() const;
  virtual unsigned int GetTime() const;

protected:
  unsigned int fADC;  // ADC count
  unsigned int fTime; // Hit time

  ClassDef(AChannel, 1)
};

//
// Inline functions
//

inline void AChannel::SetADC(unsigned int count) { fADC = count; }
inline void AChannel::SetTime(unsigned int time) { fTime = time; }
inline unsigned int AChannel::GetADC() const { return fADC; }
inline unsigned int AChannel::GetTime() const { return fTime; }

#endif

/**
$Log: AChannel.hh,v $
Revision 1.3  2023/03/30 23:35:19  cupsoft
*** empty log message ***

Revision 1.2  2022/12/19 00:43:47  cupsoft
add and clean up

Revision 1.1.1.1  2016/07/14 07:58:51  cupsoft
RawObjs

Revision 1.1.1.1  2016/02/29 08:25:13  cupsoft
RawObjs

**/
