#pragma once

#include <mutex>

#include "DAQSystem/AbsTCB.hh"
#include "NewNotice/NKTCB.hh"

class CupTCB : public AbsTCB {
public:
  CupTCB() = default;
  ~CupTCB() override = default;

  static CupTCB * Instance();
  static void DeleteInstance();

  int Open() override;
  void Close() override;

  // tcb
  void Reset() override;
  void ResetTimer() override;
  void TriggerStart() override;
  void TriggerStop() override;

  unsigned long ReadBCount(unsigned long mid) override;
  int ReadData(unsigned long mid, unsigned long bcount, unsigned char * data) override;

  void WriteCW(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadCW(unsigned long mid, unsigned long ch) override;
  void WriteGW(unsigned long mid, unsigned long data) override;
  unsigned long ReadGW(unsigned long mid) override;
  void WriteRL(unsigned long mid, unsigned long data) override;
  unsigned long ReadRL(unsigned long mid) override;
  void WriteDRAMON(unsigned long mid, unsigned long data) override;
  unsigned long ReadDRAMON(unsigned long mid) override;
  void WriteDACOFF(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadDACOFF(unsigned long mid, unsigned long ch) override;
  void MeasurePED(unsigned long mid, unsigned long ch) override;
  unsigned long ReadPED(unsigned long mid, unsigned long ch) override;
  void WriteDLY(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadDLY(unsigned long mid, unsigned long ch) override;
  void WriteTHR(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadTHR(unsigned long mid, unsigned long ch) override;
  void WritePOL(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadPOL(unsigned long mid, unsigned long ch) override;
  void WritePSW(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadPSW(unsigned long mid, unsigned long ch) override;
  void WriteAMODE(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadAMODE(unsigned long mid, unsigned long ch) override;
  void WritePCT(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadPCT(unsigned long mid, unsigned long ch) override;
  void WritePCI(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadPCI(unsigned long mid, unsigned long ch) override;
  void WritePWT(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadPWT(unsigned long mid, unsigned long ch) override;
  void WriteDT(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadDT(unsigned long mid, unsigned long ch) override;
  void WritePSS(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadPSS(unsigned long mid, unsigned long ch) override;
  void WriteTM(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadTM(unsigned long mid, unsigned long ch) override;
  void WriteTLT(unsigned long mid, unsigned long data) override;
  unsigned long ReadTLT(unsigned long mid) override;
  void WriteSTLT(unsigned long mid, unsigned long ch, unsigned long data) override;
  unsigned long ReadSTLT(unsigned long mid, unsigned long ch) override;
  void WriteDSR(unsigned long mid, unsigned long data) override;
  unsigned long ReadDSR(unsigned long mid) override;
  void AlignFADC(unsigned long mid) override;
  void AlignSADC(unsigned long mid) override;
  void AlignGADC(unsigned long mid) override;

  // iadc
  void WriteDAQMODE(unsigned long mid, unsigned long data) override;
  unsigned long ReadDAQMODE(unsigned long mid) override;
  void WriteHV(unsigned long mid, unsigned long ch, float data) override;
  float ReadHV(unsigned long mid, unsigned long ch) override;
  float ReadTEMP(unsigned long mid, unsigned long ch) override;
  void AlignIADC(unsigned long mid) override;

  // tcb
  void WritePTRIG(unsigned long data) override;
  unsigned long ReadPTRIG() override;
  void WriteTRIGENABLE(unsigned long mid, unsigned long data) override;
  unsigned long ReadTRIGENABLE(unsigned long mid) override;
  void WriteMTHRFADC(unsigned long data) override;
  unsigned long ReadMTHRFADC() override;
  void WritePSCALEFADC(unsigned long data) override;
  unsigned long ReadPSCALEFADC() override;
  void WriteMTHRSADCMU(unsigned long data) override;
  unsigned long ReadMTHRSADCMU() override;
  void WritePSCALESADCMU(unsigned long data) override;
  unsigned long ReadPSCALESADCMU() override;
  void WriteMTHRSADCLS(unsigned long data) override;
  unsigned long ReadMTHRSADCLS() override;
  void WritePSCALESADCLS(unsigned long data) override;
  unsigned long ReadPSCALESADCLS() override;
  void WriteMTHRIADC(unsigned long data) override;
  unsigned long ReadMTHRIADC() override;
  void WritePSCALEIADC(unsigned long data) override;
  unsigned long ReadPSCALEIADC() override;
  void WriteEXTOUT(unsigned long data) override;
  unsigned long ReadEXTOUT() override;
  void WriteGATEDLY(unsigned long data) override;
  unsigned long ReadGATEDLY() override;
  void WriteGATEWIDTH(unsigned long data) override;
  unsigned long ReadGATEWIDTH() override;
  void WriteEXTOUTWIDTH(unsigned long data) override;
  unsigned long ReadEXTOUTWIDTH() override;
  void SendTRIG() override;
  void ReadLNSTAT(unsigned long * data) override;
  void ReadMIDS(unsigned long * data) override;
  void AlignDRAM(unsigned long mid) override;

  void WriteTRGSWFADC(unsigned long fadc, unsigned long sadcmu, unsigned long sadcls, unsigned long iadc) override;
  void WriteTRGSWSADCMU(unsigned long fadc, unsigned long sadcmu, unsigned long sadcls, unsigned long iadc) override;
  void WriteTRGSWSADCLS(unsigned long fadc, unsigned long sadcmu, unsigned long sadcls, unsigned long iadc) override;
  void WriteTRGSWIADC(unsigned long fadc, unsigned long sadcmu, unsigned long sadcls, unsigned long iadc) override;
  unsigned long ReadTRGSWFADC() override;
  unsigned long ReadTRGSWSADCMU() override;
  unsigned long ReadTRGSWSADCLS() override;
  unsigned long ReadTRGSWIADC() override;

protected:
  static CupTCB * fTCB;

  std::mutex fMutex;
  NKTCB fNKTCB{};

  ClassDef(CupTCB, 0)
};
