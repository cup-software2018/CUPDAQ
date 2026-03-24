#include <iostream>
#include <unistd.h>

#include "DAQSystem/CupTCB.hh"

ClassImp(CupTCB)

// Thread-safe Meyers' Singleton
CupTCB * CupTCB::Instance()
{
  static CupTCB instance;
  return &instance;
}

// No-op for static singleton, but kept for interface compatibility
void CupTCB::DeleteInstance()
{
  // The instance will be destroyed automatically at program exit.
}

int CupTCB::Open() { return fNKTCB.Open(); }

void CupTCB::Close()
{
  fNKTCB.Stop();
  fNKTCB.Reset();
  fNKTCB.Close();
}

// -----------------------------------------------------------------------
// Control Functions
// -----------------------------------------------------------------------
void CupTCB::Reset() { fNKTCB.Reset(); }

void CupTCB::ResetTimer() { fNKTCB.ResetTimer(); }

void CupTCB::TriggerStart()
{
  fNKTCB.Reset();
  fNKTCB.ResetTimer();
  fNKTCB.Start();
}

void CupTCB::TriggerStop()
{
  std::lock_guard<std::mutex> lock(fMutex);
  fNKTCB.Stop();
  fNKTCB.Reset();
}

// -----------------------------------------------------------------------
// Register Wrappers
// -----------------------------------------------------------------------
void CupTCB::WriteCW(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WriteCW(mid, ch, data);
}
uint32_t CupTCB::ReadCW(uint32_t mid, uint32_t ch) { return fNKTCB.ReadCW(mid, ch); }

void CupTCB::WriteGW(uint32_t mid, uint32_t data) { fNKTCB.WriteGW(mid, data); }
uint32_t CupTCB::ReadGW(uint32_t mid) { return fNKTCB.ReadGW(mid); }

void CupTCB::WriteRL(uint32_t mid, uint32_t data) { fNKTCB.WriteRL(mid, data); }
uint32_t CupTCB::ReadRL(uint32_t mid) { return fNKTCB.ReadRL(mid); }

void CupTCB::WriteDRAMON(uint32_t mid, uint32_t data) { fNKTCB.WriteDRAMON(mid, data); }
uint32_t CupTCB::ReadDRAMON(uint32_t mid) { return fNKTCB.ReadDRAMON(mid); }

void CupTCB::WriteDACOFF(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WriteDACOFF(mid, ch, data);
}
uint32_t CupTCB::ReadDACOFF(uint32_t mid, uint32_t ch)
{
  return fNKTCB.ReadDACOFF(mid, ch);
}

void CupTCB::MeasurePED(uint32_t mid, uint32_t ch) { fNKTCB.MeasurePED(mid, ch); }
uint32_t CupTCB::ReadPED(uint32_t mid, uint32_t ch)
{
  return fNKTCB.ReadPED(mid, ch);
}

void CupTCB::WriteDLY(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WriteDLY(mid, ch, data);
}
uint32_t CupTCB::ReadDLY(uint32_t mid, uint32_t ch)
{
  return fNKTCB.ReadDLY(mid, ch);
}

void CupTCB::WriteTHR(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WriteTHR(mid, ch, data);
}
uint32_t CupTCB::ReadTHR(uint32_t mid, uint32_t ch)
{
  return fNKTCB.ReadTHR(mid, ch);
}

void CupTCB::WritePOL(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WritePOL(mid, ch, data);
}
uint32_t CupTCB::ReadPOL(uint32_t mid, uint32_t ch)
{
  return fNKTCB.ReadPOL(mid, ch);
}

void CupTCB::WritePSW(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WritePSW(mid, ch, data);
}
uint32_t CupTCB::ReadPSW(uint32_t mid, uint32_t ch)
{
  return fNKTCB.ReadPSW(mid, ch);
}

void CupTCB::WriteAMODE(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WriteAMODE(mid, ch, data);
}
uint32_t CupTCB::ReadAMODE(uint32_t mid, uint32_t ch)
{
  return fNKTCB.ReadAMODE(mid, ch);
}

void CupTCB::WritePCT(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WritePCT(mid, ch, data);
}
uint32_t CupTCB::ReadPCT(uint32_t mid, uint32_t ch)
{
  return fNKTCB.ReadPCT(mid, ch);
}

void CupTCB::WritePCI(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WritePCI(mid, ch, data);
}
uint32_t CupTCB::ReadPCI(uint32_t mid, uint32_t ch)
{
  return fNKTCB.ReadPCI(mid, ch);
}

