#pragma once

#include <cstdint>

#include "Notice/USB3Tcb.hh"

#define TM_COUNT (0x1)
#define TM_WIDTH (0x2)
#define TM_PEAKSUM (0x4)
#define TM_ORPEAKSUM (0x8)

class NKTCB {
public:
  NKTCB();
  ~NKTCB();

  int Open();
  void Close();

  void WriteLT(unsigned long mid, const char * data, int len);

  void Reset();
  void ResetTimer();
  void Start();
  void Stop();
  unsigned long ReadRUN(unsigned long mid) const;

  void WriteCW(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadCW(unsigned long mid, unsigned long ch) const;

  void WriteRL(unsigned long mid, unsigned long data) const;
  unsigned long ReadRL(unsigned long mid) const;

  void WriteGW(unsigned long mid, unsigned long data) const;
  unsigned long ReadGW(unsigned long mid) const;

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

  void WritePSS(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadPSS(unsigned long mid, unsigned long ch) const;

  void WriteRT(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadRT(unsigned long mid, unsigned long ch) const;

  void WriteSR(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadSR(unsigned long mid, unsigned long ch) const;

  void WriteDACGain(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadDACGain(unsigned long mid, unsigned long ch) const;

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

  void WriteST(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadST(unsigned long mid, unsigned long ch) const;

  void WritePT(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadPT(unsigned long mid, unsigned long ch) const;

  void WriteDRAMDLY(unsigned long mid, unsigned long ch, unsigned long data) const;
  void WriteDRAMBITSLIP(unsigned long mid, unsigned long ch) const;
  void WriteDRAMTEST(unsigned long mid, unsigned long data) const;
  unsigned long ReadDRAMTEST(unsigned long mid, unsigned long ch) const;

  void WriteDAQMODE(unsigned long mid, unsigned long data) const;
  unsigned long ReadDAQMODE(unsigned long mid) const;

  void WriteHV(unsigned long mid, unsigned long ch, float data) const;
  float ReadHV(unsigned long mid, unsigned long ch) const;

  float ReadTEMP(unsigned long mid, unsigned long ch) const;

  void WriteADCMUX(unsigned long mid, unsigned long ch, unsigned long data) const;
  void ReadFADCBUF(unsigned long mid, unsigned long * data) const;

  void AlignFADC500(unsigned long mid) const;
  void AlignSADC64(unsigned long mid) const;
  void AlignFADC125(unsigned long mid) const;
  void AlignIADC64(unsigned long mid) const;
  void AlignDRAM(unsigned long mid) const;

  void WriteRUNNO(unsigned long data) const;
  unsigned long ReadRUNNO() const;

  void WriteGATEDLY(unsigned long data) const;
  unsigned long ReadGATEDLY() const;

  void SendTRIG() const;

  void ReadLNSTAT(unsigned long * data) const;
  void ReadMIDS(unsigned long * data) const;

  void WritePTRIG(unsigned long data) const;
  unsigned long ReadPTRIG() const;

  void WriteTRIGENABLE(unsigned long mid, unsigned long data) const;
  unsigned long ReadTRIGENABLE(unsigned long mid) const;

  void WriteEXTOUT(unsigned long data) const;
  unsigned long ReadEXTOUT() const;

  void WriteGATEWIDTH(unsigned long data) const;
  unsigned long ReadGATEWIDTH() const;

  void WriteEXTOUTWIDTH(unsigned long data) const;
  unsigned long ReadEXTOUTWIDTH() const;

  unsigned long ReadBCount(unsigned long mid) const;
  int ReadData(unsigned long mid, unsigned long bcount, unsigned char * data) const;

  void WriteGAIN(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadGAIN(unsigned long mid, unsigned long ch) const;

  void WriteMTHR(unsigned long mid, unsigned long data) const;
  unsigned long ReadMTHR(unsigned long mid) const;

  void WritePSDDLY(unsigned long mid, unsigned long data) const;
  unsigned long ReadPSDDLY(unsigned long mid) const;

  void WritePSDTHR(unsigned long mid, float data) const;
  float ReadPSDTHR(unsigned long mid) const;

  unsigned long ReadADCSTATWORD(unsigned long mid) const;

  void WriteMTHRFADC500(unsigned long data) const;
  unsigned long ReadMTHRFADC500() const;

  void WritePSCALEFADC500(unsigned long data) const;
  unsigned long ReadPSCALEFADC500() const;

  void WriteMTHRSADC64MU(unsigned long data) const;
  unsigned long ReadMTHRSADC64MU() const;

  void WritePSCALESADC64MU(unsigned long data) const;
  unsigned long ReadPSCALESADC64MU() const;

  void WriteMTHRSADC64LS(unsigned long data) const;
  unsigned long ReadMTHRSADC64LS() const;

  void WritePSCALESADC64LS(unsigned long data) const;
  unsigned long ReadPSCALESADC64LS() const;

  void WriteMTHRIADC64(unsigned long data) const;
  unsigned long ReadMTHRIADC64() const;

  void WritePSCALEIADC64(unsigned long data) const;
  unsigned long ReadPSCALEIADC64() const;

  void WriteTrigSwitchFADC500(unsigned long fadc, unsigned long sadc_muon, unsigned long sadc_ls,
                              unsigned long muondaq) const;
  unsigned long ReadSwitchFADC500() const;

  void WriteTrigSwitchSADC64MU(unsigned long fadc, unsigned long sadc_muon, unsigned long sadc_ls,
                               unsigned long muondaq) const;
  unsigned long ReadSwitchSADC64MU() const;

  void WriteTrigSwitchSADC64LS(unsigned long fadc, unsigned long sadc_muon, unsigned long sadc_ls,
                               unsigned long muondaq) const;
  unsigned long ReadSwitchSADC64LS() const;

  void WriteTrigSwitchIADC64(unsigned long fadc, unsigned long sadc_muon, unsigned long sadc_ls,
                             unsigned long muondaq) const;
  unsigned long ReadSwitchIADC64() const;

  unsigned long ReadDBG() const;

  void WriteDT(unsigned long mid, unsigned long ch, unsigned long data) const;
  unsigned long ReadDT(unsigned long mid, unsigned long ch) const;

private:
  USB3Tcb _usb;
};
