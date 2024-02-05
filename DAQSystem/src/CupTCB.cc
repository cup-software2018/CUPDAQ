#include <iostream>
#include <unistd.h>

#include "DAQSystem/CupTCB.hh"
#include "Notice/NoticeTCB.hh"

using namespace std;

ClassImp(CupTCB)

    CupTCB * CupTCB::fTCB = nullptr;

CupTCB::CupTCB()
    : AbsTCB()
{
  fSID = 0;
  fMutex = new mutex();
}

CupTCB::~CupTCB() { delete fMutex; }

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

int CupTCB::Open()
{
  fUSB = new usb3tcbroot;
  fUSB->USB3TCBInit(nullptr);

  return TCBopen(fSID, NULL);
}

void CupTCB::Close()
{
  TriggerStop();
  Reset();

  TCBclose(fSID);

  //
  // fixme! corrupted double-linked list occurred
  // fUSB->USB3TCBExit(0);
  // delete fUSB;
}

// tcb
void CupTCB::Reset() { TCBreset(fSID); }

void CupTCB::ResetTIMER() { TCBresetTIMER(fSID); }

void CupTCB::TriggerStart()
{
  Reset();
  ResetTIMER();

  TCBstart(fSID);
}

void CupTCB::TriggerStop()
{
  std::lock_guard<std::mutex> lock(*fMutex);
  TCBstop(fSID);
  Reset();
}

