#ifndef AbsTCB_HH
#define AbsTCB_HH

#include "TObject.h"

#include "DAQConfig/FADCTConf.hh"
#include "DAQConfig/GADCTConf.hh"
#include "DAQConfig/IADCTConf.hh"
#include "DAQConfig/SADCTConf.hh"
#include "DAQConfig/TCBConf.hh"
#include "DAQUtils/ELogger.hh"

class AbsTCB : public TObject {
public:
  AbsTCB();
  virtual ~AbsTCB();

  virtual int Open() = 0;
  virtual void Close() = 0;

  virtual void Reset() = 0;
  virtual void ResetTIMER() = 0;
  virtual void TriggerStart() = 0;
  virtual void TriggerStop() = 0;

  virtual unsigned long ReadBCOUNT(unsigned long mid) = 0;
  virtual int ReadDATA(unsigned long mid, unsigned long bcount,
                       unsigned char * data) = 0;

  virtual void WriteCW(unsigned long mid, unsigned long ch,
                       unsigned long data) = 0;
  virtual unsigned long ReadCW(unsigned long mid, unsigned long ch) = 0;
  virtual void WriteGW(unsigned long mid, unsigned long data) = 0;
  virtual unsigned long ReadGW(unsigned long mid) = 0;
  virtual void WriteRL(unsigned long mid, unsigned long data) = 0;
  virtual unsigned long ReadRL(unsigned long mid) = 0;
  virtual void WriteDRAMON(unsigned long mid, unsigned long data) = 0;
  virtual unsigned long ReadDRAMON(unsigned long mid) = 0;
  virtual void WriteDACOFF(unsigned long mid, unsigned long ch,
                           unsigned long data) = 0;
  virtual unsigned long ReadDACOFF(unsigned long mid, unsigned long ch) = 0;
  virtual void MeasurePED(unsigned long mid, unsigned long ch) = 0;
  virtual unsigned long ReadPED(unsigned long mid, unsigned long ch) = 0;
  virtual void WriteDLY(unsigned long mid, unsigned long ch,
                        unsigned long data) = 0;
  virtual unsigned long ReadDLY(unsigned long mid, unsigned long ch) = 0;
  virtual void WriteTHR(unsigned long mid, unsigned long ch,
                        unsigned long data) = 0;
  virtual unsigned long ReadTHR(unsigned long mid, unsigned long ch) = 0;
  virtual void WritePOL(unsigned long mid, unsigned long ch,
                        unsigned long data) = 0;
  virtual unsigned long ReadPOL(unsigned long mid, unsigned long ch) = 0;
  virtual void WritePSW(unsigned long mid, unsigned long ch,
                        unsigned long data) = 0;
  virtual unsigned long ReadPSW(unsigned long mid, unsigned long ch) = 0;
  virtual void WriteAMODE(unsigned long mid, unsigned long ch,
                          unsigned long data) = 0;
  virtual unsigned long ReadAMODE(unsigned long mid, unsigned long ch) = 0;
  virtual void WritePCT(unsigned long mid, unsigned long ch,
                        unsigned long data) = 0;
  virtual unsigned long ReadPCT(unsigned long mid, unsigned long ch) = 0;
  virtual void WritePCI(unsigned long mid, unsigned long ch,
                        unsigned long data) = 0;
  virtual unsigned long ReadPCI(unsigned long mid, unsigned long ch) = 0;
  virtual void WritePWT(unsigned long mid, unsigned long ch,
                        unsigned long data) = 0;
  virtual unsigned long ReadPWT(unsigned long mid, unsigned long ch) = 0;
  virtual void WriteDT(unsigned long mid, unsigned long ch,
                       unsigned long data) = 0;
  virtual unsigned long ReadDT(unsigned long mid, unsigned long ch) = 0;
  virtual void WritePSS(unsigned long mid, unsigned long ch,
                        unsigned long data) = 0;
  virtual unsigned long ReadPSS(unsigned long mid, unsigned long ch) = 0;
  virtual void WriteTM(unsigned long mid, unsigned long ch,
                       unsigned long data) = 0;
  virtual unsigned long ReadTM(unsigned long mid, unsigned long ch) = 0;
  virtual void WriteTLT(unsigned long mid, unsigned long data) = 0;
  virtual unsigned long ReadTLT(unsigned long mid) = 0;
  virtual void WriteSTLT(unsigned long mid, unsigned long ch,
                         unsigned long data) = 0;
  virtual unsigned long ReadSTLT(unsigned long mid, unsigned long ch) = 0;
  virtual void WriteDSR(unsigned long mid, unsigned long data) = 0;
  virtual unsigned long ReadDSR(unsigned long mid) = 0;
  virtual void AlignFADC(unsigned long mid) = 0;
  virtual void AlignSADC(unsigned long mid) = 0;
  virtual void AlignGADC(unsigned long mid) = 0;

