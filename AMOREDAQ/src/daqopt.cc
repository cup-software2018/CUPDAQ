#include <cstdlib>
#include <cstring>
#include <iostream>

#include "AMOREDAQ/daqopt.hh"

namespace {
const char * basename_const(const char * path)
{
  if (!path) { return ""; }
  const char * last_slash = std::strrchr(path, '/');
  if (last_slash) { return last_slash + 1; }
  return path;
}
} // namespace

const struct option long_options[] = {{"dohist", no_argument, nullptr, 'h'},
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

const char * const short_options = "c:o:d:n:t:r:p:q:v:hs";

void daqopt::init()
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

void daqopt::print() const
{
  std::cout << "run=" << runnum << " daqid=" << daqid << " ndaq=" << daqevent << " tdaq=" << daqtime
            << " rfre=" << rfreq << " spt=" << sptime << " conf=" << (config ? config : "") << std::endl;
}

void printusage(const char * daqname)
{
  const char * name = basename_const(daqname);

  std::cout << "AMoRE DAQ program for NOTICE ADCs" << std::endl;
  std::cout << std::endl;
  std::cout << "Usage: " << name << " [OPTION] [ARG] ..." << std::endl;
  std::cout << "Example: " << name << " -c config.file -o output.root" << std::endl;
  std::cout << std::endl;
  std::cout << "options:" << std::endl;
  std::cout << "  -c, --config            [daq config file]" << std::endl;
  std::cout << "  -o, --output            [output file]" << std::endl;
  std::cout << "  -r, --run-number        [run number]" << std::endl;
  std::cout << "  -d, --daqid             [DAQ ID]" << std::endl;
  std::cout << "  -t, --daq-time          [duration time for daq]" << std::endl;
  std::cout << "  -n, --daq-event         [number of event for daq]" << std::endl;
  std::cout << "  -h, --dohist            enable online histograming" << std::endl;
  //std::cout << "  -x, --dosend            doesn't write output, send event to merger" << std::endl;
  std::cout << "  -p, --splitting-time    [time for splitting output file]" << std::endl;
  std::cout << "  -q, --report-frequency  [reporting time]" << std::endl;
  std::cout << "  -v, --verbose-level     [verbose level]" << std::endl;
  std::cout << std::endl;
}

void optparse(daqopt & daq, int argc, char ** argv)
{
  while (true) {
    const auto opt = getopt_long(argc, argv, short_options, long_options, nullptr);

    if (opt == -1) { break; }

    switch (opt) {
      case 'h': daq.dohist = true; break;
      //case 'x': daq.dosend = true; break;
      case 'c': daq.config = optarg; break;
      case 'o': daq.output = optarg; break;
      case 'd': daq.daqid = std::atoi(optarg); break;
      case 'n': daq.daqevent = std::atoi(optarg); break;
      case 't': daq.daqtime = std::atoi(optarg); break;
      case 'r': daq.runnum = std::atoi(optarg); break;
      case 'q': daq.rfreq = std::atoi(optarg); break;
      case 'p': daq.sptime = std::atoi(optarg); break;
      case 'v': daq.vlevel = std::atoi(optarg); break;
      default: printusage(argv[0]); break;
    }
  }
}
