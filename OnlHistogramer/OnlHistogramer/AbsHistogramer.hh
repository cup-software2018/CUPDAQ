#pragma once

#include <ctime>
#include <string>
#include <vector>

#include "TFile.h"
#include "TObjArray.h"

#include "DAQConfig/AbsConfList.hh"
#include "OnlConsts/adcconsts.hh"
#include "OnlObjs/BuiltEvent.hh"

using HistProxy = TObjArray;

class AbsHistogramer {
public:
  AbsHistogramer();
  virtual ~AbsHistogramer();

  virtual bool Open();
  virtual void Close();

  virtual void Book() = 0;
  virtual void Fill(BuiltEvent * bevent) = 0;
  virtual void Update();

  virtual void SetRunNumber(int run);
  virtual void SetADCType(ADC::TYPE type);
  virtual void SetConfigList(AbsConfList * configs);
  virtual void SetStartDatime(time_t time);
  virtual void SetVerboseLevel(int level);
  virtual void SetFilename(const char * fname);

protected:
  int fRunNumber{};
  ADC::TYPE fADCType{};
  AbsConfList * fConfigList{nullptr};

  time_t fStartDatime{};

  std::string fROOTFilename;
  TFile * fROOTFile{nullptr};
  std::vector<std::string> fROOTFileList;

  HistProxy * fHistProxy{nullptr};

  int fVerboseLevel{};
};

inline void AbsHistogramer::SetRunNumber(int run) { fRunNumber = run; }
inline void AbsHistogramer::SetADCType(ADC::TYPE type) { fADCType = type; }
inline void AbsHistogramer::SetConfigList(AbsConfList * configs) { fConfigList = configs; }
inline void AbsHistogramer::SetStartDatime(time_t time) { fStartDatime = time; }
inline void AbsHistogramer::SetVerboseLevel(int level) { fVerboseLevel = level; }
inline void AbsHistogramer::SetFilename(const char * fname) { fROOTFilename = fname; }
