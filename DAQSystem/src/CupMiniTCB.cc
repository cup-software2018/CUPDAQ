#include "DAQSystem/CupMiniTCB.hh"
#include "DAQUtils/ELog.hh"

ClassImp(CupMiniTCB)

int CupMiniTCB::Open()
{
  fTCB.SetIPAddress(fIPAddress.c_str());
  return fTCB.Open();
}

void CupMiniTCB::Close()
{
  TriggerStop();
  Reset();
  fTCB.Close();
}

void CupMiniTCB::Reset() { fTCB.Reset(); }

void CupMiniTCB::ResetTimer() { fTCB.ResetTimer(); }

void CupMiniTCB::TriggerStart() { fTCB.Start(); }

void CupMiniTCB::TriggerStop() { fTCB.Stop(); }

void CupMiniTCB::WriteCW(unsigned long mid, unsigned long ch, unsigned long data) { fTCB.WriteCW(mid, ch, data); }

unsigned long CupMiniTCB::ReadCW(unsigned long mid, unsigned long ch) { return fTCB.ReadCW(mid, ch); }

void CupMiniTCB::WriteGW(unsigned long mid, unsigned long data) { fTCB.WriteGW(mid, 0, data); }

unsigned long CupMiniTCB::ReadGW(unsigned long mid) { return fTCB.ReadGW(mid, 0); }

void CupMiniTCB::WriteRL(unsigned long mid, unsigned long data) { fTCB.WriteRL(mid, data); }

unsigned long CupMiniTCB::ReadRL(unsigned long mid) { return fTCB.ReadRL(mid); }

void CupMiniTCB::WriteDRAMON(unsigned long mid, unsigned long data) { fTCB.WriteDRAMON(mid, data); }

unsigned long CupMiniTCB::ReadDRAMON(unsigned long mid) { return fTCB.ReadDRAMON(mid); }

void CupMiniTCB::WriteDACOFF(unsigned long mid, unsigned long ch, unsigned long data)
{
  fTCB.WriteDACOFF(mid, ch, data);
}

unsigned long CupMiniTCB::ReadDACOFF(unsigned long mid, unsigned long ch) { return fTCB.ReadDACOFF(mid, ch); }

void CupMiniTCB::MeasurePED(unsigned long mid, unsigned long ch) { fTCB.MeasurePED(mid, ch); }

unsigned long CupMiniTCB::ReadPED(unsigned long mid, unsigned long ch) { return fTCB.ReadPED(mid, ch); }

void CupMiniTCB::WriteDLY(unsigned long mid, unsigned long ch, unsigned long data) { fTCB.WriteDLY(mid, ch, data); }

unsigned long CupMiniTCB::ReadDLY(unsigned long mid, unsigned long ch) { return fTCB.ReadDLY(mid, ch); }

void CupMiniTCB::WriteTHR(unsigned long mid, unsigned long ch, unsigned long data) { fTCB.WriteTHR(mid, ch, data); }

unsigned long CupMiniTCB::ReadTHR(unsigned long mid, unsigned long ch) { return fTCB.ReadTHR(mid, ch); }

void CupMiniTCB::WritePOL(unsigned long mid, unsigned long ch, unsigned long data) { fTCB.WritePOL(mid, ch, data); }

unsigned long CupMiniTCB::ReadPOL(unsigned long mid, unsigned long ch) { return fTCB.ReadPOL(mid, ch); }

void CupMiniTCB::WritePSW(unsigned long mid, unsigned long ch, unsigned long data) { fTCB.WritePSW(mid, ch, data); }

unsigned long CupMiniTCB::ReadPSW(unsigned long mid, unsigned long ch) { return fTCB.ReadPSW(mid, ch); }

void CupMiniTCB::WriteAMODE(unsigned long mid, unsigned long ch, unsigned long data) { fTCB.WriteAMODE(mid, ch, data); }

unsigned long CupMiniTCB::ReadAMODE(unsigned long mid, unsigned long ch) { return fTCB.ReadAMODE(mid, ch); }

void CupMiniTCB::WritePCT(unsigned long mid, unsigned long ch, unsigned long data) { fTCB.WritePCT(mid, ch, data); }

