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

void CupMiniTCB::WriteCW(uint32_t mid, uint32_t ch, uint32_t data) { fTCB.WriteCW(mid, ch, data); }

uint32_t CupMiniTCB::ReadCW(uint32_t mid, uint32_t ch) { return fTCB.ReadCW(mid, ch); }

void CupMiniTCB::WriteGW(uint32_t mid, uint32_t data) { fTCB.WriteGW(mid, 0, data); }

uint32_t CupMiniTCB::ReadGW(uint32_t mid) { return fTCB.ReadGW(mid, 0); }

void CupMiniTCB::WriteRL(uint32_t mid, uint32_t data) { fTCB.WriteRL(mid, data); }

uint32_t CupMiniTCB::ReadRL(uint32_t mid) { return fTCB.ReadRL(mid); }

void CupMiniTCB::WriteDRAMON(uint32_t mid, uint32_t data) { fTCB.WriteDRAMON(mid, data); }

uint32_t CupMiniTCB::ReadDRAMON(uint32_t mid) { return fTCB.ReadDRAMON(mid); }

void CupMiniTCB::WriteDACOFF(uint32_t mid, uint32_t ch, uint32_t data)
{
  fTCB.WriteDACOFF(mid, ch, data);
}

uint32_t CupMiniTCB::ReadDACOFF(uint32_t mid, uint32_t ch) { return fTCB.ReadDACOFF(mid, ch); }

void CupMiniTCB::MeasurePED(uint32_t mid, uint32_t ch) { fTCB.MeasurePED(mid, ch); }

uint32_t CupMiniTCB::ReadPED(uint32_t mid, uint32_t ch) { return fTCB.ReadPED(mid, ch); }

void CupMiniTCB::WriteDLY(uint32_t mid, uint32_t ch, uint32_t data) { fTCB.WriteDLY(mid, ch, data); }

uint32_t CupMiniTCB::ReadDLY(uint32_t mid, uint32_t ch) { return fTCB.ReadDLY(mid, ch); }

void CupMiniTCB::WriteTHR(uint32_t mid, uint32_t ch, uint32_t data) { fTCB.WriteTHR(mid, ch, data); }

uint32_t CupMiniTCB::ReadTHR(uint32_t mid, uint32_t ch) { return fTCB.ReadTHR(mid, ch); }

void CupMiniTCB::WritePOL(uint32_t mid, uint32_t ch, uint32_t data) { fTCB.WritePOL(mid, ch, data); }

uint32_t CupMiniTCB::ReadPOL(uint32_t mid, uint32_t ch) { return fTCB.ReadPOL(mid, ch); }

void CupMiniTCB::WritePSW(uint32_t mid, uint32_t ch, uint32_t data) { fTCB.WritePSW(mid, ch, data); }

uint32_t CupMiniTCB::ReadPSW(uint32_t mid, uint32_t ch) { return fTCB.ReadPSW(mid, ch); }

void CupMiniTCB::WriteAMODE(uint32_t mid, uint32_t ch, uint32_t data) { fTCB.WriteAMODE(mid, ch, data); }

uint32_t CupMiniTCB::ReadAMODE(uint32_t mid, uint32_t ch) { return fTCB.ReadAMODE(mid, ch); }

void CupMiniTCB::WritePCT(uint32_t mid, uint32_t ch, uint32_t data) { fTCB.WritePCT(mid, ch, data); }

uint32_t CupMiniTCB::ReadPCT(uint32_t mid, uint32_t ch) { return fTCB.ReadPCT(mid, ch); }

void CupMiniTCB::WritePCI(uint32_t mid, uint32_t ch, uint32_t data) { fTCB.WritePCI(mid, ch, data); }

uint32_t CupMiniTCB::ReadPCI(uint32_t mid, uint32_t ch) { return fTCB.ReadPCI(mid, ch); }

void CupMiniTCB::WritePWT(uint32_t mid, uint32_t ch, uint32_t data) { fTCB.WritePWT(mid, ch, data); }

uint32_t CupMiniTCB::ReadPWT(uint32_t mid, uint32_t ch) { return fTCB.ReadPWT(mid, ch); }

void CupMiniTCB::WriteDT(uint32_t mid, uint32_t ch, uint32_t data) { fTCB.WriteDT(mid, ch, data); }

uint32_t CupMiniTCB::ReadDT(uint32_t mid, uint32_t ch) { return fTCB.ReadDT(mid, ch); }

void CupMiniTCB::WriteTM(uint32_t mid, uint32_t ch, uint32_t data) { fTCB.WriteTM(mid, ch, data); }

uint32_t CupMiniTCB::ReadTM(uint32_t mid, uint32_t ch) { return fTCB.ReadTM(mid, ch); }

void CupMiniTCB::WriteTLT(uint32_t mid, uint32_t data) { fTCB.WriteTLT(mid, data); }

uint32_t CupMiniTCB::ReadTLT(uint32_t mid) { return fTCB.ReadTLT(mid); }

void CupMiniTCB::WriteSTLT(uint32_t mid, uint32_t ch, uint32_t data) { fTCB.WriteSTLT(mid, ch, data); }

uint32_t CupMiniTCB::ReadSTLT(uint32_t mid, uint32_t ch) { return fTCB.ReadSTLT(mid, ch); }

void CupMiniTCB::WriteDSR(uint32_t mid, uint32_t data) { fTCB.WriteDSR(mid, data); }

uint32_t CupMiniTCB::ReadDSR(uint32_t mid) { return fTCB.ReadDSR(mid); }

void CupMiniTCB::AlignFADC(uint32_t mid) { fTCB.AlignFADC500(mid); }

void CupMiniTCB::WritePSS(uint32_t mid, uint32_t ch, uint32_t data) { fTCB.WritePSS(mid, ch, data); }

uint32_t CupMiniTCB::ReadPSS(uint32_t mid, uint32_t ch) { return fTCB.ReadPSS(mid, ch); }

void CupMiniTCB::AlignSADC(uint32_t mid) { fTCB.AlignSADC64(mid); }

void CupMiniTCB::AlignGADC(uint32_t mid) {}

void CupMiniTCB::WritePTRIG(uint32_t data) { fTCB.WritePTRIG(data); }

uint32_t CupMiniTCB::ReadPTRIG() { return fTCB.ReadPTRIG(); }

void CupMiniTCB::WriteTRIGENABLE(uint32_t mid, uint32_t data) { fTCB.WriteTRIGENABLE(mid, data); }

uint32_t CupMiniTCB::ReadTRIGENABLE(uint32_t mid) { return fTCB.ReadTRIGENABLE(mid); }

void CupMiniTCB::WriteMTHRFADC(uint32_t data) { fTCB.WriteMTHR(data); }

uint32_t CupMiniTCB::ReadMTHRFADC() { return fTCB.ReadMTHR(); }

void CupMiniTCB::WritePSCALEFADC(uint32_t data) { fTCB.WritePSCALE(data); }

uint32_t CupMiniTCB::ReadPSCALEFADC() { return fTCB.ReadPSCALE(); }

void CupMiniTCB::SendTRIG() { fTCB.SendTRIG(); }

void CupMiniTCB::ReadLNSTAT(uint32_t * data) { data[0] = fTCB.ReadLNSTAT(); }

void CupMiniTCB::ReadMIDS(uint32_t * data)
{
  for (int i = 1; i <= 4; i++) {
    data[i - 1] = fTCB.ReadMIDS(static_cast<uint32_t>(i));
  }
}

void CupMiniTCB::AlignDRAM(uint32_t mid) { fTCB.AlignDRAM(mid); }
