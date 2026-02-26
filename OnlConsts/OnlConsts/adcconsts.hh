#ifndef adcconsts_hh
#define adcconsts_hh

#include <string>

const unsigned long kKILO = 1024;
const unsigned long kMEGA = kKILO * kKILO;
const unsigned long kGIGA = kKILO * kKILO * kKILO;

const unsigned long kKILOBYTES = kKILO;
const unsigned long kMEGABYTES = kMEGA;
const unsigned long kGIGABYTES = kGIGA;

const double kDKILOBYTES = 1024.;
const double kDMEGABYTES = kDKILOBYTES * kDKILOBYTES;
const double kDGIGABYTES = kDKILOBYTES * kDKILOBYTES * kDKILOBYTES;

const unsigned long kMICROSECOND = 1000; // 1000 nano
const unsigned long kMILLISECOND = 1000 * kMICROSECOND;
const unsigned long kONESECOND = 1000 * kMILLISECOND;
const double kDONESECOND = kONESECOND;

namespace ADC {
enum TYPE {
  TCB = 0,
  FADC = 1,
  MADC = 2,
  GADC = 3,
  IADC = 4,
  SADC = 5,
  FADCT = 11,
  MADCT = 12,
  GADCT = 13,
  IADCT = 14,
  SADCT = 15,
  FADCS = 21,
  MADCS = 22,
  GADCS = 23,
  IADCS = 24,
  SADCS = 25,
  AMOREADC = 9,
  NONE = 100
};
enum MODE { FMODE = 1, SMODE = 2 };
} // namespace ADC

namespace TCB {
enum TYPE {
  V1 = 0,
  MINI = 1,
  V2 = 2,
  NEOS = 3,
  COSINE = 4,
  NEON = 5,
  GBAR = 6,
  AMORE1 = 7,
  AMORE2 = 8
};
}

namespace TRIGGER {
enum MODE { SELF = 1, MODULE = 2, GLOBAL = 3 };
}

// ALL ADC
const int kMINIMUMBCOUNT = 16;   // kbytes
const int kMAXIMUMBCOUNT = 4096; // kbytes

//
// SADC constants
//
const int kNCHSADC = 32;
const unsigned long kBYTESPEREVENTSADC = 256;

//
// FADC constants
//
const int kNCHFADC = 4;

//
// AMORE ADC constants
//
const int kMINIMUMBCOUNTAMOREADC = 2048; // kbytes
const int kNCHAMOREADC = 16;

//
// GADC constants
//
const int kNCHGADC = 16;

//
// MADC constants
//
const int kNCHMADC = 4;

//
// IADC constants
//
const int kNCHIADC = 40;
const unsigned long kBYTESPEREVENTIADC = 256;
#endif
