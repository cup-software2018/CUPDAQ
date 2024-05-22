/*
 *
 *  Module:  FChannelData/RawObjs
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: TClonesArray for FChannels
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/03/30 23:35:19 $
 *  CVS/RCS Revision: $Revision: 1.3 $
 *  Status:           $State: Exp $
 *
 */

#ifndef FChannelData_hh
#define FChannelData_hh

#include "TClonesArray.h"

class FChannel;
class FChannelData : public TClonesArray {
public:
  FChannelData();
  FChannelData(const FChannelData & data);
  virtual ~FChannelData();

  virtual void Clear(const Option_t * opt = "");

  FChannel * Add();
  FChannel * Add(unsigned short id, int ndp);
  FChannel * Add(unsigned short id, int ndp, const unsigned short * wave);
  FChannel * Add(FChannel * ch);

  int GetN() const;
  FChannel * Get(int n) const;
  FChannel * GetChannel(unsigned short id) const;
  void CopyFrom(const FChannelData * data);

private:
  int fN; //!  just for counter

  ClassDef(FChannelData, 1)
};

//
// Inline functions
//

inline int FChannelData::GetN() const { return GetEntriesFast(); }

inline FChannel * FChannelData::Get(int n) const
{
  return (FChannel *)At(n);
}

#endif

