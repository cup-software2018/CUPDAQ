#pragma once

#include <getopt.h>
#include <vector>

#include "OnlConsts/adcconsts.hh"

struct daqopt {
  std::vector<ADC::TYPE> adctype;
  int runnum{0};
  int daqid{0};
  int daqevent{0};
  int daqtime{0};
  int rfreq{1};
  int sptime{60 * 60};
  const char * config{nullptr};
  const char * output{nullptr};
  bool dohist{false};
  bool dosend{false};
  int vlevel{0};

  void init();
  void print() const;
};

extern const struct option long_options[];
extern const char * const short_options;

void printusage(const char * daqname);
void optparse(daqopt & opt, int argc, char ** argv);