unsigned long CupMiniTCB::ReadPCT(unsigned long mid, unsigned long ch) { return fTCB.ReadPCT(mid, ch); }

void CupMiniTCB::WritePCI(unsigned long mid, unsigned long ch, unsigned long data) { fTCB.WritePCI(mid, ch, data); }

unsigned long CupMiniTCB::ReadPCI(unsigned long mid, unsigned long ch) { return fTCB.ReadPCI(mid, ch); }

void CupMiniTCB::WritePWT(unsigned long mid, unsigned long ch, unsigned long data) { fTCB.WritePWT(mid, ch, data); }

unsigned long CupMiniTCB::ReadPWT(unsigned long mid, unsigned long ch) { return fTCB.ReadPWT(mid, ch); }

void CupMiniTCB::WriteDT(unsigned long mid, unsigned long ch, unsigned long data) { fTCB.WriteDT(mid, ch, data); }

unsigned long CupMiniTCB::ReadDT(unsigned long mid, unsigned long ch) { return fTCB.ReadDT(mid, ch); }

void CupMiniTCB::WriteTM(unsigned long mid, unsigned long ch, unsigned long data) { fTCB.WriteTM(mid, ch, data); }

unsigned long CupMiniTCB::ReadTM(unsigned long mid, unsigned long ch) { return fTCB.ReadTM(mid, ch); }

void CupMiniTCB::WriteTLT(unsigned long mid, unsigned long data) { fTCB.WriteTLT(mid, data); }

unsigned long CupMiniTCB::ReadTLT(unsigned long mid) { return fTCB.ReadTLT(mid); }

void CupMiniTCB::WriteSTLT(unsigned long mid, unsigned long ch, unsigned long data) { fTCB.WriteSTLT(mid, ch, data); }

unsigned long CupMiniTCB::ReadSTLT(unsigned long mid, unsigned long ch) { return fTCB.ReadSTLT(mid, ch); }

void CupMiniTCB::WriteDSR(unsigned long mid, unsigned long data) { fTCB.WriteDSR(mid, data); }

unsigned long CupMiniTCB::ReadDSR(unsigned long mid) { return fTCB.ReadDSR(mid); }

void CupMiniTCB::AlignFADC(unsigned long mid) { fTCB.AlignFADC500(mid); }

void CupMiniTCB::WritePSS(unsigned long mid, unsigned long ch, unsigned long data) { fTCB.WritePSS(mid, ch, data); }

unsigned long CupMiniTCB::ReadPSS(unsigned long mid, unsigned long ch) { return fTCB.ReadPSS(mid, ch); }

void CupMiniTCB::AlignSADC(unsigned long mid) { fTCB.AlignSADC64(mid); }

void CupMiniTCB::AlignGADC(unsigned long mid) {}

void CupMiniTCB::WritePTRIG(unsigned long data) { fTCB.WritePTRIG(data); }

unsigned long CupMiniTCB::ReadPTRIG() { return fTCB.ReadPTRIG(); }

void CupMiniTCB::WriteTRIGENABLE(unsigned long mid, unsigned long data) { fTCB.WriteTRIGENABLE(mid, data); }

unsigned long CupMiniTCB::ReadTRIGENABLE(unsigned long mid) { return fTCB.ReadTRIGENABLE(mid); }

void CupMiniTCB::WriteMTHRFADC(unsigned long data) { fTCB.WriteMTHR(data); }

unsigned long CupMiniTCB::ReadMTHRFADC() { return fTCB.ReadMTHR(); }

void CupMiniTCB::WritePSCALEFADC(unsigned long data) { fTCB.WritePSCALE(data); }

unsigned long CupMiniTCB::ReadPSCALEFADC() { return fTCB.ReadPSCALE(); }

void CupMiniTCB::SendTRIG() { fTCB.SendTRIG(); }

void CupMiniTCB::ReadLNSTAT(unsigned long * data) { data[0] = fTCB.ReadLNSTAT(); }

void CupMiniTCB::ReadMIDS(unsigned long * data)
{
  for (int i = 1; i <= 4; i++) {
    data[i - 1] = fTCB.ReadMIDS(static_cast<unsigned long>(i));
  }
}

void CupMiniTCB::AlignDRAM(unsigned long mid) { fTCB.AlignDRAM(mid); }