  // iadc
  virtual void WriteDAQMODE(unsigned long mid, unsigned long data) = 0;
  virtual unsigned long ReadDAQMODE(unsigned long mid) = 0;
  virtual void WriteHV(unsigned long mid, unsigned long ch, float data) = 0;
  virtual float ReadHV(unsigned long mid, unsigned long ch) = 0;
  virtual float ReadTEMP(unsigned long mid, unsigned long ch) = 0;
  virtual void AlignIADC(unsigned long mid) = 0;

  virtual void WritePTRIG(unsigned long data) = 0;
  virtual unsigned long ReadPTRIG() = 0;
  virtual void WriteTRIGENABLE(unsigned long mid, unsigned long data) = 0;
  virtual unsigned long ReadTRIGENABLE(unsigned long mid) = 0;
  virtual void WriteMTHRFADC(unsigned long data) = 0;
  virtual unsigned long ReadMTHRFADC() = 0;
  virtual void WritePSCALEFADC(unsigned long data) = 0;
  virtual unsigned long ReadPSCALEFADC() = 0;
  virtual void WriteMTHRSADCMU(unsigned long data) = 0;
  virtual unsigned long ReadMTHRSADCMU() = 0;
  virtual void WritePSCALESADCMU(unsigned long data) = 0;
  virtual unsigned long ReadPSCALESADCMU() = 0;
  virtual void WriteMTHRSADCLS(unsigned long data) = 0;
  virtual unsigned long ReadMTHRSADCLS() = 0;
  virtual void WritePSCALESADCLS(unsigned long data) = 0;
  virtual unsigned long ReadPSCALESADCLS() = 0;
  virtual void WriteMTHRIADC(unsigned long data) = 0;
  virtual unsigned long ReadMTHRIADC() = 0;
  virtual void WritePSCALEIADC(unsigned long data) = 0;
  virtual unsigned long ReadPSCALEIADC() = 0;
  virtual void WriteEXTOUT(unsigned long data) = 0;
  virtual unsigned long ReadEXTOUT() = 0;
  virtual void WriteGATEDLY(unsigned long data) = 0;
  virtual unsigned long ReadGATEDLY() = 0;
  virtual void WriteGATEWIDTH(unsigned long data) = 0;
  virtual unsigned long ReadGATEWIDTH() = 0;
  virtual void WriteEXTOUTWIDTH(unsigned long data) = 0;
  virtual unsigned long ReadEXTOUTWIDTH() = 0;
  virtual void SendTRIG() = 0;
  virtual void ReadLNSTAT(unsigned long * data) = 0;
  virtual void ReadMIDS(unsigned long * data) = 0;
  virtual void AlignDRAM(unsigned long mid) = 0;

  virtual void WriteTRGSWFADC(unsigned long fadc, unsigned long sadcmu,
                              unsigned long sadcls, unsigned long iadc) = 0;
  virtual void WriteTRGSWSADCMU(unsigned long fadc, unsigned long sadcmu,
                              unsigned long sadcls, unsigned long iadc) = 0;
  virtual void WriteTRGSWSADCLS(unsigned long fadc, unsigned long sadcmu,
                              unsigned long sadcls, unsigned long iadc) = 0;
  virtual void WriteTRGSWIADC(unsigned long fadc, unsigned long sadcmu,
                              unsigned long sadcls, unsigned long iadc) = 0;
  virtual unsigned long ReadTRGSWFADC() = 0;
  virtual unsigned long ReadTRGSWSADCMU() = 0;
  virtual unsigned long ReadTRGSWSADCLS() = 0;
  virtual unsigned long ReadTRGSWIADC() = 0;

  // write register
  virtual void WriteRegisterTCB(TCBConf * conf);
  virtual void WriteRegisterFADC(FADCTConf * conf);
  virtual void WriteRegisterGADC(GADCTConf * conf);
  virtual void WriteRegisterSADC(SADCTConf * conf);
  virtual void WriteRegisterIADC(IADCTConf * conf);

  // print register
  virtual void PrintRegisterTCB(TCBConf * conf);
  virtual void PrintRegisterFADC(FADCTConf * conf);
  virtual void PrintRegisterGADC(GADCTConf * conf);
  virtual void PrintRegisterSADC(SADCTConf * conf);
  virtual void PrintRegisterIADC(IADCTConf * conf);

  virtual void MeasurePedestalFADC(FADCTConf * conf);
  virtual void MeasurePedestalGADC(GADCTConf * conf);
  virtual void MeasurePedestalSADC(SADCTConf * conf);
  virtual void MeasurePedestalIADC(IADCTConf * conf);

protected:
  ELogger * fLog;

  ClassDef(AbsTCB, 0)
};

#endif
