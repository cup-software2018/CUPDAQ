#pragma once

#include <mutex>

#include "DAQSystem/AbsTCB.hh"
#include "Notice/NKTCB.hh"

class CupTCB : public AbsTCB {
public:
  // Deleted copy constructor and assignment operator for Singleton
  CupTCB(const CupTCB &) = delete;
  CupTCB & operator=(const CupTCB &) = delete;

  // Singleton Access
  static CupTCB * Instance();

  // Legacy cleanup (No-op in modern singleton, kept for API compatibility)
  static void DeleteInstance();

  int Open() override;
  void Close() override;

  // -----------------------------------------------------------------------
  // Core TCB Control
  // -----------------------------------------------------------------------
  void Reset() override;
  void ResetTimer() override;
  void TriggerStart() override;
  void TriggerStop() override;

  // -----------------------------------------------------------------------
  // Data Readout
  // -----------------------------------------------------------------------
  uint32_t ReadBCount(uint32_t mid) override;
  int ReadData(uint32_t mid, uint32_t bcount, unsigned char * data) override;

  // -----------------------------------------------------------------------
  // Register Access (Read/Write)
  // -----------------------------------------------------------------------
  void WriteCW(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadCW(uint32_t mid, uint32_t ch) override;

  void WriteGW(uint32_t mid, uint32_t data) override;
  uint32_t ReadGW(uint32_t mid) override;

  void WriteRL(uint32_t mid, uint32_t data) override;
  uint32_t ReadRL(uint32_t mid) override;

  void WriteDRAMON(uint32_t mid, uint32_t data) override;
  uint32_t ReadDRAMON(uint32_t mid) override;

  void WriteDACOFF(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadDACOFF(uint32_t mid, uint32_t ch) override;

  void MeasurePED(uint32_t mid, uint32_t ch) override;
  uint32_t ReadPED(uint32_t mid, uint32_t ch) override;

  void WriteDLY(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadDLY(uint32_t mid, uint32_t ch) override;

  void WriteTHR(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadTHR(uint32_t mid, uint32_t ch) override;

  void WritePOL(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadPOL(uint32_t mid, uint32_t ch) override;

  void WritePSW(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadPSW(uint32_t mid, uint32_t ch) override;

  void WriteAMODE(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadAMODE(uint32_t mid, uint32_t ch) override;

  void WritePCT(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadPCT(uint32_t mid, uint32_t ch) override;

  void WritePCI(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadPCI(uint32_t mid, uint32_t ch) override;

  void WritePWT(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadPWT(uint32_t mid, uint32_t ch) override;

  void WriteDT(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadDT(uint32_t mid, uint32_t ch) override;

  void WritePSS(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadPSS(uint32_t mid, uint32_t ch) override;

  void WriteTM(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadTM(uint32_t mid, uint32_t ch) override;

  void WriteTLT(uint32_t mid, uint32_t data) override;
  uint32_t ReadTLT(uint32_t mid) override;

  void WriteSTLT(uint32_t mid, uint32_t ch, uint32_t data) override;
  uint32_t ReadSTLT(uint32_t mid, uint32_t ch) override;

  void WriteDSR(uint32_t mid, uint32_t data) override;
  uint32_t ReadDSR(uint32_t mid) override;

  // -----------------------------------------------------------------------
  // Alignment Functions
  // -----------------------------------------------------------------------
  void AlignFADC(uint32_t mid) override;
  void AlignSADC(uint32_t mid) override;
  void AlignGADC(uint32_t mid) override; // FADC125
  void AlignIADC(uint32_t mid) override;
  void AlignDRAM(uint32_t mid) override;

  // -----------------------------------------------------------------------
  // IADC Specific
  // -----------------------------------------------------------------------
  void WriteDAQMODE(uint32_t mid, uint32_t data) override;
  uint32_t ReadDAQMODE(uint32_t mid) override;

  void WriteHV(uint32_t mid, uint32_t ch, float data) override;
  float ReadHV(uint32_t mid, uint32_t ch) override;
  float ReadTEMP(uint32_t mid, uint32_t ch) override;

  // -----------------------------------------------------------------------
  // Trigger Configuration
  // -----------------------------------------------------------------------
  void WritePTRIG(uint32_t data) override;
  uint32_t ReadPTRIG() override;

  void WriteTRIGENABLE(uint32_t mid, uint32_t data) override;
  uint32_t ReadTRIGENABLE(uint32_t mid) override;

  void WriteMTHRFADC(uint32_t data) override;
  uint32_t ReadMTHRFADC() override;

  void WritePSCALEFADC(uint32_t data) override;
  uint32_t ReadPSCALEFADC() override;

  void WriteMTHRSADCMU(uint32_t data) override;
  uint32_t ReadMTHRSADCMU() override;

  void WritePSCALESADCMU(uint32_t data) override;
  uint32_t ReadPSCALESADCMU() override;

  void WriteMTHRSADCLS(uint32_t data) override;
  uint32_t ReadMTHRSADCLS() override;

  void WritePSCALESADCLS(uint32_t data) override;
  uint32_t ReadPSCALESADCLS() override;

  void WriteMTHRIADC(uint32_t data) override;
  uint32_t ReadMTHRIADC() override;

  void WritePSCALEIADC(uint32_t data) override;
  uint32_t ReadPSCALEIADC() override;

  void WriteEXTOUT(uint32_t data) override;
  uint32_t ReadEXTOUT() override;

  void WriteGATEDLY(uint32_t data) override;
  uint32_t ReadGATEDLY() override;

  void WriteGATEWIDTH(uint32_t data) override;
  uint32_t ReadGATEWIDTH() override;

  void WriteEXTOUTWIDTH(uint32_t data) override;
  uint32_t ReadEXTOUTWIDTH() override;

  void SendTRIG() override;

  void ReadLNSTAT(uint32_t * data) override;
  void ReadMIDS(uint32_t * data) override;

  // -----------------------------------------------------------------------
  // Trigger Switches
  // -----------------------------------------------------------------------
  void WriteTRGSWFADC(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls,
                      uint32_t iadc) override;
  void WriteTRGSWSADCMU(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls,
                        uint32_t iadc) override;
  void WriteTRGSWSADCLS(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls,
                        uint32_t iadc) override;
  void WriteTRGSWIADC(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls,
                      uint32_t iadc) override;

  uint32_t ReadTRGSWFADC() override;
  uint32_t ReadTRGSWSADCMU() override;
  uint32_t ReadTRGSWSADCLS() override;
  uint32_t ReadTRGSWIADC() override;

private:
  // Private Constructor for Singleton
  CupTCB() = default;
  ~CupTCB() override = default;

  std::mutex fMutex;
  NKTCB fNKTCB{};

  ClassDef(CupTCB, 0)
};