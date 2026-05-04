#pragma once

#include "Rtypes.h"

//
// Commands
//
const unsigned long kCONFIGRUN = 1;
const unsigned long kSTARTRUN = 2;
const unsigned long kENDRUN = 3;
const unsigned long kEXIT = 4;
const unsigned long kQUERYDAQSTATUS = 10;
const unsigned long kQUERYRUNINFO = 12;
const unsigned long kQUERYRUNSTATS = 13;
const unsigned long kQUERYTRGINFO = 14;
const unsigned long kQUERYANEVENT = 15;
const unsigned long kREQUESTCONFIG = 16;
const unsigned long kSPLITOUTPUTFILE = 17;
const unsigned long kRECVEVENT = 18;
const unsigned long kQUERYCHTRGINFO = 19;
const unsigned long kSETERROR = 20;
const unsigned long kQUERYMONITOR = 21;

namespace RUNSTATE
{
enum STATE
{
  kDOWN = 0,
  kBOOTED = 1,
  kCONFIGURED = 2,
  kRUNNING = 3,
  kRUNENDING = 4,
  kRUNENDED = 5,
  kPROCENDED = 6,
  kWARNING = 7,
  kERROR = 8,
};

inline STATE GetState(const std::atomic<unsigned long> & state)
{
  unsigned long val = state.load();
  for (int i = 1; i <= 16; i++) {
    if (val & (1UL << i)) return static_cast<STATE>(i);
  }
  return kDOWN;
}

inline STATE GetState(unsigned long val)
{
  for (int i = 1; i <= 16; i++) {
    if (val & (1UL << i)) return static_cast<STATE>(i);
  }
  return kDOWN;
}

inline void SetState(std::atomic<unsigned long> & state, STATE cstate)
{
  unsigned long oldval = state.load();
  unsigned long newval;
  do {
    newval = oldval;
    STATE pstate = GetState(newval);
    newval &= ~(1UL << pstate); // CLRBIT
    newval |= (1UL << cstate);  // SETBIT
  } while (!state.compare_exchange_weak(oldval, newval));
}

inline void SetError(std::atomic<unsigned long> & state) { state.fetch_or(1UL << kERROR); }

inline bool CheckError(const std::atomic<unsigned long> & state)
{
  return state.load() & (1UL << kERROR);
}

inline bool CheckState(const std::atomic<unsigned long> & state, STATE pstate)
{
  return state.load() & (1UL << pstate);
}
} // namespace RUNSTATE

//
// ERROR code
//
const unsigned long kNONE = 0;
const unsigned long kCONNECTION = 1;
const unsigned long kREADWRITE = 2;

//
// DAQ parameters
//
const bool kSADCDAQFAILSAFE = true;
const char * const kFORCEDENDRUNFILE = "/tmp/forced.endrun";
const char * const kDEFAULTOUTPUTDIR = "/data";

// root file compression level
const int kROOTFILECOMPRESSIONLEVEL = 1;

const int kTHREADSLEEP = 1000; // milisecond

// maximum event buffer size for reformatter
const double kMAXEVENTBUFFERSIZE = 1.0; // gigabytes

// output root file switching
const double kOUTPUTSWITCHINGTIME = 60 * 60; // seconds

// output file format
namespace OUTPUT
{
enum FORMAT
{
  NONE = 0,
  ROOT = 1,
  HDF5 = 2,
  GZIP = 3
};
}

// DAQ types
namespace DAQ
{
enum TYPE
{
  TCB = 0,
  STDDAQ = 1,
  TCBDAQ = 2,
  TCBCTRL = 3,
  MERGER = 4,
  AMORETCB = 10,
  AMOREDAQ = 11,
  NULLTCB = 90,
  NULLDAQ = 91,
  NULLMERGER = 92
};
enum EXPERIMENT
{
  NONE = 0,
  NEOS = 1,
  COSINE = 2,
  NEON = 3,
  AMORE1 = 4,
  AMORE2 = 5
};
} // namespace DAQ

// =========================================================================
// Network Port Offsets
// Offset values added to the Base Port to determine the port for each specific function.
// =========================================================================
namespace PORT_OFFSET
{
constexpr int CONTROL = 0;  // Control port (ZMQ REP - command reception)
constexpr int CONFIG = 10;  // Configuration sync port (ROOT TSocket - ConfigList transfer)
constexpr int DATA = 20;    // Data transfer port (ZMQ PUSH/PULL or TCP)
constexpr int MONITOR = 30; // Online monitoring port (Histogram, status streaming)
} // namespace PORT_OFFSET
