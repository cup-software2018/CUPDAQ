//
// Created by jslee on 5/17/17.
//

#ifndef AMORECONSTS_HH
#define AMORECONSTS_HH

#include "OnlConsts/adcconsts.hh"

namespace AMORE {
const int kNCRYSTAL = 18;
// const char * kCRYSTALNAME[kNCRYSTAL] = {"SB28", "S35" , "SS68",
//                                         "SE01", "SB29", "SE02"};
const int kNADC = 3;
const int kCHUNKSIZE = kMINIMUMBCOUNTAMOREADC * kKILOBYTES;
const int kCHUNKNDP = kCHUNKSIZE / 64;
const int kRECORDLENGTH = 30000;
const int kRTRECORDLENGTH = 100000;
} // namespace AMORE

#endif // AMORECONSTS_HH
