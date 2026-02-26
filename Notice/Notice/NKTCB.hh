#pragma once

#include <cstdint>
#include <vector>

#include "Notice/USB3Tcb.hh"

// Hardware Constants
namespace NKTCBConst {
// Trigger Mode Bitmasks
constexpr uint32_t TM_COUNT = 0x1;
constexpr uint32_t TM_WIDTH = 0x2;
constexpr uint32_t TM_PEAKSUM = 0x4;
constexpr uint32_t TM_ORPEAKSUM = 0x8;
} // namespace NKTCBConst

class NKTCB {
public:
  NKTCB();
  ~NKTCB();

  int Open();
  void Close();

  // Load Lookup Table (Bulk Write)
  void WriteLT(uint32_t mid, const char * data, int len);

  // -----------------------------------------------------------------------
  // Control & Status Registers
  // -----------------------------------------------------------------------
  void Reset();
  void ResetTimer();
  void Start();
  void Stop();
  uint32_t ReadRUN(uint32_t mid) const;

  void WriteCW(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadCW(uint32_t mid, uint32_t ch) const;

  void WriteRL(uint32_t mid, uint32_t data) const;
  uint32_t ReadRL(uint32_t mid) const;

  void WriteGW(uint32_t mid, uint32_t data) const;
  uint32_t ReadGW(uint32_t mid) const;

  void WriteDRAMON(uint32_t mid, uint32_t data) const;
  uint32_t ReadDRAMON(uint32_t mid) const;

  void WriteDACOFF(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadDACOFF(uint32_t mid, uint32_t ch) const;

  void MeasurePED(uint32_t mid, uint32_t ch) const;
  uint32_t ReadPED(uint32_t mid, uint32_t ch) const;

  // Delays & Thresholds
  void WriteDLY(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadDLY(uint32_t mid, uint32_t ch) const;

  void WriteAMOREDLY(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadAMOREDLY(uint32_t mid, uint32_t ch) const;

  void WriteTHR(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadTHR(uint32_t mid, uint32_t ch) const;

  void WritePOL(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadPOL(uint32_t mid, uint32_t ch) const;

  void WritePSW(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadPSW(uint32_t mid, uint32_t ch) const;

  void WriteAMODE(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadAMODE(uint32_t mid, uint32_t ch) const;

  // Pulse & Trigger Configuration
  void WritePCT(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadPCT(uint32_t mid, uint32_t ch) const;

  void WritePCI(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadPCI(uint32_t mid, uint32_t ch) const;

  void WritePWT(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadPWT(uint32_t mid, uint32_t ch) const;

  void WritePSS(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadPSS(uint32_t mid, uint32_t ch) const;

  void WriteRT(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadRT(uint32_t mid, uint32_t ch) const;

  void WriteSR(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadSR(uint32_t mid, uint32_t ch) const;

  void WriteDACGain(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadDACGain(uint32_t mid, uint32_t ch) const;

  void WriteTM(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadTM(uint32_t mid, uint32_t ch) const;

  void WriteTLT(uint32_t mid, uint32_t data) const;
  uint32_t ReadTLT(uint32_t mid) const;

  void WriteSTLT(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadSTLT(uint32_t mid, uint32_t ch) const;

  void WriteZEROSUP(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadZEROSUP(uint32_t mid, uint32_t ch) const;

  // ADC Controls
  void SendADCRST(uint32_t mid) const;
  void SendADCCAL(uint32_t mid) const;

  void WriteADCDLY(uint32_t mid, uint32_t ch, uint32_t data) const;
  void WriteADCALIGN(uint32_t mid, uint32_t data) const;
  uint32_t ReadADCSTAT(uint32_t mid) const;

  void WriteBITSLIP(uint32_t mid, uint32_t ch, uint32_t data) const;

  void WriteFMUX(uint32_t mid, uint32_t ch) const;
  uint32_t ReadFMUX(uint32_t mid) const;

  void ArmFADC(uint32_t mid) const;
  uint32_t ReadFREADY(uint32_t mid) const;

  void WriteZSFD(uint32_t mid, uint32_t data) const;
  uint32_t ReadZSFD(uint32_t mid) const;

  void WriteDSR(uint32_t mid, uint32_t data) const;
  uint32_t ReadDSR(uint32_t mid) const;

  void WriteST(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadST(uint32_t mid, uint32_t ch) const;

  void WritePT(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadPT(uint32_t mid, uint32_t ch) const;

  // DRAM Controls
  void WriteDRAMDLY(uint32_t mid, uint32_t ch, uint32_t data) const;
  void WriteDRAMBITSLIP(uint32_t mid, uint32_t ch) const;
  void WriteDRAMTEST(uint32_t mid, uint32_t data) const;
  uint32_t ReadDRAMTEST(uint32_t mid, uint32_t ch) const;

  void WriteDAQMODE(uint32_t mid, uint32_t data) const;
  uint32_t ReadDAQMODE(uint32_t mid) const;

  // Sensors & Environment
  void WriteHV(uint32_t mid, uint32_t ch, float data) const;
  float ReadHV(uint32_t mid, uint32_t ch) const;
  float ReadTEMP(uint32_t mid, uint32_t ch) const;

  void WriteADCMUX(uint32_t mid, uint32_t ch, uint32_t data) const;
  void ReadFADCBUF(uint32_t mid, uint32_t * data) const;

  // Calibration & Alignment (Complex Logic)
  void AlignFADC500(uint32_t mid) const;
  void AlignSADC64(uint32_t mid) const;
  void AlignFADC125(uint32_t mid) const;
  void AlignIADC64(uint32_t mid) const;
  void AlignDRAM(uint32_t mid) const;

  // Global Trigger Controls
  void WriteRUNNO(uint32_t data) const;
  uint32_t ReadRUNNO() const;

  void WriteGATEDLY(uint32_t data) const;
  uint32_t ReadGATEDLY() const;

  void SendTRIG() const;

  void ReadLNSTAT(uint32_t * data) const;
  void ReadMIDS(uint32_t * data) const;

  void WritePTRIG(uint32_t data) const;
  uint32_t ReadPTRIG() const;

  void WriteTRIGENABLE(uint32_t mid, uint32_t data) const;
  uint32_t ReadTRIGENABLE(uint32_t mid) const;

  void WriteEXTOUT(uint32_t data) const;
  uint32_t ReadEXTOUT() const;

  void WriteGATEWIDTH(uint32_t data) const;
  uint32_t ReadGATEWIDTH() const;

  void WriteEXTOUTWIDTH(uint32_t data) const;
  uint32_t ReadEXTOUTWIDTH() const;

  // Data Readout
  uint32_t ReadBCount(uint32_t mid) const;
  int ReadData(uint32_t mid, uint32_t bcount, unsigned char * data) const;

  void WriteGAIN(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadGAIN(uint32_t mid, uint32_t ch) const;

  void WriteMTHR(uint32_t mid, uint32_t data) const;
  uint32_t ReadMTHR(uint32_t mid) const;

  void WritePSDDLY(uint32_t mid, uint32_t data) const;
  uint32_t ReadPSDDLY(uint32_t mid) const;

  void WritePSDTHR(uint32_t mid, float data) const;
  float ReadPSDTHR(uint32_t mid) const;

  uint32_t ReadADCSTATWORD(uint32_t mid) const;

  // Specific ADC Controls
  void WriteMTHRFADC500(uint32_t data) const;
  uint32_t ReadMTHRFADC500() const;
  void WritePSCALEFADC500(uint32_t data) const;
  uint32_t ReadPSCALEFADC500() const;

  void WriteMTHRSADC64MU(uint32_t data) const;
  uint32_t ReadMTHRSADC64MU() const;
  void WritePSCALESADC64MU(uint32_t data) const;
  uint32_t ReadPSCALESADC64MU() const;

  void WriteMTHRSADC64LS(uint32_t data) const;
  uint32_t ReadMTHRSADC64LS() const;
  void WritePSCALESADC64LS(uint32_t data) const;
  uint32_t ReadPSCALESADC64LS() const;

  void WriteMTHRIADC64(uint32_t data) const;
  uint32_t ReadMTHRIADC64() const;
  void WritePSCALEIADC64(uint32_t data) const;
  uint32_t ReadPSCALEIADC64() const;

  // Trigger Switches
  void WriteTrigSwitchFADC500(uint32_t fadc, uint32_t sadc_mu, uint32_t sadc_ls,
                              uint32_t iadc) const;
  uint32_t ReadSwitchFADC500() const;

  void WriteTrigSwitchSADC64MU(uint32_t fadc, uint32_t sadc_mu, uint32_t sadc_ls,
                               uint32_t iadc) const;
  uint32_t ReadSwitchSADC64MU() const;

  void WriteTrigSwitchSADC64LS(uint32_t fadc, uint32_t sadc_mu, uint32_t sadc_ls,
                               uint32_t iadc) const;
  uint32_t ReadSwitchSADC64LS() const;

  void WriteTrigSwitchIADC64(uint32_t fadc, uint32_t sadc_mu, uint32_t sadc_ls,
                             uint32_t iadc) const;
  uint32_t ReadSwitchIADC64() const;

  uint32_t ReadDBG() const;

  void WriteDT(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadDT(uint32_t mid, uint32_t ch) const;

private:
  USB3Tcb _usb;

  // Helper for address calculation
  uint32_t GetAddr(uint32_t baseAddr, uint32_t mid, uint32_t ch) const;
};