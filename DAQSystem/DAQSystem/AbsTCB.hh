#pragma once

#include "TObject.h"

#include "DAQConfig/FADCTConf.hh"
#include "DAQConfig/IADCTConf.hh"
#include "DAQConfig/SADCTConf.hh"
#include "DAQConfig/TCBConf.hh"

class AbsTCB : public TObject {
public:
  AbsTCB() = default;
  ~AbsTCB() override = default;

  virtual int Open() = 0;
  virtual void Close() = 0;

  virtual void Reset() = 0;
  virtual void ResetTimer() = 0;
  virtual void TriggerStart() = 0;
  virtual void TriggerStop() = 0;

  virtual uint32_t ReadBCount(uint32_t mid) = 0;
  virtual int ReadData(uint32_t mid, uint32_t bcount, unsigned char * data) = 0;

  virtual void WriteCW(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadCW(uint32_t mid, uint32_t ch) = 0;
  virtual void WriteGW(uint32_t mid, uint32_t data) = 0;
  virtual uint32_t ReadGW(uint32_t mid) = 0;
  virtual void WriteRL(uint32_t mid, uint32_t data) = 0;
  virtual uint32_t ReadRL(uint32_t mid) = 0;
  virtual void WriteDRAMON(uint32_t mid, uint32_t data) = 0;
  virtual uint32_t ReadDRAMON(uint32_t mid) = 0;
  virtual void WriteDACOFF(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadDACOFF(uint32_t mid, uint32_t ch) = 0;
  virtual void MeasurePED(uint32_t mid, uint32_t ch) = 0;
  virtual uint32_t ReadPED(uint32_t mid, uint32_t ch) = 0;
  virtual void WriteDLY(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadDLY(uint32_t mid, uint32_t ch) = 0;
  virtual void WriteTHR(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadTHR(uint32_t mid, uint32_t ch) = 0;
  virtual void WritePOL(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadPOL(uint32_t mid, uint32_t ch) = 0;
  virtual void WritePSW(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadPSW(uint32_t mid, uint32_t ch) = 0;
  virtual void WriteAMODE(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadAMODE(uint32_t mid, uint32_t ch) = 0;
  virtual void WritePCT(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadPCT(uint32_t mid, uint32_t ch) = 0;
  virtual void WritePCI(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadPCI(uint32_t mid, uint32_t ch) = 0;
  virtual void WritePWT(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadPWT(uint32_t mid, uint32_t ch) = 0;
  virtual void WriteDT(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadDT(uint32_t mid, uint32_t ch) = 0;
  virtual void WritePSS(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadPSS(uint32_t mid, uint32_t ch) = 0;
  virtual void WriteTM(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadTM(uint32_t mid, uint32_t ch) = 0;
  virtual void WriteTLT(uint32_t mid, uint32_t data) = 0;
  virtual uint32_t ReadTLT(uint32_t mid) = 0;
  virtual void WriteSTLT(uint32_t mid, uint32_t ch, uint32_t data) = 0;
  virtual uint32_t ReadSTLT(uint32_t mid, uint32_t ch) = 0;
  virtual void WriteDSR(uint32_t mid, uint32_t data) = 0;
  virtual uint32_t ReadDSR(uint32_t mid) = 0;
  virtual void AlignFADC(uint32_t mid) = 0;
  virtual void AlignSADC(uint32_t mid) = 0;
  virtual void AlignGADC(uint32_t mid) = 0;

  // iadc
  virtual void WriteDAQMODE(uint32_t mid, uint32_t data) = 0;
  virtual uint32_t ReadDAQMODE(uint32_t mid) = 0;
  virtual void WriteHV(uint32_t mid, uint32_t ch, float data) = 0;
  virtual float ReadHV(uint32_t mid, uint32_t ch) = 0;
  virtual float ReadTEMP(uint32_t mid, uint32_t ch) = 0;
  virtual void AlignIADC(uint32_t mid) = 0;

  virtual void WritePTRIG(uint32_t data) = 0;
  virtual uint32_t ReadPTRIG() = 0;
  virtual void WriteTRIGENABLE(uint32_t mid, uint32_t data) = 0;
  virtual uint32_t ReadTRIGENABLE(uint32_t mid) = 0;
  virtual void WriteMTHRFADC(uint32_t data) = 0;
  virtual uint32_t ReadMTHRFADC() = 0;
  virtual void WritePSCALEFADC(uint32_t data) = 0;
  virtual uint32_t ReadPSCALEFADC() = 0;
  virtual void WriteMTHRSADCMU(uint32_t data) = 0;
  virtual uint32_t ReadMTHRSADCMU() = 0;
  virtual void WritePSCALESADCMU(uint32_t data) = 0;
  virtual uint32_t ReadPSCALESADCMU() = 0;
  virtual void WriteMTHRSADCLS(uint32_t data) = 0;
  virtual uint32_t ReadMTHRSADCLS() = 0;
  virtual void WritePSCALESADCLS(uint32_t data) = 0;
  virtual uint32_t ReadPSCALESADCLS() = 0;
  virtual void WriteMTHRIADC(uint32_t data) = 0;
  virtual uint32_t ReadMTHRIADC() = 0;
  virtual void WritePSCALEIADC(uint32_t data) = 0;
  virtual uint32_t ReadPSCALEIADC() = 0;
  virtual void WriteEXTOUT(uint32_t data) = 0;
  virtual uint32_t ReadEXTOUT() = 0;
  virtual void WriteGATEDLY(uint32_t data) = 0;
  virtual uint32_t ReadGATEDLY() = 0;
  virtual void WriteGATEWIDTH(uint32_t data) = 0;
  virtual uint32_t ReadGATEWIDTH() = 0;
  virtual void WriteEXTOUTWIDTH(uint32_t data) = 0;
  virtual uint32_t ReadEXTOUTWIDTH() = 0;
  virtual void SendTRIG() = 0;
  virtual void ReadLNSTAT(uint32_t * data) = 0;
  virtual void ReadMIDS(uint32_t * data) = 0;
  virtual void AlignDRAM(uint32_t mid) = 0;

  virtual void WriteTRGSWFADC(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls, uint32_t iadc) = 0;
  virtual void WriteTRGSWSADCMU(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls, uint32_t iadc) = 0;
  virtual void WriteTRGSWSADCLS(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls, uint32_t iadc) = 0;
  virtual void WriteTRGSWIADC(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls, uint32_t iadc) = 0;
  virtual uint32_t ReadTRGSWFADC() = 0;
  virtual uint32_t ReadTRGSWSADCMU() = 0;
  virtual uint32_t ReadTRGSWSADCLS() = 0;
  virtual uint32_t ReadTRGSWIADC() = 0;

  // write register
  virtual void WriteRegisterTCB(TCBConf * conf);
  virtual void WriteRegisterFADC(FADCTConf * conf);
  virtual void WriteRegisterSADC(SADCTConf * conf);
  virtual void WriteRegisterIADC(IADCTConf * conf);

  // print register
  virtual void PrintRegisterTCB(TCBConf * conf);
  virtual void PrintRegisterFADC(FADCTConf * conf);
  virtual void PrintRegisterSADC(SADCTConf * conf);
  virtual void PrintRegisterIADC(IADCTConf * conf);

  virtual void MeasurePedestalFADC(FADCTConf * conf);
  virtual void MeasurePedestalSADC(SADCTConf * conf);
  virtual void MeasurePedestalIADC(IADCTConf * conf);

protected:
  ClassDef(AbsTCB, 0)
};
