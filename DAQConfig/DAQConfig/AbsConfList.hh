/*
 *
 *  Module:  AbsConfList
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: Container class for config objects, inherited from TObjArray
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/01/12 03:46:12 $
 *  CVS/RCS Revision: $Revision: 1.8 $
 *  Status:           $State: Exp $
 *
 */

#ifndef AbsConfList_hh
#define AbsConfList_hh

#include "TObjArray.h"

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class AbsConfList : public TObjArray {
public:
  AbsConfList();
  virtual ~AbsConfList();

  virtual int GetNADC(ADC::TYPE type, bool isalive = false) const;

  virtual AbsConf * FindConfig(int mid) const;
  virtual AbsConf * FindConfig(const char * name, int mid) const;
  virtual AbsConf * FindConfig(ADC::TYPE type, int mid) const;
  virtual AbsConf * GetConfig(ADC::TYPE type, int n) const;

  virtual AbsConf * GetTCBConfig() const;
  virtual AbsConf * GetSTRGConfig(ADC::TYPE type) const;
  virtual AbsConf * GetDAQConfig() const;

  virtual void Dump() const;

  ClassDef(AbsConfList, 1)
};

#endif

/**
$Log: AbsConfList.hh,v $
Revision 1.8  2023/01/12 03:46:12  cupsoft
add DAQconfig

Revision 1.7  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.7  2020/07/10 02:38:36  cupsoft
*** empty log message ***

Revision 1.6  2019/11/25 02:04:07  cupsoft
*** empty log message ***

Revision 1.5  2018/01/08 05:14:45  cupsoft
*** empty log message ***

Revision 1.4  2017/04/06 12:11:55  cupsoft
*** empty log message ***

Revision 1.3  2016/12/09 04:35:59  cupsoft
*** empty log message ***

Revision 1.2  2016/10/02 10:55:13  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/09/30 09:22:24  cupsoft
DAQConfig

Revision 1.3  2016/09/19 07:34:04  cupsoft
*** empty log message ***

Revision 1.2  2016/09/05 05:26:48  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/07/14 07:42:30  cupsoft
DAQSys

Revision 1.1.1.1  2016/03/08 04:37:46  amore
DAQSts

Revision 1.1.1.1  2016/02/29 08:26:44  cupsoft
DAQSys

**/
