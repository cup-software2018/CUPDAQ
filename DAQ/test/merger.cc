/*
 *
 *  Module:  daq
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: CUP DAQ program for FADC/AMOREADC/SADC with run control
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/04/03 02:01:10 $
 *  CVS/RCS Revision: $Revision: 1.1 $
 *  Status:           $State: Exp $
 *
 */

#include "DAQ/CupDAQManager.hh"
#include "DAQ/daqopt.hh"
#include "DAQTrigger/CupSoftTrigger.hh"

int main(int argc, char ** argv)
{
  if (argc < 2) {
    printusage(argv[0]);
    return 0;
  }

  daqopt option;
  option.init();  
  optparse(option, argc, argv);

  // for TCB controlled ADC
  ADC::TYPE adctype = static_cast<ADC::TYPE>(static_cast<int>(option.adctype[0]) + 10);  

  auto * DAQ = new CupDAQManager();
  DAQ->SetDAQType(DAQ::MERGER);
  DAQ->SetRunNumber(option.runnum);
  DAQ->SetADCType(adctype);
  DAQ->SetDAQID(option.daqid);
  DAQ->SetConfigFilename(option.config);
  DAQ->SetTriggerMonTime(option.rfreq);
  DAQ->SetVerboseLevel(option.vlevel);
  if (option.dohist) DAQ->EnableHistograming();

  auto * swtrigger = new CupSoftTrigger();
  swtrigger->SetVerboseLevel(option.vlevel);
  DAQ->SetSoftTrigger(swtrigger);

  DAQ->Run();

  delete swtrigger;
  delete DAQ;

  return 0;
}

/**
$Log: merger.cc,v $
Revision 1.1  2023/04/03 02:01:10  cupsoft
*** empty log message ***

Revision 1.14  2023/02/09 07:24:04  cupsoft
*** empty log message ***

Revision 1.13  2022/12/21 00:02:44  cupsoft
*** empty log message ***

Revision 1.13  2020/07/10 02:37:31  cupsoft
*** empty log message ***

Revision 1.12  2019/07/24 06:14:57  cupsoft
*** empty log message ***

Revision 1.11  2019/07/11 08:21:35  cupsoft
*** empty log message ***

Revision 1.10  2019/07/05 00:25:04  cupsoft
*** empty log message ***

**/