void CupTCB::WritePWT(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WritePWT(mid, ch, data);
}
uint32_t CupTCB::ReadPWT(uint32_t mid, uint32_t ch)
{
  return fNKTCB.ReadPWT(mid, ch);
}

void CupTCB::WriteDT(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WriteDT(mid, ch, data);
}
uint32_t CupTCB::ReadDT(uint32_t mid, uint32_t ch) { return fNKTCB.ReadDT(mid, ch); }

void CupTCB::WriteTM(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WriteTM(mid, ch, data);
}
uint32_t CupTCB::ReadTM(uint32_t mid, uint32_t ch) { return fNKTCB.ReadTM(mid, ch); }

void CupTCB::WriteTLT(uint32_t mid, uint32_t data) { fNKTCB.WriteTLT(mid, data); }
uint32_t CupTCB::ReadTLT(uint32_t mid) { return fNKTCB.ReadTLT(mid); }

void CupTCB::WriteSTLT(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WriteSTLT(mid, ch, data);
}
uint32_t CupTCB::ReadSTLT(uint32_t mid, uint32_t ch)
{
  return fNKTCB.ReadSTLT(mid, ch);
}

void CupTCB::WriteDSR(uint32_t mid, uint32_t data) { fNKTCB.WriteDSR(mid, data); }
uint32_t CupTCB::ReadDSR(uint32_t mid) { return fNKTCB.ReadDSR(mid); }

void CupTCB::WritePSS(uint32_t mid, uint32_t ch, uint32_t data)
{
  fNKTCB.WritePSS(mid, ch, data);
}
uint32_t CupTCB::ReadPSS(uint32_t mid, uint32_t ch)
{
  return fNKTCB.ReadPSS(mid, ch);
}

// -----------------------------------------------------------------------
// Alignment
// -----------------------------------------------------------------------
void CupTCB::AlignFADC(uint32_t mid) { fNKTCB.AlignFADC500(mid); }
void CupTCB::AlignSADC(uint32_t mid) { fNKTCB.AlignSADC64(mid); }
void CupTCB::AlignGADC(uint32_t mid) { fNKTCB.AlignFADC125(mid); }
void CupTCB::AlignIADC(uint32_t mid) { fNKTCB.AlignIADC64(mid); }
void CupTCB::AlignDRAM(uint32_t mid) { fNKTCB.AlignDRAM(mid); }

// -----------------------------------------------------------------------
// IADC Specifics
// -----------------------------------------------------------------------
void CupTCB::WriteDAQMODE(uint32_t mid, uint32_t data) { fNKTCB.WriteDAQMODE(mid, data); }
uint32_t CupTCB::ReadDAQMODE(uint32_t mid) { return fNKTCB.ReadDAQMODE(mid); }

void CupTCB::WriteHV(uint32_t mid, uint32_t ch, float data)
{
  fNKTCB.WriteHV(mid, ch, data);
}
float CupTCB::ReadHV(uint32_t mid, uint32_t ch) { return fNKTCB.ReadHV(mid, ch); }
float CupTCB::ReadTEMP(uint32_t mid, uint32_t ch) { return fNKTCB.ReadTEMP(mid, ch); }

// -----------------------------------------------------------------------
// Trigger Configuration
// -----------------------------------------------------------------------
void CupTCB::WritePTRIG(uint32_t data) { fNKTCB.WritePTRIG(data); }
uint32_t CupTCB::ReadPTRIG() { return fNKTCB.ReadPTRIG(); }

void CupTCB::WriteTRIGENABLE(uint32_t mid, uint32_t data)
{
  fNKTCB.WriteTRIGENABLE(mid, data);
}
uint32_t CupTCB::ReadTRIGENABLE(uint32_t mid) { return fNKTCB.ReadTRIGENABLE(mid); }

void CupTCB::WriteMTHRFADC(uint32_t data) { fNKTCB.WriteMTHRFADC500(data); }
uint32_t CupTCB::ReadMTHRFADC() { return fNKTCB.ReadMTHRFADC500(); }

void CupTCB::WritePSCALEFADC(uint32_t data) { fNKTCB.WritePSCALEFADC500(data); }
uint32_t CupTCB::ReadPSCALEFADC() { return fNKTCB.ReadPSCALEFADC500(); }

void CupTCB::WriteMTHRSADCMU(uint32_t data) { fNKTCB.WriteMTHRSADC64MU(data); }
uint32_t CupTCB::ReadMTHRSADCMU() { return fNKTCB.ReadMTHRSADC64MU(); }

void CupTCB::WritePSCALESADCMU(uint32_t data) { fNKTCB.WritePSCALESADC64MU(data); }
uint32_t CupTCB::ReadPSCALESADCMU() { return fNKTCB.ReadPSCALESADC64MU(); }

