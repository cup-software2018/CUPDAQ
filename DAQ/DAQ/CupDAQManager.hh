// CupDAQManager.hh
#pragma once

#include <chrono>
#include <ctime>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <vector>
#include <zmq.hpp>

#include "TBenchmark.h"
#include "TFile.h"
#include "TObjArray.h"
#include "TTree.h"

#include "DAQ/CupGeneralTCB.hh"
#include "DAQConfig/AbsConf.hh"
#include "DAQConfig/AbsConfList.hh"
#include "DAQSystem/AbsADC.hh"
#include "DAQTrigger/AbsSoftTrigger.hh"
#include "DAQUtils/ConcurrentDeque.hh"
#include "DAQUtils/ELog.hh"
#include "DAQUtils/Json.hh"
#ifdef ENABLE_HDF5
#include "HDF5Utils/AbsH5Event.hh"
#include "HDF5Utils/H5DataWriter.hh"
#endif
#include "OnlConsts/adcconsts.hh"
#include "OnlConsts/onlconsts.hh"
#include "OnlObjs/AbsADCRaw.hh"
#include "OnlObjs/BuiltEvent.hh"

class CupDAQManager : public TObjArray {
public:
  enum PROCSTATE { NONE, READY, RUNNING, ENDED, ERROR };

  CupDAQManager();
  ~CupDAQManager() override;

  void SetRunNumber(int run);
  void SetDAQID(int id);
  void SetDAQType(DAQ::TYPE type);
  void SetADCType(ADC::TYPE type);
  void SetTriggerMode(TRIGGER::MODE mode);
  void SetSoftTrigger(AbsSoftTrigger * trigger);
  void SetConfigFilename(const char * name);
  void SetMinimumBCount(int val);

  void UseEventMerger();

  virtual void AddADC(AbsADC * adc);
  virtual bool AddADC(AbsConf * conf);
  virtual bool AddADC(AbsConfList * conflist);

  virtual AbsADC * FindADC(int sid);
  virtual int FindADCAt(int sid);

  virtual bool OpenDAQ();
  virtual void CloseDAQ();
  virtual bool PrepareDAQ();
  virtual bool ConfigureDAQ();
  virtual bool InitializeDAQ();
  virtual void StartTrigger();
  virtual void StopTrigger();

  virtual int ReadBCount(int n);
  virtual int ReadBCountMin(int * bcounts = nullptr);
  virtual int ReadBCountMax(int * bcounts = nullptr);
  virtual int ReadADCData(int n, int bcount, unsigned char * databuffer = nullptr);
  virtual int ReadData(int bcount, unsigned char ** databuffer);

  void SetOutputFileFormat(OUTPUT::FORMAT format);
  void SetOutputFilename(const char * fname);
  void SetCompressionLevel(int level);
  void SetOutputSplitTime(int time);

  void SetVerboseLevel(int level);
  void SetTriggerMonTime(int time);
  void SetNEvent(int n);
  void SetDAQTime(int t);
  void EnableHistograming();

  virtual void Run();

  void TF_TriggerMon();
  void TF_DebugMon();
  void TF_RunManager();
  void TF_MsgServer();
  void TF_DataServer();
  void TF_ReadData();
  void TF_SortEvent();
  void TF_BuildEvent();
  void TF_WriteEvent();
  void TF_SplitOutput(bool ontcb);
  void TF_Histogramer();
  void TF_ShrinkToFit();

  void TF_SendEvent();
  void TF_MergeEvent();

protected:
  bool IsStandaloneDAQ() const;

  int GetNADC() const;
  int GetNDP() const;
  int GetADCEventDataSize() const;
  int GetADCChannelDataSize() const;

  void RC_TCB();
  void RC_STDDAQ();
  void RC_TCBDAQ();
  void RC_TCBCTRLDAQ();
  void RC_MERGER();
  void RC_NullTCB();
  void RC_NullDAQ();
  void RC_NullMERGER();

