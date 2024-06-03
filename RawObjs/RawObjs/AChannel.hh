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
  unsigned int fADC = 0;  // ADC count
  unsigned int fTime = 0; // Hit time

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
