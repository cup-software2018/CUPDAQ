#ifndef onlconsts_hh
#define onlconsts_hh

#include "Rtypes.h"

//
// for ELog Server
//
const char * const kELOGIPADDR = "localhost";
const int kELOGPORT = 7800;

//
// For socket communication
//
const int kMESSLEN = 32;

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

namespace RUNSTATE {
enum STATE {
  kDOWN = 0,
  kBOOTED = 1,
  kCONFIGURED = 2,
  kRUNNING = 3,
  kRUNENDED = 4,
  kPROCENDED = 5,
  kWARNING = 6,
  kERROR = 7,
  kEXIT = 8
};

inline STATE GetState(unsigned long state)
{
  for (int i = 1; i <= 16; i++) {
    if (TESTBIT(state, i)) return (STATE)i;
  }
  return kDOWN;
}

inline void SetState(unsigned long & state, STATE cstate)
{
  STATE pstate = GetState(state);
  CLRBIT(state, pstate);
  SETBIT(state, cstate);
}

inline void SetError(unsigned long & state) { SETBIT(state, kERROR); }

inline bool CheckError(unsigned long state) { return TESTBIT(state, kERROR); }

inline bool CheckState(unsigned long state, STATE pstate)
{
  return TESTBIT(state, pstate);    
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
namespace OUTPUT {
enum FORMAT { NONE = 0, ROOT = 1, HDF5 = 2, GZIP = 3 };
}

// DAQ types
namespace DAQ {
enum TYPE {
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
enum EXPERIMENT {
  NONE = 0,
  NEOS = 1,
  COSINE = 2,
  NEON = 3,
  AMORE1 = 4,
  AMORE2 = 5
};
} // namespace DAQ

#endif
