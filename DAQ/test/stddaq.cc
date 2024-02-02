/*
 *
 *  Module:  stddaq
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: Console DAQ program for standalone FADC/AMOREADC/SADC
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/04/04 02:37:18 $
 *  CVS/RCS Revision: $Revision: 1.8 $
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

  // for standalone ADC
  ADC::TYPE adctype =
      static_cast<ADC::TYPE>(static_cast<int>(option.adctype[0]) + 20);  

  auto * DAQ = new CupDAQManager();
  DAQ->SetDAQType(DAQ::STDDAQ);
  DAQ->SetRunNumber(option.runnum);  
  DAQ->SetADCType(adctype);
  DAQ->SetTriggerMode(TRIGGER::GLOBAL);
  DAQ->SetConfigFilename(option.config);
  DAQ->SetOutputFilename(option.output);
  DAQ->SetDAQTime(option.daqtime);
  DAQ->SetNEvent(option.daqevent);
  DAQ->SetOutputSplitTime(option.sptime);
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
$Log: stddaq.cc,v $
Revision 1.8  2023/04/04 02:37:18  cupsoft
*** empty log message ***

Revision 1.7  2022/12/21 00:02:44  cupsoft
*** empty log message ***

Revision 1.8  2020/07/13 01:25:14  cupsoft
*** empty log message ***

Revision 1.7  2020/07/10 02:37:32  cupsoft
*** empty log message ***

Revision 1.6  2019/07/24 06:14:57  cupsoft
*** empty log message ***

Revision 1.5  2019/07/03 00:11:03  cupsoft
*** empty log message ***

Revision 1.4  2018/02/21 02:32:12  cupsoft
*** empty log message ***

Revision 1.3  2018/02/19 03:34:07  cupsoft
*** empty log message ***

Revision 1.2  2018/01/09 04:19:56  cupsoft
*** empty log message ***

Revision 1.1  2018/01/08 03:52:27  cupsoft
*** empty log message ***

Revision 1.6  2017/05/31 01:45:37  cupsoft
many changes

Revision 1.5  2017/03/22 09:02:34  cupsoft
*** empty log message ***

Revision 1.4  2017/03/22 08:18:06  cupsoft
*** empty log message ***

Revision 1.3  2017/03/22 06:19:39  cupsoft
*** empty log message ***

Revision 1.2  2017/03/15 12:34:30  cupsoft
*** empty log message ***

Revision 1.1.1.1  2017/02/08 06:40:05  cupsoft
DAQ

Revision 1.3  2016/12/14 04:57:45  cupsoft
*** empty log message ***

Revision 1.2  2016/12/05 09:51:31  cupsoft
*** empty log message ***

Revision 1.1  2016/10/17 06:24:39  cupsoft
*** empty log message ***


**/
