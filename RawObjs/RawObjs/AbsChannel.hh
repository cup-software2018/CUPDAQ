/*
 *
 *  Module:  AbsChannel/RawObjs
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: An Abstract Class for storing basic channel information
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/03/30 23:35:19 $
 *  CVS/RCS Revision: $Revision: 1.3 $
 *  Status:           $State: Exp $
 *
 */

#ifndef AbsChannel_hh
#define AbsChannel_hh

#include "TObject.h"

class AbsChannel : public TObject {
public:
  AbsChannel();
  AbsChannel(unsigned short id);
  AbsChannel(const AbsChannel & ch);

  virtual ~AbsChannel();

  virtual void SetID(unsigned short id);
  virtual void SetBit(unsigned short bit);

  virtual unsigned short GetID() const;
  virtual unsigned short GetBit() const;

protected:
  unsigned short fID;  // channel id
  unsigned short fBit; // trigger bit

  ClassDef(AbsChannel, 1)
};

//
// Inline functions
//

inline void AbsChannel::SetID(unsigned short id) { fID = id; }
inline void AbsChannel::SetBit(unsigned short bit) { fBit = bit; }
inline unsigned short AbsChannel::GetID() const { return fID; }
inline unsigned short AbsChannel::GetBit() const { return fBit; }

#endif

/**
$Log: AbsChannel.hh,v $
Revision 1.3  2023/03/30 23:35:19  cupsoft
*** empty log message ***

Revision 1.2  2022/12/19 00:43:47  cupsoft
add and clean up

Revision 1.1.1.1  2016/07/14 07:58:51  cupsoft
RawObjs

Revision 1.1.1.1  2016/02/29 08:25:13  cupsoft
RawObjs

**/