void CupTCB::WriteCW(unsigned long mid, unsigned long ch, unsigned long data)
{
  TCBwrite_CW(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadCW(unsigned long mid, unsigned long ch)
{
  return TCBread_CW(fSID, mid, ch);
}

// m64adc only
void CupTCB::WriteGW(unsigned long mid, unsigned long data)
{
  TCBwrite_GW(fSID, mid, data);
}

unsigned long CupTCB::ReadGW(unsigned long mid)
{
  return TCBread_GW(fSID, mid);
}

// fadc, sadc, amoreadc
void CupTCB::WriteRL(unsigned long mid, unsigned long data)
{
  TCBwrite_RL(fSID, mid, data);
}

unsigned long CupTCB::ReadRL(unsigned long mid)
{
  return TCBread_RL(fSID, mid);
}

void CupTCB::WriteDRAMON(unsigned long mid, unsigned long data)
{
  TCBwrite_DRAMON(fSID, mid, data);
}

unsigned long CupTCB::ReadDRAMON(unsigned long mid)
{
  return TCBread_DRAMON(fSID, mid);
}

void CupTCB::WriteDACOFF(unsigned long mid, unsigned long ch,
                         unsigned long data)
{
  TCBwrite_DACOFF(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadDACOFF(unsigned long mid, unsigned long ch)
{
  return TCBread_DACOFF(fSID, mid, ch);
}

void CupTCB::MeasurePED(unsigned long mid, unsigned long ch)
{
  return TCBmeasure_PED(fSID, mid, ch);
}

unsigned long CupTCB::ReadPED(unsigned long mid, unsigned long ch)
{
  return TCBread_PED(fSID, mid, ch);
}

void CupTCB::WriteDLY(unsigned long mid, unsigned long ch, unsigned long data)
{
  TCBwrite_DLY(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadDLY(unsigned long mid, unsigned long ch)
{
  return TCBread_DLY(fSID, mid, ch);
}

void CupTCB::WriteTHR(unsigned long mid, unsigned long ch, unsigned long data)
{
  TCBwrite_THR(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadTHR(unsigned long mid, unsigned long ch)
{
  return TCBread_THR(fSID, mid, ch);
}

// fadc, sadc
void CupTCB::WritePOL(unsigned long mid, unsigned long ch, unsigned long data)
{
  TCBwrite_POL(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadPOL(unsigned long mid, unsigned long ch)
{
  return TCBread_POL(fSID, mid, ch);
}

void CupTCB::WritePSW(unsigned long mid, unsigned long ch, unsigned long data)
{
  TCBwrite_PSW(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadPSW(unsigned long mid, unsigned long ch)
{
  return TCBread_PSW(fSID, mid, ch);
}

// fadc
void CupTCB::WriteAMODE(unsigned long mid, unsigned long ch, unsigned long data)
{
  TCBwrite_AMODE(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadAMODE(unsigned long mid, unsigned long ch)
{
  return TCBread_AMODE(fSID, mid, ch);
}

void CupTCB::WritePCT(unsigned long mid, unsigned long ch, unsigned long data)
{
  TCBwrite_PCT(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadPCT(unsigned long mid, unsigned long ch)
{
  return TCBread_PCT(fSID, mid, ch);
}

void CupTCB::WritePCI(unsigned long mid, unsigned long ch, unsigned long data)
{
  TCBwrite_PCI(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadPCI(unsigned long mid, unsigned long ch)
{
  return TCBread_PCI(fSID, mid, ch);
}

void CupTCB::WritePWT(unsigned long mid, unsigned long ch, unsigned long data)
{
  TCBwrite_PWT(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadPWT(unsigned long mid, unsigned long ch)
{
  return TCBread_PWT(fSID, mid, ch);
}

void CupTCB::WriteDT(unsigned long mid, unsigned long ch, unsigned long data)
{
  TCBwrite_DT(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadDT(unsigned long mid, unsigned long ch)
{
  return TCBread_DT(fSID, mid, ch);
}

void CupTCB::WriteTM(unsigned long mid, unsigned long ch, unsigned long data)
{
  TCBwrite_TM(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadTM(unsigned long mid, unsigned long ch)
{
  return TCBread_TM(fSID, mid, ch);
}

void CupTCB::WriteTLT(unsigned long mid, unsigned long data)
{
  TCBwrite_TLT(fSID, mid, data);
}

unsigned long CupTCB::ReadTLT(unsigned long mid)
{
  return TCBread_TLT(fSID, mid);
}

void CupTCB::WriteSTLT(unsigned long mid, unsigned long ch, unsigned long data)
{
  TCBwrite_STLT(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadSTLT(unsigned long mid, unsigned long ch)
{
  return TCBread_STLT(fSID, mid, ch);
}

void CupTCB::WriteDSR(unsigned long mid, unsigned long data)
{
  TCBwrite_DSR(fSID, mid, data);
}

unsigned long CupTCB::ReadDSR(unsigned long mid)
{
  return TCBread_DSR(fSID, mid);
}

void CupTCB::AlignFADC(unsigned long mid) { TCB_ADCALIGN_500(fSID, mid); }

void CupTCB::WritePSS(unsigned long mid, unsigned long ch, unsigned long data)
{
  TCBwrite_PSS(fSID, mid, ch, data);
}

unsigned long CupTCB::ReadPSS(unsigned long mid, unsigned long ch)
{
  return TCBread_PSS(fSID, mid, ch);
}

void CupTCB::AlignSADC(unsigned long mid) { TCB_ADCALIGN_64(fSID, mid); }

void CupTCB::AlignGADC(unsigned long mid) { TCB_ADCALIGN_125(fSID, mid); }

// iadc
void CupTCB::WriteDAQMODE(unsigned long mid, unsigned long data)
{
  TCBwrite_DAQMODE(fSID, mid, data);
}

unsigned long CupTCB::ReadDAQMODE(unsigned long mid)
{
  return TCBread_DAQMODE(fSID, mid);
}

void CupTCB::WriteHV(unsigned long mid, unsigned long ch, float data)
{
  TCBwrite_HV(fSID, mid, ch, data);
}

float CupTCB::ReadHV(unsigned long mid, unsigned long ch)
{
  return TCBread_HV(fSID, mid, ch);
}

float CupTCB::ReadTEMP(unsigned long mid, unsigned long ch)
{
  return TCBread_TEMP(fSID, mid, ch);
}

void CupTCB::AlignIADC(unsigned long mid) { TCB_ADCALIGN_MUONDAQ(fSID, mid); }

void CupTCB::WritePTRIG(unsigned long data) { TCBwrite_PTRIG(fSID, data); }

unsigned long CupTCB::ReadPTRIG() { return TCBread_PTRIG(fSID); }

void CupTCB::WriteTRIGENABLE(unsigned long mid, unsigned long data)
{
  TCBwrite_TRIGENABLE(fSID, mid, data);
}

unsigned long CupTCB::ReadTRIGENABLE(unsigned long mid)
{
  return TCBread_TRIGENABLE(fSID, mid);
}

void CupTCB::WriteMTHRFADC(unsigned long data)
{
  TCBwrite_MTHR_NKFADC500(fSID, data);
}

unsigned long CupTCB::ReadMTHRFADC() { return TCBread_MTHR_NKFADC500(fSID); }

void CupTCB::WritePSCALEFADC(unsigned long data)
{
  TCBwrite_PSCALE_NKFADC500(fSID, data);
}

unsigned long CupTCB::ReadPSCALEFADC()
{
  return TCBread_PSCALE_NKFADC500(fSID);
}

void CupTCB::WriteMTHRSADCMU(unsigned long data)
{
  TCBwrite_MTHR_M64ADC_MUON(fSID, data);
}

unsigned long CupTCB::ReadMTHRSADCMU()
{
  return TCBread_MTHR_M64ADC_MUON(fSID);
}

void CupTCB::WritePSCALESADCMU(unsigned long data)
{
  TCBwrite_PSCALE_M64ADC_MUON(fSID, data);
}

unsigned long CupTCB::ReadPSCALESADCMU()
{
  return TCBread_PSCALE_M64ADC_MUON(fSID);
}

void CupTCB::WriteMTHRSADCLS(unsigned long data)
{
  TCBwrite_MTHR_M64ADC_LS(fSID, data);
}

unsigned long CupTCB::ReadMTHRSADCLS() { return TCBread_MTHR_M64ADC_LS(fSID); }

void CupTCB::WritePSCALESADCLS(unsigned long data)
{
  TCBwrite_PSCALE_M64ADC_LS(fSID, data);
}

unsigned long CupTCB::ReadPSCALESADCLS()
{
  return TCBread_PSCALE_M64ADC_LS(fSID);
}

void CupTCB::WriteMTHRIADC(unsigned long data)
{
  TCBwrite_MTHR_MUONDAQ(fSID, data);
}

unsigned long CupTCB::ReadMTHRIADC() { return TCBread_MTHR_MUONDAQ(fSID); }

void CupTCB::WritePSCALEIADC(unsigned long data)
{
  TCBwrite_PSCALE_MUONDAQ(fSID, data);
}

unsigned long CupTCB::ReadPSCALEIADC() { return TCBread_PSCALE_MUONDAQ(fSID); }

void CupTCB::WriteEXTOUT(unsigned long data) { TCBwrite_EXTOUT(fSID, data); }

unsigned long CupTCB::ReadEXTOUT() { return TCBread_EXTOUT(fSID); }

void CupTCB::SendTRIG() { TCBsend_TRIG(fSID); }

void CupTCB::ReadLNSTAT(unsigned long * data) { TCBread_LNSTAT(fSID, data); }

void CupTCB::ReadMIDS(unsigned long * data) { TCBread_MIDS(fSID, data); }

unsigned long CupTCB::ReadBCOUNT(unsigned long mid)
{
  std::lock_guard<std::mutex> lock(*fMutex);
  return TCBread_BCOUNT(fSID, mid);
}

int CupTCB::ReadDATA(unsigned long mid, unsigned long bcount,
                     unsigned char * data)
{
  std::lock_guard<std::mutex> lock(*fMutex);
  return TCBread_DATA(fSID, mid, bcount, data);
}

void CupTCB::WriteGATEDLY(unsigned long data) { TCBwrite_GATEDLY(fSID, data); }
unsigned long CupTCB::ReadGATEDLY() { return TCBread_GATEDLY(fSID); }

void CupTCB::WriteGATEWIDTH(unsigned long data)
{
  TCBwrite_GATEWIDTH(fSID, data);
}
unsigned long CupTCB::ReadGATEWIDTH() { return TCBread_GATEWIDTH(fSID); }

void CupTCB::WriteEXTOUTWIDTH(unsigned long data)
{
  TCBwrite_EXTOUTWIDTH(fSID, data);
}
unsigned long CupTCB::ReadEXTOUTWIDTH() { return TCBread_EXTOUTWIDTH(fSID); }

void CupTCB::AlignDRAM(unsigned long mid) { TCB_ADCALIGN_DRAM(fSID, mid); }

void CupTCB::WriteTRGSWFADC(unsigned long fadc, unsigned long sadcmu,
                            unsigned long sadcls, unsigned long iadc)
{
  TCBwrite_TRIG_SWITCH_NKFADC500(fSID, fadc, sadcmu, sadcls, iadc);
}
void CupTCB::WriteTRGSWSADCMU(unsigned long fadc, unsigned long sadcmu,
                              unsigned long sadcls, unsigned long iadc)
{
  TCBwrite_TRIG_SWITCH_M64ADC_MUON(fSID, fadc, sadcmu, sadcls, iadc);
}
void CupTCB::WriteTRGSWSADCLS(unsigned long fadc, unsigned long sadcmu,
                              unsigned long sadcls, unsigned long iadc)
{
  TCBwrite_TRIG_SWITCH_M64ADC_LS(fSID, fadc, sadcmu, sadcls, iadc);
}
void CupTCB::WriteTRGSWIADC(unsigned long fadc, unsigned long sadcmu,
                            unsigned long sadcls, unsigned long iadc)
{
  TCBwrite_TRIG_SWITCH_MUONDAQ(fSID, fadc, sadcmu, sadcls, iadc);
}
unsigned long CupTCB::ReadTRGSWFADC() { return TCBread_SWITCH_NKFADC500(fSID); }
unsigned long CupTCB::ReadTRGSWSADCMU()
{
  return TCBread_SWITCH_M64ADC_MUON(fSID);
}
unsigned long CupTCB::ReadTRGSWSADCLS()
{
  return TCBread_SWITCH_M64ADC_LS(fSID);
}
unsigned long CupTCB::ReadTRGSWIADC() { return TCBread_SWITCH_MUONDAQ(fSID); }
