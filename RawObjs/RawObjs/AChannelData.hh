/*
 *
 *  Module:  AChannelData/RawObjs
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: TClonesArray for AChannels
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/03/30 23:35:19 $
 *  CVS/RCS Revision: $Revision: 1.3 $
 *  Status:           $State: Exp $
 *
 */

#ifndef AChannelData_hh
#define AChannelData_hh

#include "TClonesArray.h"

class AChannel;
class AChannelData : public TClonesArray {
public:
  AChannelData();
  AChannelData(const AChannelData & data);
  virtual ~AChannelData();

  virtual void Clear(const Option_t * opt = "");

  AChannel * Add();
  AChannel * Add(unsigned short id);
  AChannel * Add(AChannel * ch);

  int GetN() const;
  AChannel * Get(int n) const;
  AChannel * GetChannel(unsigned short id) const;
  void CopyFrom(const AChannelData * data);

private:
  int fN; //!  just for counter

  ClassDef(AChannelData, 1)
};

//
// Inline functions
//

inline int AChannelData::GetN() const { return GetEntriesFast(); }

inline AChannel * AChannelData::Get(int n) const
{
  return (AChannel *)At(n);
}

#endif

