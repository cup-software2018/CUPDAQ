#pragma once

#include <cstdint>

#include "Notice/USB3Com.hh"

class NKFADC125S {
public:
  NKFADC125S() = default;
  explicit NKFADC125S(int sid);
  ~NKFADC125S();

  void SetSID(int sid);

  int Open();
  void Close();

  void Reset() const;
  void ResetTimer() const;
  void Start() const;
  void Stop() const;

  unsigned long ReadRun() const;

  void WriteCW(unsigned long ch, unsigned long data) const;
  unsigned long ReadCW(unsigned long ch) const;

  void WriteRL(unsigned long data) const;
  unsigned long ReadRL() const;

  void WriteDRAMON(unsigned long data) const;
  unsigned long ReadDRAMON() const;

  void WriteDACOFF(unsigned long ch, unsigned long data) const;
  unsigned long ReadDACOFF(unsigned long ch) const;

  void MeasurePED(unsigned long ch) const;
  unsigned long ReadPED(unsigned long ch) const;

  void WriteDLY(unsigned long ch, unsigned long data) const;
  unsigned long ReadDLY(unsigned long ch) const;

  void WriteTHR(unsigned long ch, unsigned long data) const;
  unsigned long ReadTHR(unsigned long ch) const;

  void WritePOL(unsigned long ch, unsigned long data) const;
  unsigned long ReadPOL(unsigned long ch) const;

  void WritePSW(unsigned long ch, unsigned long data) const;
  unsigned long ReadPSW(unsigned long ch) const;

  void WritePCT(unsigned long ch, unsigned long data) const;
  unsigned long ReadPCT(unsigned long ch) const;

  void WritePCI(unsigned long ch, unsigned long data) const;
  unsigned long ReadPCI(unsigned long ch) const;

  void WritePWT(unsigned long ch, unsigned long data) const;
  unsigned long ReadPWT(unsigned long ch) const;

  void WriteDT(unsigned long ch, unsigned long data) const;
  unsigned long ReadDT(unsigned long ch) const;

  void WritePTRIG(unsigned long data) const;
  unsigned long ReadPTRIG() const;

  void SendTRIG() const;
  unsigned long ReadCHNUM() const;

  void WriteTRIGENABLE(unsigned long data) const;
  unsigned long ReadTRIGENABLE() const;

  void WriteTM(unsigned long ch, unsigned long data) const;
  unsigned long ReadTM(unsigned long ch) const;

  void WriteTLT(unsigned long data) const;
  unsigned long ReadTLT() const;

  void SendADCRST() const;
  void SendADCCAL() const;

  void WriteADCDLY(unsigned long ch, unsigned long data) const;
  void WriteADCSETUP(unsigned long addr, unsigned long data) const;
  unsigned long ReadADCSTAT() const;

  void WriteDRAMDLY(unsigned long ch, unsigned long data) const;
  void WriteDRAMBITSLIP(unsigned long ch) const;

  void WriteDRAMTEST(unsigned long data) const;
  unsigned long ReadDRAMTEST(unsigned long ch) const;

  void WriteADCBITSLIP(unsigned long ch, unsigned long data) const;

  void WritePSCALE(unsigned long data) const;
  unsigned long ReadPSCALE() const;

  void WriteDSR(unsigned long data) const;
  unsigned long ReadDSR() const;

  int ReadBCount() const;
  int ReadData(int bcount, unsigned char * data, unsigned int timeout = 0) const;
  void FlushData() const;

  void AlignADC() const;
  void AlignDRAM() const;

  int Sid() const;

private:
  int _sid{0};
  USB3Com _usb{};
};