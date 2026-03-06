#include <cstdlib>
#include <filesystem>
#include <iostream>

#include "DAQ/daqopt.hh"

using namespace std;

void printusage(const char * daqname)
{
  std::string name = std::filesystem::path(daqname).filename().string();

  cout << "CUP DAQ program for NOTICE ADCs\n\n"
       << "Usage: " << name << " [OPTION] [ARG] ...\n"
       << "Example: " << name << " [-f/g/m/i/s] -c config.file -o output.root\n\n"
       << "options:\n"
       << "  -f, --fadc              for FADC500\n"
       << "  -g, --gadc              for FADC125 (16 channels)\n"
       << "  -m, --madc              for FADC125 (4 channels)\n"
       << "  -i, --iadc              for IADC\n"
       << "  -s, --sadc              for SADC\n"
       << "  -c, --config            [daq config file]\n"
       << "  -o, --output            [output file]\n"
       << "  -r, --run-number        [run number]\n"
       << "  -d, --daqid             [DAQ ID]\n"
       << "  -t, --daq-time          [duration time for daq]\n"
       << "  -n, --daq-event         [number of event for daq]\n"
       << "  -h, --dohist            enable online histograming\n"
       << "  -x, --dosend            doesn't write output, send event to merger\n"
       << "  -p, --splitting-time    [time for splitting output file]\n"
       << "  -q, --report-frequency  [reporting time]\n"
       << "  -v, --verbose-level     [verbose level]\n"
       << endl;
}

void optparse(daqopt & daq, int argc, char ** argv)
{
  while (true) {
    const auto opt = getopt_long(argc, argv, short_options, long_options, nullptr);

    if (-1 == opt) break;

    switch (opt) {
      case 'f': daq.adctype.push_back(ADC::FADC); break;
      case 'g': daq.adctype.push_back(ADC::GADC); break;
      case 'm': daq.adctype.push_back(ADC::MADC); break;
      case 'i': daq.adctype.push_back(ADC::IADC); break;
      case 's': daq.adctype.push_back(ADC::SADC); break;
      case 'h': daq.dohist = true; break;
      case 'x': daq.dosend = true; break;
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