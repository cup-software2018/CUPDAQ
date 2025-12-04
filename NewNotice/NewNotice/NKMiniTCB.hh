#pragma once

#include <cstdint>
#include <string>

class NKMiniTCB {
public:
  NKMiniTCB() = default;
  ~NKMiniTCB();

  void SetIPAddress(const char * ip);

  int Open();
  void Close();

  void Reset();
  void ResetTimer();
  void Start();
  void Stop();

  void Write(unsigned long address, unsigned long data) const;
  unsigned long Read(unsigned long address) const;

  void WriteModule(unsigned long mid, unsigned long address, unsigned long data) const;
  unsigned long ReadModule(unsigned long mid, unsigned long address) const;

  unsigned long ReadRUN(unsigned long mid) const;

  void WriteCW(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadCW(unsigned long mid, unsigned long ch) const;

  void WriteGW(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadGW(unsigned long mid, unsigned long ch) const;

  void WriteRL(unsigned long mid, unsigned long data) const;
  unsigned long ReadRL(unsigned long mid) const;

  void WriteDRAMON(unsigned long mid, unsigned long data) const;
  unsigned long ReadDRAMON(unsigned long mid) const;

  void WriteDACOFF(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadDACOFF(unsigned long mid, unsigned long ch) const;

  void MeasurePED(unsigned long mid, unsigned long ch) const;
  unsigned long ReadPED(unsigned long mid, unsigned long ch) const;

  void WriteDLY(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadDLY(unsigned long mid, unsigned long ch) const;

  void WriteAMOREDLY(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadAMOREDLY(unsigned long mid, unsigned long ch) const;

  void WriteTHR(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadTHR(unsigned long mid, unsigned long ch) const;

  void WritePOL(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadPOL(unsigned long mid, unsigned long ch) const;

  void WritePSW(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadPSW(unsigned long mid, unsigned long ch) const;

  void WriteAMODE(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadAMODE(unsigned long mid, unsigned long ch) const;

  void WritePCT(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadPCT(unsigned long mid, unsigned long ch) const;

  void WritePCI(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadPCI(unsigned long mid, unsigned long ch) const;

  void WritePWT(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadPWT(unsigned long mid, unsigned long ch) const;

  void WriteDT(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadDT(unsigned long mid, unsigned long ch) const;

  void WriteTM(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadTM(unsigned long mid, unsigned long ch) const;

  void WriteTLT(unsigned long mid, unsigned long data) const;
  unsigned long ReadTLT(unsigned long mid) const;

  void WriteSTLT(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadSTLT(unsigned long mid, unsigned long ch) const;

  void WriteZEROSUP(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadZEROSUP(unsigned long mid, unsigned long ch) const;

  void SendADCRST(unsigned long mid) const;
  void SendADCCAL(unsigned long mid) const;

  void WriteADCDLY(unsigned long mid, unsigned long ch, unsigned long data) const;
  void WriteADCALIGN(unsigned long mid, unsigned long data) const;
  unsigned long ReadADCSTAT(unsigned long mid) const;

  void WriteBITSLIP(unsigned long mid, unsigned long ch, unsigned long data) const;

  void WriteFMUX(unsigned long mid, unsigned long ch) const;
  unsigned long ReadFMUX(unsigned long mid) const;

  void ArmFADC(unsigned long mid) const;
  unsigned long ReadFREADY(unsigned long mid) const;

  void WriteZSFD(unsigned long mid, unsigned long data) const;
  unsigned long ReadZSFD(unsigned long mid) const;

  void WriteDSR(unsigned long mid, unsigned long data) const;
  unsigned long ReadDSR(unsigned long mid) const;

  void ReadFADCBUF(unsigned long mid, unsigned long * data) const;

  void AlignFADC500(unsigned long mid) const;
  void AlignSADC64(unsigned long mid) const;
  void AlignDRAM(unsigned long mid) const;

  void WritePSS(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadPSS(unsigned long mid, unsigned long ch) const;

  void WriteRT(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadRT(unsigned long mid, unsigned long ch) const;

  void WriteSR(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadSR(unsigned long mid, unsigned long ch) const;

  void WriteDACGAIN(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadDACGAIN(unsigned long mid, unsigned long ch) const;

  void WriteST(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadST(unsigned long mid, unsigned long ch) const;

  void WritePT(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadPT(unsigned long mid, unsigned long ch) const;

  void WriteRUNNO(unsigned long data) const;
  unsigned long ReadRUNNO() const;

  void SendTRIG() const;

  unsigned long ReadLNSTAT() const;
  unsigned long ReadMIDS(unsigned long ch) const;

  void WritePTRIG(unsigned long data) const;
  unsigned long ReadPTRIG() const;

  void WriteTRIGENABLE(unsigned long mid, unsigned long data) const;
  unsigned long ReadTRIGENABLE(unsigned long mid) const;

  void WriteMTHR(unsigned long data) const;
  unsigned long ReadMTHR() const;

  void WritePSCALE(unsigned long data) const;
  unsigned long ReadPSCALE() const;

  void WriteDRAMDLY(unsigned long mid, unsigned long ch, unsigned long data) const;
  void WriteDRAMBITSLIP(unsigned long mid, unsigned long ch) const;
  void WriteDRAMTEST(unsigned long mid, unsigned long data) const;
  unsigned long ReadDRAMTEST(unsigned long mid, unsigned long ch) const;

private:
  int Transmit(char * buf, int len) const;
  int Receive(char * buf, int len) const;

  int _tcpHandle{-1};
  std::string _ipaddr{};
};

inline void NKMiniTCB::SetIPAddress(const char * ip)
{
  _ipaddr = ip;
}