  void ReadData_GLT();
  void ReadData_MOD();
  void SortEvent_MOD();
  void SortEvent_CHA();
  void BuildEvent_GLT();
  void BuildEvent_MOD();
  void BuildEvent_SLF();
  void WriteSADC_MOD_ROOT();
  void WriteFADC_MOD_ROOT();
  void WriteSADC_MOD_HDF5();
  void WriteFADC_MOD_HDF5();
  void WriteSADC_MOD_GZIP();
  void WriteFADC_MOD_GZIP();

  bool OpenNewOutputFile();
  long OpenNewROOTFile(const char * fname);
  long OpenNewHDF5File(const char * fname);
  long OpenNewGZIPFile(const char * fname);
  long SwitchRootFile(TFile *& oldfile, TFile * newfile);
  void CloseHDF5Output();

  void CheckEventSanity(ADCHeader ** header, unsigned int * tn, unsigned long * tt, int * status);
  virtual void PrintDAQSummary();

  bool ThreadWait(unsigned long & state, bool & exit);
  void ThreadSleep(int & sleep, double & perror, double & integral, int size, int tsize = 1,
                   double ki = 0.01);

  nlohmann::json SendCommandToDAQ(const std::unique_ptr<zmq::socket_t> & socket_ptr,
                                  const std::string & cmd, std::string & daq_name);
  nlohmann::json SendCommandToDAQ(const std::unique_ptr<zmq::socket_t> & socket_ptr,
                                  const std::string & cmd);
  void SendCommandToDAQs(const std::string & cmd);

  unsigned long QueryDAQStatus(const std::unique_ptr<zmq::socket_t> & socket_ptr,
                               std::string & daq_name);
  unsigned long QueryDAQStatus(const std::unique_ptr<zmq::socket_t> & socket_ptr);

  bool WaitDAQStatus(RUNSTATE::STATE status);
  bool IsDAQRunning();
  bool IsDAQFail();

  bool WaitState(unsigned long & state, RUNSTATE::STATE pstate, bool errorexit = true);
  int WaitCommand(bool & isgo);
  int WaitCommand(bool & isgo, bool & exit);
  int WaitCommand(bool & isgo, unsigned long & state);
  bool IsForcedEndRunFile(bool useRC = false);

  const char * GetADCName() const;
  const char * GetADCName(ADC::TYPE type) const;

  void StartBenchmark(const char * name);
  void StopBenchmark(const char * name);

protected:
  int fRunNumber;
  int fSubRunNumber;

  int fDAQID;
  std::string fDAQName;
  int fDAQPort;

  DAQ::TYPE fDAQType;
  ADC::TYPE fADCType;
  ADC::MODE fADCMode;
  TRIGGER::MODE fTriggerMode;

  std::string fConfigFilename;
  AbsConfList * fConfigList;

  std::unique_ptr<CupGeneralTCB> fTCB;

  zmq::context_t fZMQContext{1};
  std::vector<std::unique_ptr<zmq::socket_t>> fDAQSocket;

  int fRecordLength;
  int fMinimumBCount;
  int fADCEventDataSize;
  int fADCChannelDataSize;
  int fNDP;

  bool fIsOwnADC;
  bool fIsDAQOpen;

  unsigned long fRunStatus;
  unsigned long fErrorCode;
  bool fDoConfigRun;
  bool fDoStartRun;
  bool fDoEndRun;
  bool fDoExit;
  bool fDoSplitOutputFile;

  unsigned long fRunStatusTCB;
  unsigned long fErrorCodeTCB;
  bool fDoConfigRunTCB;
  bool fDoStartRunTCB;
  bool fDoEndRunTCB;
  bool fDoExitTCB;
  bool fDoSplitOutputFileTCB;

  PROCSTATE fReadStatus;
  PROCSTATE fSortStatus;
  PROCSTATE fBuildStatus;
  PROCSTATE fWriteStatus;

  int fReadSleep;
  int fSortSleep;
  int fBuildSleep;
  int fWriteSleep;

