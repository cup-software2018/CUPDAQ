/*
 *
 *  Module:  AbsConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: Pure abstract class for configuration
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/07/18 12:25:11 $
 *  CVS/RCS Revision: $Revision: 1.10 $
 *  Status:           $State: Exp $
 *
 */

#ifndef AbsConf_hh
#define AbsConf_hh

#include "TNamed.h"
#include "OnlConsts/adcconsts.hh"

class AbsConf : public TNamed {
public:
  AbsConf();
  AbsConf(int sid, ADC::TYPE type = ADC::TCB);
  virtual ~AbsConf();

  virtual void SetEnable();
  virtual bool IsEnabled() const;

  virtual void SetLink();
  virtual bool IsLinked() const;

  virtual void SetSID(int id);
  virtual void SetMID(int id);

  virtual int SID() const;
  virtual int MID() const;

  virtual void SetCID(int ch, int val) = 0;
  virtual void SetPID(int ch, int val) = 0;

  virtual int CID(int ch) const = 0;
  virtual int PID(int ch) const = 0;

  virtual void SetADCType(ADC::TYPE type);
  virtual ADC::TYPE GetADCType() const;
  
  virtual void SetDAQID(int id);
  virtual int GetDAQID() const;

  virtual int Compare(const TObject * object) const;
  virtual const char * InfoStr() const;

  virtual void PrintConf() const = 0;

protected:
  bool fIsEnabled;
  bool fIsLinked;

  int fSID;
  int fMID;

  ADC::TYPE fADCType;
  int fDAQID;

ClassDef(AbsConf, 1)
};

inline void AbsConf::SetEnable()
{
  fIsEnabled = true;
}

inline bool AbsConf::IsEnabled() const
{
  return fIsEnabled;
}

inline void AbsConf::SetLink()
{
  fIsLinked = true;
}

inline bool AbsConf::IsLinked() const
{
  return fIsLinked;
}

inline void AbsConf::SetSID(int id)
{
  fSID = id;
}

inline void AbsConf::SetMID(int id)
{
  fMID = id;
}

inline int AbsConf::SID() const
{
  return fSID;
}

inline int AbsConf::MID() const
{
  return fMID;
}

inline void AbsConf::SetADCType(ADC::TYPE type)
{
  fADCType = type;
}

inline ADC::TYPE AbsConf::GetADCType() const
{
  return fADCType;
}

inline void AbsConf::SetDAQID(int id)
{
  fDAQID = id;
}

inline int AbsConf::GetDAQID() const
{
  return fDAQID;
}

#endif

/**
$Log: AbsConf.hh,v $
Revision 1.10  2023/07/18 12:25:11  cupsoft
*** empty log message ***

Revision 1.9  2022/12/19 01:01:11  cupsoft
add IADC things

Revision 1.8  2019/11/25 02:04:07  cupsoft
*** empty log message ***

Revision 1.7  2019/05/20 03:00:30  cupsoft
*** empty log message ***

Revision 1.6  2018/11/06 00:22:07  cupsoft
*** empty log message ***

Revision 1.5  2018/11/06 00:19:51  cupsoft
*** empty log message ***

Revision 1.4  2018/11/01 01:40:44  cupsoft
*** empty log message ***

Revision 1.3  2018/02/21 02:32:37  cupsoft
*** empty log message ***

Revision 1.2  2016/12/05 09:52:50  cupsoft
*** empty log message ***

Revision 1.1.1.1  2016/09/30 09:22:24  cupsoft
DAQConfig

Revision 1.3  2016/09/27 03:41:51  cupsoft
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
