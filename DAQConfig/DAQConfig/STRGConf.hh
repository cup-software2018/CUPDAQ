/*
 *
 *  Module:  STRGConf
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: Software trigger configuration class
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/03/23 04:32:42 $
 *  CVS/RCS Revision: $Revision: 1.2 $
 *  Status:           $State: Exp $
 *
 */

#ifndef STRGConf_hh
#define STRGConf_hh

#include "TString.h"

#include "DAQConfig/AbsConf.hh"
#include "OnlConsts/adcconsts.hh"

class STRGConf : public AbsConf {
public:
  STRGConf();
  virtual ~STRGConf();

  virtual void SetZSUMode(int val);
  virtual void SetPrescale(int val);
  virtual void SetInputCard(const char * fname);

  virtual int GetZSUMode() const;
  virtual int GetPrescale() const;
  virtual const char * GetInputCard() const;

  virtual void SetCID(int ch, int val) {}
  virtual void SetPID(int ch, int val) {}
  virtual int CID(int ch) const { return 0; }
  virtual int PID(int ch) const { return 0; }
  virtual void PrintConf() const {}

private:
  int fZSUMode;
  int fPrescale;
  TString fFilename;

  ClassDef(STRGConf, 1)
};

inline void STRGConf::SetZSUMode(int val) { fZSUMode = val; }
inline void STRGConf::SetPrescale(int val) { fPrescale = val; }
inline void STRGConf::SetInputCard(const char * fname) { fFilename = fname; }
inline int STRGConf::GetZSUMode() const { return fZSUMode; }
inline int STRGConf::GetPrescale() const { return fPrescale; }
inline const char * STRGConf::GetInputCard() const { return fFilename.Data(); }

#endif
