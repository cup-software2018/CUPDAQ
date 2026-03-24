#pragma once

#include <cstdint>
#include <string>

class NKMiniTCB {
public:
  NKMiniTCB() = default;
  ~NKMiniTCB();

  void SetIPAddress(const std::string & ip);

  int Open();
  void Close();

  // Control
  void Reset();
  void ResetTimer();
  void Start();
  void Stop();

  // Low-level Access
  void Write(uint32_t address, uint32_t data) const;
  uint32_t Read(uint32_t address) const;

  void WriteModule(uint32_t mid, uint32_t address, uint32_t data) const;
  uint32_t ReadModule(uint32_t mid, uint32_t address) const;

  // High-level Registers
  uint32_t ReadRUN(uint32_t mid) const;

  void WriteCW(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadCW(uint32_t mid, uint32_t ch) const;

  void WriteGW(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadGW(uint32_t mid, uint32_t ch) const;

  void WriteRL(uint32_t mid, uint32_t data) const;
  uint32_t ReadRL(uint32_t mid) const;

  void WriteDRAMON(uint32_t mid, uint32_t data) const;
  uint32_t ReadDRAMON(uint32_t mid) const;

  void WriteDACOFF(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadDACOFF(uint32_t mid, uint32_t ch) const;

  void MeasurePED(uint32_t mid, uint32_t ch) const;
  uint32_t ReadPED(uint32_t mid, uint32_t ch) const;

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

  void WritePCT(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadPCT(uint32_t mid, uint32_t ch) const;

  void WritePCI(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadPCI(uint32_t mid, uint32_t ch) const;

  void WritePWT(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadPWT(uint32_t mid, uint32_t ch) const;

  void WriteDT(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadDT(uint32_t mid, uint32_t ch) const;

  void WriteTM(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadTM(uint32_t mid, uint32_t ch) const;

  void WriteTLT(uint32_t mid, uint32_t data) const;
  uint32_t ReadTLT(uint32_t mid) const;

  void WriteSTLT(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadSTLT(uint32_t mid, uint32_t ch) const;

  void WriteZEROSUP(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadZEROSUP(uint32_t mid, uint32_t ch) const;

  // ADC Specific
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

  void ReadFADCBUF(uint32_t mid, uint32_t * data) const;

  // Alignment
  void AlignFADC500(uint32_t mid) const;
  void AlignSADC64(uint32_t mid) const;
  void AlignDRAM(uint32_t mid) const;

  // ETC
  void WritePSS(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadPSS(uint32_t mid, uint32_t ch) const;

  void WriteRT(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadRT(uint32_t mid, uint32_t ch) const;

  void WriteSR(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadSR(uint32_t mid, uint32_t ch) const;

  void WriteDACGAIN(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadDACGAIN(uint32_t mid, uint32_t ch) const;

  void WriteST(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadST(uint32_t mid, uint32_t ch) const;

  void WritePT(uint32_t mid, uint32_t ch, uint32_t data) const;
  uint32_t ReadPT(uint32_t mid, uint32_t ch) const;

  void WriteRUNNO(uint32_t data) const;
  uint32_t ReadRUNNO() const;

  void SendTRIG() const;

  uint32_t ReadLNSTAT() const;
  uint32_t ReadMIDS(uint32_t ch) const;

  void WritePTRIG(uint32_t data) const;
  uint32_t ReadPTRIG() const;

  void WriteTRIGENABLE(uint32_t mid, uint32_t data) const;
  uint32_t ReadTRIGENABLE(uint32_t mid) const;

  void WriteMTHR(uint32_t data) const;
  uint32_t ReadMTHR() const;

  void WritePSCALE(uint32_t data) const;
  uint32_t ReadPSCALE() const;

  void WriteDRAMDLY(uint32_t mid, uint32_t ch, uint32_t data) const;
  void WriteDRAMBITSLIP(uint32_t mid, uint32_t ch) const;
  void WriteDRAMTEST(uint32_t mid, uint32_t data) const;
  uint32_t ReadDRAMTEST(uint32_t mid, uint32_t ch) const;

private:
  int Transmit(const char * buf, int len) const;
  int Receive(char * buf, int len) const;

  int _tcpHandle{-1};
  std::string _ipaddr{};
};

inline void NKMiniTCB::SetIPAddress(const std::string & ip) { _ipaddr = ip; }