  std::vector<ConcurrentDeque<std::unique_ptr<AbsADCRaw>> *> fADCRawBuffers;
  ConcurrentDeque<std::unique_ptr<BuiltEvent>> fBuiltEventBuffer1;
  ConcurrentDeque<std::unique_ptr<BuiltEvent>> fBuiltEventBuffer2;

  OUTPUT::FORMAT fOutputFileFormat;
  std::string fOutputFilename;
  int fCompressionLevel;
  int fOutputSplitTime;
  std::vector<const char *> fOutputFileList;

  TFile * fROOTFile;
  TTree * fROOTTree;

#ifdef ENABLE_HDF5
  H5DataWriter * fHDF5File;
  AbsH5Event * fH5Event;
#endif

  std::string fHistFilename;
  bool fDoHistograming;
  bool fHistogramerEnded;
  int fHistSleep;

  int fVerboseLevel;
  int fTriggerMonTime;
  int * fRemainingBCount;
  unsigned long fCurrentTime;
  unsigned long fTriggerTime;
  unsigned int fTriggerNumber;
  unsigned int fNBuiltEvent;
  double fTotalReadDataSize;
  double fTotalWrittenDataSize;

  time_t fStartDatime;
  time_t fEndDatime;

  int fSetNEvent;
  int fSetDAQTime;

  unsigned long fMonitorServerOn;

  TBenchmark * fBenchmark;

  AbsSoftTrigger * fSoftTrigger;

  std::mutex fMonitorMutex;
  std::mutex fHistogramMutex;
  std::mutex fSTDADCMutex;
  std::mutex fWriteFileMutex;
  std::mutex fBenchmarkMutex;
  std::mutex fRecvBufferMutex;

  std::string fADCName;

  bool fDoSendEvent;
  using BuiltEventQueue = ConcurrentDeque<std::unique_ptr<BuiltEvent>>;
  using RecvBufferEntry = std::pair<int, std::unique_ptr<BuiltEventQueue>>;
  std::vector<RecvBufferEntry> fRecvEventBuffer;

  PROCSTATE fSendStatus;
  PROCSTATE fRecvStatus;
  PROCSTATE fMergeStatus;

  int fSendSleep;
  int fMergeSleep;

  std::string fMergeServerIPAddr;
  int fMergeServerPort;
  unsigned long fTotalRawDataSize;

  ClassDef(CupDAQManager, 0)
};

inline void CupDAQManager::SetRunNumber(int run) { fRunNumber = run; }

inline void CupDAQManager::SetDAQID(int id) { fDAQID = id; }

inline void CupDAQManager::SetDAQType(DAQ::TYPE type) { fDAQType = type; }

inline void CupDAQManager::SetADCType(ADC::TYPE type)
{
  fADCType = type;

  switch (type) {
    case ADC::SADCS: fADCName = "SADC"; break;
    case ADC::SADCT: fADCName = "SADC"; break;
    case ADC::FADCS: fADCName = "FADC"; break;
    case ADC::FADCT: fADCName = "FADC"; break;
    case ADC::GADCS: fADCName = "GADC"; break;
    case ADC::GADCT: fADCName = "GADC"; break;
    case ADC::MADCS: fADCName = "MADC"; break;
    default: break;
  }
}

inline void CupDAQManager::SetTriggerMode(TRIGGER::MODE mode) { fTriggerMode = mode; }

inline void CupDAQManager::SetSoftTrigger(AbsSoftTrigger * trigger) { fSoftTrigger = trigger; }

inline void CupDAQManager::SetConfigFilename(const char * name) { fConfigFilename = name; }

inline void CupDAQManager::SetMinimumBCount(int val) { fMinimumBCount = val; }

inline void CupDAQManager::UseEventMerger() { fDoSendEvent = true; }

inline void CupDAQManager::EnableHistograming() { fDoHistograming = true; }

inline void CupDAQManager::SetVerboseLevel(int level) { fVerboseLevel = level; }

inline void CupDAQManager::SetTriggerMonTime(int time) { fTriggerMonTime = time; }

inline void CupDAQManager::SetOutputFileFormat(OUTPUT::FORMAT format)
{
  fOutputFileFormat = format;
}

