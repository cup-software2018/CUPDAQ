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
#include "TObject.h"

typedef TClonesArray AChannelColl;

class AChannel;
class AChannelData : public TObject {
public:
  AChannelData();
  AChannelData(const AChannelData & data);
  virtual ~AChannelData();

  virtual void Clear(const Option_t * opt = "");
  virtual void Dump() const;

  AChannel * Add();
  AChannel * Add(unsigned short id);
  AChannel * Add(AChannel * ch);

  int GetN() const;
  AChannel * Get(int n) const;
  AChannel * GetChannel(unsigned short id) const;
  AChannelColl * GetColl() const;
  void CopyFrom(const AChannelData * data);

private:
  int fNCh; //! just for counter
  AChannelColl * fColl;

  ClassDef(AChannelData, 1)
};

//
// Inline functions
//

inline AChannelColl * AChannelData::GetColl() const { return fColl; }
inline int AChannelData::GetN() const { return fColl->GetEntries(); }

inline AChannel * AChannelData::Get(int n) const
{
  return (AChannel *)fColl->At(n);
}

#endif

/**
$Log: AChannelData.hh,v $
Revision 1.3  2023/03/30 23:35:19  cupsoft
*** empty log message ***

Revision 1.2  2022/12/19 00:43:47  cupsoft
add and clean up

Revision 1.1.1.1  2016/07/14 07:58:51  cupsoft
RawObjs

Revision 1.1.1.1  2016/02/29 08:25:13  cupsoft
RawObjs

**/
