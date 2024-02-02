/*
 *
 *  Module:  tcb
 *
 *  Author:  Jaison Lee
 *
 *  Purpose: tcb program (rc version)
 *
 *  Last Update:      $Author: cupsoft $
 *  Update Date:      $Date: 2023/02/09 07:24:04 $
 *  CVS/RCS Revision: $Revision: 1.13 $
 *  Status:           $State: Exp $
 *
 */
#include "DAQ/CupDAQManager.hh"
#include "DAQ/daqopt.hh"

int main(int argc, char ** argv)
{
  if (argc < 2) return 0;

  daqopt option;
  option.init();
  optparse(option, argc, argv);

  // for TCB controlled ADC
  auto * DAQ = new CupDAQManager();
  DAQ->SetDAQType(DAQ::TCB);
  DAQ->SetRunNumber(option.runnum);
  DAQ->SetConfigFilename(option.config);
  DAQ->SetOutputSplitTime(option.sptime);
  DAQ->Run();

  delete DAQ;
  return 0;
}

/**
$Log: tcb.cc,v $
Revision 1.13  2023/02/09 07:24:04  cupsoft
*** empty log message ***

Revision 1.12  2022/12/21 00:02:44  cupsoft
*** empty log message ***

Revision 1.12  2020/07/10 02:37:32  cupsoft
*** empty log message ***

Revision 1.11  2020/01/09 04:56:56  cupsoft
*** empty log message ***

Revision 1.10  2019/11/27 23:58:35  cupsoft
*** empty log message ***

Revision 1.9  2019/07/24 06:14:57  cupsoft
*** empty log message ***

Revision 1.8  2019/07/05 00:25:04  cupsoft
*** empty log message ***

**/