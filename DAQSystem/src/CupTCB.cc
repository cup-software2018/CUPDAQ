#include <iostream>
#include <unistd.h>

#include "DAQSystem/CupTCB.hh"

using namespace std;

ClassImp(CupTCB)

CupTCB * CupTCB::fTCB = nullptr;

CupTCB * CupTCB::Instance()
{
  if (!fTCB) { fTCB = new CupTCB(); }
  return fTCB;
}

void CupTCB::DeleteInstance()
{
  if (fTCB) { delete fTCB; }
  fTCB = nullptr;
}

int CupTCB::Open() { return fNKTCB.Open(); }

void CupTCB::Close()
{
  fNKTCB.Stop();
  fNKTCB.Reset();
  fNKTCB.Close();
}

// tcb
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

void CupTCB::WriteCW(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WriteCW(mid, ch, data); }

unsigned long CupTCB::ReadCW(unsigned long mid, unsigned long ch) { return fNKTCB.ReadCW(mid, ch); }

// m64adc only
void CupTCB::WriteGW(unsigned long mid, unsigned long data) { fNKTCB.WriteGW(mid, data); }

unsigned long CupTCB::ReadGW(unsigned long mid) { return fNKTCB.ReadGW(mid); }

// fadc, sadc, amoreadc
void CupTCB::WriteRL(unsigned long mid, unsigned long data) { fNKTCB.WriteRL(mid, data); }

unsigned long CupTCB::ReadRL(unsigned long mid) { return fNKTCB.ReadRL(mid); }

void CupTCB::WriteDRAMON(unsigned long mid, unsigned long data) { fNKTCB.WriteDRAMON(mid, data); }

unsigned long CupTCB::ReadDRAMON(unsigned long mid) { return fNKTCB.ReadDRAMON(mid); }

void CupTCB::WriteDACOFF(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WriteDACOFF(mid, ch, data); }

unsigned long CupTCB::ReadDACOFF(unsigned long mid, unsigned long ch) { return fNKTCB.ReadDACOFF(mid, ch); }

void CupTCB::MeasurePED(unsigned long mid, unsigned long ch) { return fNKTCB.MeasurePED(mid, ch); }

unsigned long CupTCB::ReadPED(unsigned long mid, unsigned long ch) { return fNKTCB.ReadPED(mid, ch); }

void CupTCB::WriteDLY(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WriteDLY(mid, ch, data); }

unsigned long CupTCB::ReadDLY(unsigned long mid, unsigned long ch) { return fNKTCB.ReadDLY(mid, ch); }

void CupTCB::WriteTHR(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WriteTHR(mid, ch, data); }

unsigned long CupTCB::ReadTHR(unsigned long mid, unsigned long ch) { return fNKTCB.ReadTHR(mid, ch); }

// fadc, sadc
void CupTCB::WritePOL(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WritePOL(mid, ch, data); }

unsigned long CupTCB::ReadPOL(unsigned long mid, unsigned long ch) { return fNKTCB.ReadPOL(mid, ch); }

void CupTCB::WritePSW(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WritePSW(mid, ch, data); }

unsigned long CupTCB::ReadPSW(unsigned long mid, unsigned long ch) { return fNKTCB.ReadPSW(mid, ch); }

// fadc
void CupTCB::WriteAMODE(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WriteAMODE(mid, ch, data); }

unsigned long CupTCB::ReadAMODE(unsigned long mid, unsigned long ch) { return fNKTCB.ReadAMODE(mid, ch); }

void CupTCB::WritePCT(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WritePCT(mid, ch, data); }

unsigned long CupTCB::ReadPCT(unsigned long mid, unsigned long ch) { return fNKTCB.ReadPCT(mid, ch); }

void CupTCB::WritePCI(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WritePCI(mid, ch, data); }

unsigned long CupTCB::ReadPCI(unsigned long mid, unsigned long ch) { return fNKTCB.ReadPCI(mid, ch); }

void CupTCB::WritePWT(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WritePWT(mid, ch, data); }

unsigned long CupTCB::ReadPWT(unsigned long mid, unsigned long ch) { return fNKTCB.ReadPWT(mid, ch); }

void CupTCB::WriteDT(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WriteDT(mid, ch, data); }

