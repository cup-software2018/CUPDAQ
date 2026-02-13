#pragma once

#include "DAQSystem/AbsTCB.hh"
#include "Notice/NKMiniTCB.hh"

class CupMiniTCB : public AbsTCB {
public:
  CupMiniTCB() = default;
  ~CupMiniTCB() override = default;

  void SetIPAddress(const char * ipaddr);

  int Open() override;
  void Close() override;

  // tcb
  void Reset() override;
  void ResetTimer() override;
  void TriggerStart() override;
  void TriggerStop() override;

  uint32_t ReadBCount(uint32_t mid) override { return 0; }
  int ReadData(uint32_t mid, uint32_t bcount, unsigned char * data) override { return 0; }

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
  void AlignFADC(uint32_t mid) override;
  void AlignSADC(uint32_t mid) override;
  void AlignGADC(uint32_t mid) override;

  // iadc
  void WriteDAQMODE(uint32_t mid, uint32_t data) override {}
  uint32_t ReadDAQMODE(uint32_t mid) override { return 0; }
  void WriteHV(uint32_t mid, uint32_t ch, float data) override {}
  float ReadHV(uint32_t mid, uint32_t ch) override { return 0; }
  float ReadTEMP(uint32_t mid, uint32_t ch) override { return 0; }
  void AlignIADC(uint32_t mid) override {}

  // tcb
  void WritePTRIG(uint32_t data) override;
  uint32_t ReadPTRIG() override;
  void WriteTRIGENABLE(uint32_t mid, uint32_t data) override;
  uint32_t ReadTRIGENABLE(uint32_t mid) override;
  void WriteMTHRFADC(uint32_t data) override;
  uint32_t ReadMTHRFADC() override;
  void WritePSCALEFADC(uint32_t data) override;
  uint32_t ReadPSCALEFADC() override;
  void WriteMTHRSADCMU(uint32_t data) override {}
  uint32_t ReadMTHRSADCMU() override { return 0; }
  void WritePSCALESADCMU(uint32_t data) override {}
  uint32_t ReadPSCALESADCMU() override { return 0; }
  void WriteMTHRSADCLS(uint32_t data) override {}
  uint32_t ReadMTHRSADCLS() override { return 0; }
  void WritePSCALESADCLS(uint32_t data) override {}
  uint32_t ReadPSCALESADCLS() override { return 0; }
  void WriteMTHRIADC(uint32_t data) override {}
  uint32_t ReadMTHRIADC() override { return 0; }
  void WritePSCALEIADC(uint32_t data) override {}
  uint32_t ReadPSCALEIADC() override { return 0; }
  void WriteEXTOUT(uint32_t data) override {}
  uint32_t ReadEXTOUT() override { return 0; }
  void WriteGATEDLY(uint32_t data) override {}
  uint32_t ReadGATEDLY() override { return 0; }
  void WriteGATEWIDTH(uint32_t data) override {}
  uint32_t ReadGATEWIDTH() override { return 0; }
  void WriteEXTOUTWIDTH(uint32_t data) override {}
  uint32_t ReadEXTOUTWIDTH() override { return 0; }
  void SendTRIG() override;
  void ReadLNSTAT(uint32_t * data) override;
  void ReadMIDS(uint32_t * data) override;
  void AlignDRAM(uint32_t mid) override;

  void WriteTRGSWFADC(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls, uint32_t iadc) override {}
  void WriteTRGSWSADCMU(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls, uint32_t iadc) override {}
  void WriteTRGSWSADCLS(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls, uint32_t iadc) override {}
  void WriteTRGSWIADC(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls, uint32_t iadc) override {}
  uint32_t ReadTRGSWFADC() override { return 0; }
  uint32_t ReadTRGSWSADCMU() override { return 0; }
  uint32_t ReadTRGSWSADCLS() override { return 0; }
  uint32_t ReadTRGSWIADC() override { return 0; }

protected:
  std::string fIPAddress{};
  NKMiniTCB fTCB{};

  ClassDef(CupMiniTCB, 0)
};

inline void CupMiniTCB::SetIPAddress(const char * ipaddr) { fIPAddress = ipaddr; }
