#include <iostream>

#include "TString.h"
#include "TSystem.h"

#include "DAQ/daqopt.hh"

using namespace std;

void printusage(const char* daqname)
{
  const char* name = gSystem->BaseName(daqname);

  cout << "CUP DAQ program for NOTICE ADCs" << endl;
  cout << endl;
  cout << Form("Usage: %s [OPTION] [ARG] ...", name) << endl;
  cout << Form("Example: %s [-f/g/m/i/s] -c config.file -o output.root", name)
       << endl;
  cout << endl;
  cout << "options:" << endl;
  cout << "  -f, --fadc              for FADC500" << endl;
  cout << "  -g, --gadc              for FADC125 (16 channels)" << endl;
  cout << "  -m, --madc              for FADC125 (4 channels)" << endl;
  cout << "  -i, --iadc              for SIADC" << endl;
  cout << "  -s, --sadc              for SADC" << endl;
  cout << "  -c, --config            [daq config file]" << endl;
  cout << "  -o, --output            [output file]" << endl;
  cout << "  -r, --run-number        [run number]" << endl;
  cout << "  -d, --daqid             [DAQ ID]" << endl;
  cout << "  -t, --daq-time          [duration time for daq]" << endl;
  cout << "  -n, --daq-event         [number of event for daq]" << endl;
  cout << "  -h, --dohist            enable online histograming" << endl;
  cout << "  -x, --dosend            doesn't write output, send event to merger" << endl;  
  cout << "  -p, --splitting-time    [time for splitting output file]" << endl;
  cout << "  -q, --report-frequency  [reporting time]" << endl;
  cout << "  -v, --verbose-level     [verbose level]" << endl;
  cout << endl;
}

void optparse(daqopt& daq, int argc, char** argv)
{
  while (true) {
    const auto opt = getopt_long(argc, argv, short_options, long_options, nullptr);

    if (-1 == opt)
      break;

    switch (opt) {
      case 'f':
        daq.adctype.push_back(ADC::FADC);
        break;
      case 'g':
        daq.adctype.push_back(ADC::GADC);
        break;
      case 'm':
        daq.adctype.push_back(ADC::MADC);
        break;
      case 'i':
        daq.adctype.push_back(ADC::IADC);
        break;
      case 's':
        daq.adctype.push_back(ADC::SADC);
        break;
      case 'h':
        daq.dohist = true;
        break;
      case 'x':
        daq.dosend = true;
        break;        
      case 'c':
        daq.config = optarg;
        break;
      case 'o':
        daq.output = optarg;
        break;
      case 'd':
        daq.daqid = TString(optarg).Atoi();
        break;
      case 'n':
        daq.daqevent = TString(optarg).Atoi();
        break;        
      case 't':
        daq.daqtime = TString(optarg).Atoi();
        break;
      case 'r':
        daq.runnum = TString(optarg).Atoi();
        break;
      case 'q':
        daq.rfreq = TString(optarg).Atoi();
        break;
      case 'p':
        daq.sptime = TString(optarg).Atoi();
        break;
      case 'v':
        daq.vlevel = TString(optarg).Atoi();
        break;
      default:
        printusage(argv[0]);
        break;
    }
  }
}