unsigned long CupTCB::ReadDT(unsigned long mid, unsigned long ch) { return fNKTCB.ReadDT(mid, ch); }

void CupTCB::WriteTM(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WriteTM(mid, ch, data); }

unsigned long CupTCB::ReadTM(unsigned long mid, unsigned long ch) { return fNKTCB.ReadTM(mid, ch); }

void CupTCB::WriteTLT(unsigned long mid, unsigned long data) { fNKTCB.WriteTLT(mid, data); }

unsigned long CupTCB::ReadTLT(unsigned long mid) { return fNKTCB.ReadTLT(mid); }

void CupTCB::WriteSTLT(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WriteSTLT(mid, ch, data); }

unsigned long CupTCB::ReadSTLT(unsigned long mid, unsigned long ch) { return fNKTCB.ReadSTLT(mid, ch); }

void CupTCB::WriteDSR(unsigned long mid, unsigned long data) { fNKTCB.WriteDSR(mid, data); }

unsigned long CupTCB::ReadDSR(unsigned long mid) { return fNKTCB.ReadDSR(mid); }

void CupTCB::AlignFADC(unsigned long mid) { fNKTCB.AlignFADC500(mid); }

void CupTCB::WritePSS(unsigned long mid, unsigned long ch, unsigned long data) { fNKTCB.WritePSS(mid, ch, data); }

unsigned long CupTCB::ReadPSS(unsigned long mid, unsigned long ch) { return fNKTCB.ReadPSS(mid, ch); }

void CupTCB::AlignSADC(unsigned long mid) { fNKTCB.AlignSADC64(mid); }

void CupTCB::AlignGADC(unsigned long mid) { fNKTCB.AlignFADC125(mid); }

// iadc
void CupTCB::WriteDAQMODE(unsigned long mid, unsigned long data) { fNKTCB.WriteDAQMODE(mid, data); }

unsigned long CupTCB::ReadDAQMODE(unsigned long mid) { return fNKTCB.ReadDAQMODE(mid); }

void CupTCB::WriteHV(unsigned long mid, unsigned long ch, float data) { fNKTCB.WriteHV(mid, ch, data); }

float CupTCB::ReadHV(unsigned long mid, unsigned long ch) { return fNKTCB.ReadHV(mid, ch); }

float CupTCB::ReadTEMP(unsigned long mid, unsigned long ch) { return fNKTCB.ReadTEMP(mid, ch); }

void CupTCB::AlignIADC(unsigned long mid) { fNKTCB.AlignIADC64(mid); }

void CupTCB::WritePTRIG(unsigned long data) { fNKTCB.WritePTRIG(data); }

unsigned long CupTCB::ReadPTRIG() { return fNKTCB.ReadPTRIG(); }

void CupTCB::WriteTRIGENABLE(unsigned long mid, unsigned long data) { fNKTCB.WriteTRIGENABLE(mid, data); }

unsigned long CupTCB::ReadTRIGENABLE(unsigned long mid) { return fNKTCB.ReadTRIGENABLE(mid); }

void CupTCB::WriteMTHRFADC(unsigned long data) { fNKTCB.WriteMTHRFADC500(data); }

unsigned long CupTCB::ReadMTHRFADC() { return fNKTCB.ReadMTHRFADC500(); }

void CupTCB::WritePSCALEFADC(unsigned long data) { fNKTCB.WritePSCALEFADC500(data); }

unsigned long CupTCB::ReadPSCALEFADC() { return fNKTCB.ReadPSCALEFADC500(); }

void CupTCB::WriteMTHRSADCMU(unsigned long data) { fNKTCB.WriteMTHRSADC64MU(data); }

unsigned long CupTCB::ReadMTHRSADCMU() { return fNKTCB.ReadMTHRSADC64MU(); }

void CupTCB::WritePSCALESADCMU(unsigned long data) { fNKTCB.WritePSCALESADC64MU(data); }

unsigned long CupTCB::ReadPSCALESADCMU() { return fNKTCB.ReadPSCALESADC64MU(); }

void CupTCB::WriteMTHRSADCLS(unsigned long data) { fNKTCB.WriteMTHRSADC64LS(data); }

