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
  unsigned short fID = 0;  // channel id
  unsigned short fBit = 0; // trigger bit

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

