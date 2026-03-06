
#pragma once

#include <climits>
#include <getopt.h>
#include <iostream>
#include <vector>

#include "OnlConsts/adcconsts.hh"

struct daqopt {
  std::vector<ADC::TYPE> adctype;
  int runnum;
  int daqid;
  int daqevent;
  int daqtime;
  int rfreq;
  int sptime;
  const char * config;
  const char * output;
  bool dohist;
  bool dosend;
  int vlevel;

  void init()
  {
    adctype.clear();
    runnum = 0;
    daqid = 0;
    daqevent = 0;
    daqtime = 0;
    rfreq = 1;
    sptime = 60 * 60;
    config = nullptr;
    output = nullptr;
    dohist = false;
    dosend = false;
    vlevel = 0;
  }

  void print() const
  {
    std::cout << "run=" << runnum << " daqid=" << daqid << " ndaq=" << daqevent
              << " tdaq=" << daqtime << " rfre=" << rfreq << " spt=" << sptime
              << " conf=" << (config ? config : "none") << std::endl;
  }
};

static struct option const long_options[] = {{"fadc", no_argument, nullptr, 'f'},
                                             {"gadc", no_argument, nullptr, 'g'},
                                             {"madc", no_argument, nullptr, 'm'},
                                             {"iadc", no_argument, nullptr, 'i'},
                                             {"sadc", no_argument, nullptr, 's'},
                                             {"dohist", no_argument, nullptr, 'h'},
                                             {"dosend", no_argument, nullptr, 'x'},
                                             {"config", required_argument, nullptr, 'c'},
                                             {"output", required_argument, nullptr, 'o'},
                                             {"daqid", required_argument, nullptr, 'd'},
                                             {"daq-event", required_argument, nullptr, 'n'},
                                             {"daq-time", required_argument, nullptr, 't'},
                                             {"run-number", required_argument, nullptr, 'r'},
                                             {"report-frequency", required_argument, nullptr, 'q'},
                                             {"splitting-time", required_argument, nullptr, 'p'},
                                             {"verbose-level", required_argument, nullptr, 'v'},
                                             {nullptr, 0, nullptr, 0}};

const char * const short_options = "c:o:d:n:t:r:p:q:v:fgmishx";

void printusage(const char * daqname);
void optparse(daqopt & opt, int argc, char ** argv);