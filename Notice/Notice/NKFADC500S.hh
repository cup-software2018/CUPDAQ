#pragma once

#include <cstdint>

#include "Notice/USB3Com.hh"

class NKFADC500S {
public:
  NKFADC500S() = default;
  explicit NKFADC500S(int sid);
  ~NKFADC500S();

  void SetSID(int sid);

  int Open();
  void Close();

  // Control & Status
  void Reset() const;
  void ResetTimer() const;
  void Start() const;
  void Stop() const;

  uint32_t ReadRun() const;

  // Register Access
  void WriteCW(uint32_t ch, uint32_t data) const;
  uint32_t ReadCW(uint32_t ch) const;

  void WriteRL(uint32_t data) const;
  uint32_t ReadRL() const;

  void WriteDRAMON(uint32_t data) const;
  uint32_t ReadDRAMON() const;

  void WriteDACOFF(uint32_t ch, uint32_t data) const;
  uint32_t ReadDACOFF(uint32_t ch) const;

  void MeasurePED(uint32_t ch) const;
  uint32_t ReadPED(uint32_t ch) const;

  void WriteDLY(uint32_t ch, uint32_t data) const;
  uint32_t ReadDLY(uint32_t ch) const;

  void WriteTHR(uint32_t ch, uint32_t data) const;
  uint32_t ReadTHR(uint32_t ch) const;

  void WritePOL(uint32_t ch, uint32_t data) const;
  uint32_t ReadPOL(uint32_t ch) const;

  void WritePSW(uint32_t ch, uint32_t data) const;
  uint32_t ReadPSW(uint32_t ch) const;

  void WriteAMODE(uint32_t ch, uint32_t data) const;
  uint32_t ReadAMODE(uint32_t ch) const;

  void WritePCT(uint32_t ch, uint32_t data) const;
  uint32_t ReadPCT(uint32_t ch) const;

  void WritePCI(uint32_t ch, uint32_t data) const;
  uint32_t ReadPCI(uint32_t ch) const;

  void WritePWT(uint32_t ch, uint32_t data) const;
  uint32_t ReadPWT(uint32_t ch) const;

  void WriteDT(uint32_t ch, uint32_t data) const;
  uint32_t ReadDT(uint32_t ch) const;

  void WritePTRIG(uint32_t data) const;
  uint32_t ReadPTRIG() const;

  void SendTRIG() const;

  void WriteTRIGENABLE(uint32_t data) const;
  uint32_t ReadTRIGENABLE() const;

  void WriteTM(uint32_t ch, uint32_t data) const;
  uint32_t ReadTM(uint32_t ch) const;

  void WriteTLT(uint32_t data) const;
  uint32_t ReadTLT() const;

  void WriteZEROSUP(uint32_t ch, uint32_t data) const;
  uint32_t ReadZEROSUP(uint32_t ch) const;

  // ADC Specifics
  void SendADCRST() const;
  void SendADCCAL() const;

  void WriteADCDLY(uint32_t ch, uint32_t data) const;
  void WriteADCALIGN(uint32_t data) const;
  uint32_t ReadADCSTAT() const;

  void WriteDRAMDLY(uint32_t ch, uint32_t data) const;
  void WriteDRAMBITSLIP(uint32_t ch) const;

  void WriteDRAMTEST(uint32_t data) const;
  uint32_t ReadDRAMTEST(uint32_t ch) const;

  void WritePSCALE(uint32_t data) const;
  uint32_t ReadPSCALE() const;

  void WriteDSR(uint32_t data) const;
  uint32_t ReadDSR() const;

  // Data Readout
  int ReadBCount() const;
  int ReadData(int bcount, unsigned char * data, unsigned int timeout = 0) const;
  void FlushData() const;

  // Calibration
  void AlignADC() const;
  void AlignDRAM() const;

  int Sid() const;

private:
  int _sid{0};
  USB3Com _usb{};
};