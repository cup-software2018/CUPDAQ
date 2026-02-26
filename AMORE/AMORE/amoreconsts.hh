#pragma once

#include "OnlConsts/onlconsts.hh"

namespace AMORE {
const int kMINIMUMBCOUNT = 4096; // kbytes
const int kNCHPERADC = 16;

const int kNCRYSTAL = 18;
// const char * kCRYSTALNAME[kNCRYSTAL] = {"SB28", "S35" , "SS68",
//                                         "SE01", "SB29", "SE02"};
const int kNADC = 3;
const int kCHUNKSIZE = kMINIMUMBCOUNT * kKILOBYTES;
const int kCHUNKNDP = kCHUNKSIZE / 64;
const int kRECORDLENGTH = 30000;
const int kRTRECORDLENGTH = 100000;
} // namespace AMORE