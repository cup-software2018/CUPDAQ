#include <iostream>
#include <unistd.h>

#include "DAQSystem/CupMiniTCB.hh"
#include "Notice/NoticeMINITCB.hh"

using namespace std;

ClassImp(CupMiniTCB)

CupMiniTCB::CupMiniTCB()
    : AbsTCB()
{
  fTCPHandle = 0;
}

CupMiniTCB::~CupMiniTCB() {}

int CupMiniTCB::Open()
{
  fTCPHandle = MINITCBopen((char *)fIPAddress.Data());

  return fTCPHandle;
}

void CupMiniTCB::Close()
{
  TriggerStop();
  Reset();

  MINITCBclose(fTCPHandle);
}

// tcb
void CupMiniTCB::Reset() { MINITCBreset(fTCPHandle); }

void CupMiniTCB::ResetTIMER() { MINITCBresetTIMER(fTCPHandle); }

void CupMiniTCB::TriggerStart() { MINITCBstart(fTCPHandle); }

void CupMiniTCB::TriggerStop() { MINITCBstop(fTCPHandle); }

unsigned long CupMiniTCB::ReadRUN(unsigned long mid)
{
  return MINITCBread_RUN(fTCPHandle, mid);
}

// tcb, fadc, sadc, amoreadc
void CupMiniTCB::WriteCW(unsigned long mid, unsigned long ch,
                         unsigned long data)
{
  MINITCBwrite_CW(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadCW(unsigned long mid, unsigned long ch)
{
  return MINITCBread_CW(fTCPHandle, mid, ch);
}

// m64adc only
void CupMiniTCB::WriteGW(unsigned long mid, unsigned long data)
{
  MINITCBwrite_GW(fTCPHandle, mid, 0, data);
}

unsigned long CupMiniTCB::ReadGW(unsigned long mid)
{
  return MINITCBread_GW(fTCPHandle, mid, 0);
}

// fadc, sadc, amoreadc
void CupMiniTCB::WriteRL(unsigned long mid, unsigned long data)
{
  MINITCBwrite_RL(fTCPHandle, mid, data);
}

unsigned long CupMiniTCB::ReadRL(unsigned long mid)
{
  return MINITCBread_RL(fTCPHandle, mid);
}

void CupMiniTCB::WriteDRAMON(unsigned long mid, unsigned long data)
{
  MINITCBwrite_DRAMON(fTCPHandle, mid, data);
}

unsigned long CupMiniTCB::ReadDRAMON(unsigned long mid)
{
  return MINITCBread_DRAMON(fTCPHandle, mid);
}

void CupMiniTCB::WriteDACOFF(unsigned long mid, unsigned long ch,
                             unsigned long data)
{
  MINITCBwrite_DACOFF(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadDACOFF(unsigned long mid, unsigned long ch)
{
  return MINITCBread_DACOFF(fTCPHandle, mid, ch);
}

void CupMiniTCB::MeasurePED(unsigned long mid, unsigned long ch)
{
  return MINITCBmeasure_PED(fTCPHandle, mid, ch);
}

unsigned long CupMiniTCB::ReadPED(unsigned long mid, unsigned long ch)
{
  return MINITCBread_PED(fTCPHandle, mid, ch);
}

void CupMiniTCB::WriteDLY(unsigned long mid, unsigned long ch,
                          unsigned long data)
{
  MINITCBwrite_DLY(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadDLY(unsigned long mid, unsigned long ch)
{
  return MINITCBread_DLY(fTCPHandle, mid, ch);
}

void CupMiniTCB::WriteAMOREDLY(unsigned long mid, unsigned long ch,
                               unsigned long data)
{
  MINITCBwrite_AMOREDLY(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadAMOREDLY(unsigned long mid, unsigned long ch)
{
  return MINITCBread_AMOREDLY(fTCPHandle, mid, ch);
}

void CupMiniTCB::WriteTHR(unsigned long mid, unsigned long ch,
                          unsigned long data)
{
  MINITCBwrite_THR(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadTHR(unsigned long mid, unsigned long ch)
{
  return MINITCBread_THR(fTCPHandle, mid, ch);
}

// fadc, sadc
void CupMiniTCB::WritePOL(unsigned long mid, unsigned long ch,
                          unsigned long data)
{
  MINITCBwrite_POL(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadPOL(unsigned long mid, unsigned long ch)
{
  return MINITCBread_POL(fTCPHandle, mid, ch);
}

void CupMiniTCB::WritePSW(unsigned long mid, unsigned long ch,
                          unsigned long data)
{
  MINITCBwrite_PSW(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadPSW(unsigned long mid, unsigned long ch)
{
  return MINITCBread_PSW(fTCPHandle, mid, ch);
}

// fadc
void CupMiniTCB::WriteAMODE(unsigned long mid, unsigned long ch,
                            unsigned long data)
{
  MINITCBwrite_AMODE(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadAMODE(unsigned long mid, unsigned long ch)
{
  return MINITCBread_AMODE(fTCPHandle, mid, ch);
}

void CupMiniTCB::WritePCT(unsigned long mid, unsigned long ch,
                          unsigned long data)
{
  MINITCBwrite_PCT(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadPCT(unsigned long mid, unsigned long ch)
{
  return MINITCBread_PCT(fTCPHandle, mid, ch);
}

void CupMiniTCB::WritePCI(unsigned long mid, unsigned long ch,
                          unsigned long data)
{
  MINITCBwrite_PCI(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadPCI(unsigned long mid, unsigned long ch)
{
  return MINITCBread_PCI(fTCPHandle, mid, ch);
}

void CupMiniTCB::WritePWT(unsigned long mid, unsigned long ch,
                          unsigned long data)
{
  MINITCBwrite_PWT(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadPWT(unsigned long mid, unsigned long ch)
{
  return MINITCBread_PWT(fTCPHandle, mid, ch);
}

void CupMiniTCB::WriteDT(unsigned long mid, unsigned long ch,
                         unsigned long data)
{
  MINITCBwrite_DT(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadDT(unsigned long mid, unsigned long ch)
{
  return MINITCBread_DT(fTCPHandle, mid, ch);
}

void CupMiniTCB::WriteTM(unsigned long mid, unsigned long ch,
                         unsigned long data)
{
  MINITCBwrite_TM(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadTM(unsigned long mid, unsigned long ch)
{
  return MINITCBread_TM(fTCPHandle, mid, ch);
}

void CupMiniTCB::WriteTLT(unsigned long mid, unsigned long data)
{
  MINITCBwrite_TLT(fTCPHandle, mid, data);
}

unsigned long CupMiniTCB::ReadTLT(unsigned long mid)
{
  return MINITCBread_TLT(fTCPHandle, mid);
}

void CupMiniTCB::WriteSTLT(unsigned long mid, unsigned long ch,
                           unsigned long data)
{
  MINITCBwrite_STLT(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadSTLT(unsigned long mid, unsigned long ch)
{
  return MINITCBread_STLT(fTCPHandle, mid, ch);
}

void CupMiniTCB::WriteZEROSUP(unsigned long mid, unsigned long ch,
                              unsigned long data)
{
  MINITCBwrite_ZEROSUP(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadZEROSUP(unsigned long mid, unsigned long ch)
{
  return MINITCBread_ZEROSUP(fTCPHandle, mid, ch);
}

void CupMiniTCB::WriteZSFD(unsigned long mid, unsigned long data)
{
  MINITCBwrite_ZSFD(fTCPHandle, mid, data);
}

unsigned long CupMiniTCB::ReadZSFD(unsigned long mid)
{
  return MINITCBread_ZSFD(fTCPHandle, mid);
}

void CupMiniTCB::WriteDSR(unsigned long mid, unsigned long data)
{
  MINITCBwrite_DSR(fTCPHandle, mid, data);
}

unsigned long CupMiniTCB::ReadDSR(unsigned long mid)
{
  return MINITCBread_DSR(fTCPHandle, mid);
}

void CupMiniTCB::AlignFADC(unsigned long mid)
{
  MINITCB_ADCALIGN(fTCPHandle, mid);
}

// sadc
void CupMiniTCB::WriteFMUX(unsigned long mid, unsigned long ch)
{
  MINITCBwrite_FMUX(fTCPHandle, mid, ch);
}

unsigned long CupMiniTCB::ReadFMUX(unsigned long mid)
{
  return MINITCBread_FMUX(fTCPHandle, mid);
}

void CupMiniTCB::WritePSS(unsigned long mid, unsigned long ch,
                          unsigned long data)
{
  MINITCBwrite_PSS(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadPSS(unsigned long mid, unsigned long ch)
{
  return MINITCBread_PSS(fTCPHandle, mid, ch);
}

void CupMiniTCB::ArmFADC(unsigned long mid)
{
  MINITCBarm_FADC(fTCPHandle, mid);
}

unsigned long CupMiniTCB::ReadFREADY(unsigned long mid)
{
  return MINITCBread_FREADY(fTCPHandle, mid);
}

void CupMiniTCB::AlignSADC(unsigned long mid)
{
  MINITCB_ADCALIGN_64(fTCPHandle, mid);
}

void CupMiniTCB::AlignGADC(unsigned long mid)
{
  // not yet implemented
  // MINITCB_ADCALIGN_125(fTCPHandle, mid);
}

// amoreadc
void CupMiniTCB::WriteRT(unsigned long mid, unsigned long ch,
                         unsigned long data)
{
  MINITCBwrite_RT(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadRT(unsigned long mid, unsigned long ch)
{
  return MINITCBread_RT(fTCPHandle, mid, ch);
}

void CupMiniTCB::WriteST(unsigned long mid, unsigned long ch,
                         unsigned long data)
{
  MINITCBwrite_ST(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadST(unsigned long mid, unsigned long ch)
{
  return MINITCBread_ST(fTCPHandle, mid, ch);
}

void CupMiniTCB::WritePT(unsigned long mid, unsigned long ch,
                         unsigned long data)
{
  MINITCBwrite_PT(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadPT(unsigned long mid, unsigned long ch)
{
  return MINITCBread_PT(fTCPHandle, mid, ch);
}

void CupMiniTCB::WriteSR(unsigned long mid, unsigned long ch,
                         unsigned long data)
{
  MINITCBwrite_SR(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadSR(unsigned long mid, unsigned long ch)
{
  return MINITCBread_SR(fTCPHandle, mid, ch);
}

void CupMiniTCB::WriteDACGAIN(unsigned long mid, unsigned long ch,
                              unsigned long data)
{
  MINITCBwrite_DACGAIN(fTCPHandle, mid, ch, data);
}

unsigned long CupMiniTCB::ReadDACGAIN(unsigned long mid, unsigned long ch)
{
  return MINITCBread_DACGAIN(fTCPHandle, mid, ch);
}

// tcb
void CupMiniTCB::WriteRUNNO(unsigned long data)
{
  MINITCBwrite_RUNNO(fTCPHandle, data);
}

unsigned long CupMiniTCB::ReadRUNNO() { return MINITCBread_RUNNO(fTCPHandle); }

void CupMiniTCB::WritePTRIG(unsigned long data)
{
  MINITCBwrite_PTRIG(fTCPHandle, data);
}

unsigned long CupMiniTCB::ReadPTRIG() { return MINITCBread_PTRIG(fTCPHandle); }

void CupMiniTCB::WriteTRIGENABLE(unsigned long mid, unsigned long data)
{
  MINITCBwrite_TRIGENABLE(fTCPHandle, mid, data);
}

unsigned long CupMiniTCB::ReadTRIGENABLE(unsigned long mid)
{
  return MINITCBread_TRIGENABLE(fTCPHandle, mid);
}

void CupMiniTCB::WriteMTHRFADC(unsigned long data)
{
  MINITCBwrite_MTHR(fTCPHandle, data);
}

unsigned long CupMiniTCB::ReadMTHRFADC()
{
  return MINITCBread_MTHR(fTCPHandle);
}

void CupMiniTCB::WritePSCALEFADC(unsigned long data)
{
  MINITCBwrite_PSCALE(fTCPHandle, data);
}

unsigned long CupMiniTCB::ReadPSCALEFADC()
{
  return MINITCBread_PSCALE(fTCPHandle);
}

void CupMiniTCB::SendTRIG() { MINITCBsend_TRIG(fTCPHandle); }

void CupMiniTCB::ReadLNSTAT(unsigned long * data)
{
  data[0] = MINITCBread_LNSTAT(fTCPHandle);
}

void CupMiniTCB::ReadMIDS(unsigned long * data)
{
  for (int i = 1; i <= 4; i++) {
    data[i - 1] = MINITCBread_MIDS(fTCPHandle, i);
  }
}

void CupMiniTCB::AlignDRAM(unsigned long mid)
{
  MINITCB_ADCALIGN_DRAM(fTCPHandle, mid);
}