unsigned long CupTCB::ReadMTHRSADCLS() { return fNKTCB.ReadMTHRSADC64LS(); }

void CupTCB::WritePSCALESADCLS(unsigned long data) { fNKTCB.WritePSCALESADC64LS(data); }

unsigned long CupTCB::ReadPSCALESADCLS() { return fNKTCB.ReadPSCALESADC64LS(); }

void CupTCB::WriteMTHRIADC(unsigned long data) { fNKTCB.WriteMTHRIADC64(data); }

unsigned long CupTCB::ReadMTHRIADC() { return fNKTCB.ReadMTHRIADC64(); }

void CupTCB::WritePSCALEIADC(unsigned long data) { fNKTCB.WritePSCALEIADC64(data); }

unsigned long CupTCB::ReadPSCALEIADC() { return fNKTCB.ReadPSCALEIADC64(); }

void CupTCB::WriteEXTOUT(unsigned long data) { fNKTCB.WriteEXTOUT(data); }

unsigned long CupTCB::ReadEXTOUT() { return fNKTCB.ReadEXTOUT(); }

void CupTCB::SendTRIG() { fNKTCB.SendTRIG(); }

void CupTCB::ReadLNSTAT(unsigned long * data) { fNKTCB.ReadLNSTAT(data); }

void CupTCB::ReadMIDS(unsigned long * data) { fNKTCB.ReadMIDS(data); }

unsigned long CupTCB::ReadBCount(unsigned long mid)
{
  std::lock_guard<std::mutex> lock(fMutex);
  return fNKTCB.ReadBCount(mid);
}

int CupTCB::ReadData(unsigned long mid, unsigned long bcount, unsigned char * data)
{
  std::lock_guard<std::mutex> lock(fMutex);
  return fNKTCB.ReadData(mid, bcount, data);
}

void CupTCB::WriteGATEDLY(unsigned long data) { fNKTCB.WriteGATEDLY(data); }

unsigned long CupTCB::ReadGATEDLY() { return fNKTCB.ReadGATEDLY(); }

void CupTCB::WriteGATEWIDTH(unsigned long data) { fNKTCB.WriteGATEWIDTH(data); }

unsigned long CupTCB::ReadGATEWIDTH() { return fNKTCB.ReadGATEWIDTH(); }

void CupTCB::WriteEXTOUTWIDTH(unsigned long data) { fNKTCB.WriteEXTOUTWIDTH(data); }

unsigned long CupTCB::ReadEXTOUTWIDTH() { return fNKTCB.ReadEXTOUTWIDTH(); }

void CupTCB::AlignDRAM(unsigned long mid) { fNKTCB.AlignDRAM(mid); }

void CupTCB::WriteTRGSWFADC(unsigned long fadc, unsigned long sadcmu, unsigned long sadcls, unsigned long iadc)
{
  fNKTCB.WriteTrigSwitchFADC500(fadc, sadcmu, sadcls, iadc);
}

void CupTCB::WriteTRGSWSADCMU(unsigned long fadc, unsigned long sadcmu, unsigned long sadcls, unsigned long iadc)
{
  fNKTCB.WriteTrigSwitchSADC64MU(fadc, sadcmu, sadcls, iadc);
}

void CupTCB::WriteTRGSWSADCLS(unsigned long fadc, unsigned long sadcmu, unsigned long sadcls, unsigned long iadc)
{
  fNKTCB.WriteTrigSwitchFADC500(fadc, sadcmu, sadcls, iadc);
}

void CupTCB::WriteTRGSWIADC(unsigned long fadc, unsigned long sadcmu, unsigned long sadcls, unsigned long iadc)
{
  fNKTCB.WriteTrigSwitchIADC64(fadc, sadcmu, sadcls, iadc);
}

unsigned long CupTCB::ReadTRGSWFADC() { return fNKTCB.ReadSwitchFADC500(); }

unsigned long CupTCB::ReadTRGSWSADCMU() { return fNKTCB.ReadSwitchSADC64MU(); }

unsigned long CupTCB::ReadTRGSWSADCLS() { return fNKTCB.ReadSwitchSADC64LS(); }

unsigned long CupTCB::ReadTRGSWIADC() { return fNKTCB.ReadSwitchIADC64(); }
