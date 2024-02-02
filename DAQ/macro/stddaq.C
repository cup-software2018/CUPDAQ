R__LOAD_LIBRARY(libusb-1.0)
R__LOAD_LIBRARY(libGui)
R__LOAD_LIBRARY(libTree)
R__LOAD_LIBRARY(libFramework)
R__LOAD_LIBRARY(libRawObjs)
R__LOAD_LIBRARY(libNotice)
R__LOAD_LIBRARY(libDAQConfig)
R__LOAD_LIBRARY(libDAQUtils)
R__LOAD_LIBRARY(libDAQSystem)
R__LOAD_LIBRARY(libOnlHistogramer)
R__LOAD_LIBRARY(libDAQ)

void stddaq()
{
  ADC::TYPE adctype = ADC::FADCS;
  TString configfilename;
  TString outputfilename;
  TString histfilename;
  int presetnevent = 0;
  int presetdaqtime = 0;
  int reporttime = 10;
  int verboselevel = 0;
  int outputsplittime = 60*60;

  bool dohist = false;

  auto * daq = new CupDAQManager();
  daq->SetDAQType(DAQ::STDDAQ);
  daq->SetADCType(adctype);
  daq->SetTriggerMode(TRIGGER::GLOBAL);
  daq->SetConfigFileName(configfilename.Data());
  daq->SetOutputFileName(outputfilename.Data());
  daq->SetDAQTime(presetdaqtime);
  daq->SetNEvent(presetnevent);
  daq->SetOutputSplitTime(outputsplittime);
  daq->SetStatusReportTime(reporttime);
  daq->SetVerboseLevel(verboselevel);
  daq->EnableHistograming(dohist);
  daq->SetHistFileName(histfilename);

  daq->Run();

  delete daq;
}