inline void CupDAQManager::SetOutputFilename(const char * fname) { fOutputFilename = fname; }

inline void CupDAQManager::SetCompressionLevel(int level) { fCompressionLevel = level; }

inline void CupDAQManager::SetOutputSplitTime(int time) { fOutputSplitTime = time; }

inline void CupDAQManager::SetNEvent(int n) { fSetNEvent = n; }

inline void CupDAQManager::SetDAQTime(int t) { fSetDAQTime = t; }

inline bool CupDAQManager::IsStandaloneDAQ() const
{
  return (fADCType == ADC::FADCS || fADCType == ADC::SADCS || fADCType == ADC::GADCS ||
          fADCType == ADC::MADCS);
}

inline int CupDAQManager::GetNADC() const { return GetEntries(); }

inline int CupDAQManager::GetNDP() const
{
  int ndp = 0;
  switch (fADCType) {
    case ADC::SADCS: break;
    case ADC::SADCT: break;
    case ADC::FADCS: ndp = 64 * fRecordLength - 16; break;
    case ADC::FADCT: ndp = 64 * fRecordLength - 16; break;
    case ADC::GADCS: ndp = 16 * fRecordLength - 2; break;
    case ADC::GADCT: ndp = 16 * fRecordLength - 2; break;
    case ADC::MADCS: ndp = 16 * fRecordLength - 16; break;
    case ADC::IADCT: {
      if (fRecordLength > 0) { ndp = 8 * fRecordLength - 2; }
      else {
        ndp = 0;
      }
      break;
    }
    default: break;
  }

  return ndp;
}

inline int CupDAQManager::GetADCEventDataSize() const
{
  int dataSize = 0;

  switch (fADCType) {
    case ADC::SADCS: dataSize = kBYTESPEREVENTSADC; break;
    case ADC::SADCT: dataSize = kBYTESPEREVENTSADC; break;
    case ADC::FADCS: dataSize = kNCHFADC * 128 * fRecordLength; break;
    case ADC::FADCT: dataSize = kNCHFADC * 128 * fRecordLength; break;
    case ADC::GADCS: dataSize = kNCHGADC * 32 * fRecordLength; break;
    case ADC::GADCT: dataSize = kNCHGADC * 32 * fRecordLength; break;
    case ADC::MADCS: dataSize = kNCHMADC * 32 * fRecordLength; break;
    case ADC::IADCT: {
      if (fRecordLength > 0) { dataSize = 512 * fRecordLength; }
      else {
        dataSize = kBYTESPEREVENTIADC;
      }
      break;
    }
    default: break;
  }

  return dataSize;
}

inline int CupDAQManager::GetADCChannelDataSize() const
{
  int dataSize = 0;

  switch (fADCType) {
    case ADC::SADCS: dataSize = 0; break;
    case ADC::SADCT: dataSize = 0; break;
    case ADC::FADCS: dataSize = 128 * fRecordLength; break;
    case ADC::FADCT: dataSize = 128 * fRecordLength; break;
    case ADC::GADCS: dataSize = 32 * fRecordLength; break;
    case ADC::GADCT: dataSize = 32 * fRecordLength; break;
    case ADC::MADCS: dataSize = 32 * fRecordLength; break;
    case ADC::IADCT: {
      if (fRecordLength > 0) { dataSize = 128 * fRecordLength; }
      else {
        dataSize = 0;
      }
      break;
    }
    default: break;
  }

  return dataSize;
}

inline const char * CupDAQManager::GetADCName() const { return fADCName.c_str(); }

inline const char * CupDAQManager::GetADCName(ADC::TYPE type) const
{
  switch (type) {
    case ADC::SADCS:
    case ADC::SADCT: return "SADC";

    case ADC::FADCS:
    case ADC::FADCT: return "FADC";

    case ADC::GADCS:
    case ADC::GADCT: return "GADC";

    case ADC::IADCT: return "IADC";
    case ADC::MADCS: return "MADC";

    default: return "Unknown"; // 안전을 위해 기본값 제공
  }
}