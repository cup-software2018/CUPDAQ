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
 *  CVS/RCS Revision: $Revision: 1.2 $
 *  Status:           $State: Exp $
 *
 */

#ifndef FChannelData_hh
#define FChannelData_hh

#include "TClonesArray.h"
#include "TObject.h"

typedef TClonesArray FChannelColl;

class FChannel;
class FChannelData : public TObject {
public:
  FChannelData();
  FChannelData(const FChannelData & data);
  virtual ~FChannelData();

  virtual void Clear(const Option_t * opt = "");
  virtual void Dump() const;

  FChannel * Add();
  FChannel * Add(unsigned short id, int ndp);
  FChannel * Add(unsigned short id, int ndp, const unsigned short * wave);
  FChannel * Add(FChannel * ch);

  int GetN() const;
  FChannel * Get(int n) const;
  FChannel * GetChannel(unsigned short id) const;
  FChannelColl * GetColl() const;

  void CopyFrom(const FChannelData * data);

private:
  int fNCh; //! just for counter
  FChannelColl * fColl;

  ClassDef(FChannelData, 1)
};

//
// Inline functions
//

inline FChannelColl * FChannelData::GetColl() const { return fColl; }
inline int FChannelData::GetN() const { return fColl->GetEntries(); }

inline FChannel * FChannelData::Get(int n) const
{
  return (FChannel *)fColl->At(n);
}

#endif

/**
$Log: FChannelData.hh,v $
Revision 1.2  2023/03/30 23:35:19  cupsoft
*** empty log message ***

Revision 1.1  2022/12/19 00:43:47  cupsoft
add and clean up

Revision 1.2  2020/01/14 02:26:08  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/07/14 07:58:51  cupsoft
RawObjs

Revision 1.2  2016/03/17 08:12:12  jslee
add inheritance from TArray class
remove member array for waveform

Revision 1.1.1.1  2016/02/29 08:25:13  cupsoft
RawObjs

**/