void CupTCB::WriteMTHRSADCLS(uint32_t data) { fNKTCB.WriteMTHRSADC64LS(data); }
uint32_t CupTCB::ReadMTHRSADCLS() { return fNKTCB.ReadMTHRSADC64LS(); }

void CupTCB::WritePSCALESADCLS(uint32_t data) { fNKTCB.WritePSCALESADC64LS(data); }
uint32_t CupTCB::ReadPSCALESADCLS() { return fNKTCB.ReadPSCALESADC64LS(); }

void CupTCB::WriteMTHRIADC(uint32_t data) { fNKTCB.WriteMTHRIADC64(data); }
uint32_t CupTCB::ReadMTHRIADC() { return fNKTCB.ReadMTHRIADC64(); }

void CupTCB::WritePSCALEIADC(uint32_t data) { fNKTCB.WritePSCALEIADC64(data); }
uint32_t CupTCB::ReadPSCALEIADC() { return fNKTCB.ReadPSCALEIADC64(); }

void CupTCB::WriteEXTOUT(uint32_t data) { fNKTCB.WriteEXTOUT(data); }
uint32_t CupTCB::ReadEXTOUT() { return fNKTCB.ReadEXTOUT(); }

void CupTCB::WriteGATEDLY(uint32_t data) { fNKTCB.WriteGATEDLY(data); }
uint32_t CupTCB::ReadGATEDLY() { return fNKTCB.ReadGATEDLY(); }

void CupTCB::WriteGATEWIDTH(uint32_t data) { fNKTCB.WriteGATEWIDTH(data); }
uint32_t CupTCB::ReadGATEWIDTH() { return fNKTCB.ReadGATEWIDTH(); }

void CupTCB::WriteEXTOUTWIDTH(uint32_t data) { fNKTCB.WriteEXTOUTWIDTH(data); }
uint32_t CupTCB::ReadEXTOUTWIDTH() { return fNKTCB.ReadEXTOUTWIDTH(); }

void CupTCB::SendTRIG() { fNKTCB.SendTRIG(); }

void CupTCB::ReadLNSTAT(uint32_t * data) { fNKTCB.ReadLNSTAT(data); }
void CupTCB::ReadMIDS(uint32_t * data) { fNKTCB.ReadMIDS(data); }

// -----------------------------------------------------------------------
// Data Readout & Mutex Protected Area
// -----------------------------------------------------------------------
uint32_t CupTCB::ReadBCount(uint32_t mid)
{
  std::lock_guard<std::mutex> lock(fMutex);
  return fNKTCB.ReadBCount(mid);
}

int CupTCB::ReadData(uint32_t mid, uint32_t bcount, unsigned char * data)
{
  std::lock_guard<std::mutex> lock(fMutex);
  return fNKTCB.ReadData(mid, bcount, data);
}

// -----------------------------------------------------------------------
// Trigger Switches (Fixed Copy/Paste Bug)
// -----------------------------------------------------------------------
void CupTCB::WriteTRGSWFADC(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls,
                            uint32_t iadc)
{
  fNKTCB.WriteTrigSwitchFADC500(fadc, sadcmu, sadcls, iadc);
}

void CupTCB::WriteTRGSWSADCMU(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls,
                              uint32_t iadc)
{
  fNKTCB.WriteTrigSwitchSADC64MU(fadc, sadcmu, sadcls, iadc);
}

void CupTCB::WriteTRGSWSADCLS(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls,
                              uint32_t iadc)
{
  // BUG FIX: Was calling WriteTrigSwitchFADC500, now calls WriteTrigSwitchSADC64LS
  fNKTCB.WriteTrigSwitchSADC64LS(fadc, sadcmu, sadcls, iadc);
}

void CupTCB::WriteTRGSWIADC(uint32_t fadc, uint32_t sadcmu, uint32_t sadcls,
                            uint32_t iadc)
{
  fNKTCB.WriteTrigSwitchIADC64(fadc, sadcmu, sadcls, iadc);
}

uint32_t CupTCB::ReadTRGSWFADC() { return fNKTCB.ReadSwitchFADC500(); }
uint32_t CupTCB::ReadTRGSWSADCMU() { return fNKTCB.ReadSwitchSADC64MU(); }
uint32_t CupTCB::ReadTRGSWSADCLS() { return fNKTCB.ReadSwitchSADC64LS(); }
uint32_t CupTCB::ReadTRGSWIADC() { return fNKTCB.ReadSwitchIADC64(); }