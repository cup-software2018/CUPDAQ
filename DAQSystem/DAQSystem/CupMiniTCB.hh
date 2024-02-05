#ifndef CupMiniTCB_HH
#define CupMiniTCB_HH

#include "DAQSystem/AbsTCB.hh"
#include "Notice/usb3tcbroot.hh"

class CupMiniTCB : public AbsTCB {
public:
  CupMiniTCB();
  virtual ~CupMiniTCB();

  void SetIPAddress(const char * ipaddr);

  virtual int Open();
  virtual void Close();

  // tcb
  virtual void Reset();
  virtual void ResetTIMER();
  virtual void TriggerStart();
  virtual void TriggerStop();

  virtual unsigned long ReadBCOUNT(unsigned long mid) { return 0; }
  virtual int ReadDATA(unsigned long mid, unsigned long bcount,
                       unsigned char * data)
  {
    return 0;
  }

  virtual void WriteCW(unsigned long mid, unsigned long ch, unsigned long data);
  virtual unsigned long ReadCW(unsigned long mid, unsigned long ch);
  virtual void WriteGW(unsigned long mid, unsigned long data);
  virtual unsigned long ReadGW(unsigned long mid);
  virtual void WriteRL(unsigned long mid, unsigned long data);
  virtual unsigned long ReadRL(unsigned long mid);
  virtual void WriteDRAMON(unsigned long mid, unsigned long data);
  virtual unsigned long ReadDRAMON(unsigned long mid);
  virtual void WriteDACOFF(unsigned long mid, unsigned long ch,
                           unsigned long data);
  virtual unsigned long ReadDACOFF(unsigned long mid, unsigned long ch);
  virtual void MeasurePED(unsigned long mid, unsigned long ch);
  virtual unsigned long ReadPED(unsigned long mid, unsigned long ch);
  virtual void WriteDLY(unsigned long mid, unsigned long ch,
                        unsigned long data);
  virtual unsigned long ReadDLY(unsigned long mid, unsigned long ch);
  virtual void WriteTHR(unsigned long mid, unsigned long ch,
                        unsigned long data);
  virtual unsigned long ReadTHR(unsigned long mid, unsigned long ch);
  virtual void WritePOL(unsigned long mid, unsigned long ch,
                        unsigned long data);
  virtual unsigned long ReadPOL(unsigned long mid, unsigned long ch);
  virtual void WritePSW(unsigned long mid, unsigned long ch,
                        unsigned long data);
  virtual unsigned long ReadPSW(unsigned long mid, unsigned long ch);
  virtual void WriteAMODE(unsigned long mid, unsigned long ch,
                          unsigned long data);
  virtual unsigned long ReadAMODE(unsigned long mid, unsigned long ch);
  virtual void WritePCT(unsigned long mid, unsigned long ch,
                        unsigned long data);
  virtual unsigned long ReadPCT(unsigned long mid, unsigned long ch);
  virtual void WritePCI(unsigned long mid, unsigned long ch,
                        unsigned long data);
  virtual unsigned long ReadPCI(unsigned long mid, unsigned long ch);
  virtual void WritePWT(unsigned long mid, unsigned long ch,
                        unsigned long data);
  virtual unsigned long ReadPWT(unsigned long mid, unsigned long ch);
  virtual void WriteDT(unsigned long mid, unsigned long ch, unsigned long data);
  virtual unsigned long ReadDT(unsigned long mid, unsigned long ch);
  virtual void WritePSS(unsigned long mid, unsigned long ch,
                        unsigned long data);
  virtual unsigned long ReadPSS(unsigned long mid, unsigned long ch);   
  virtual void WriteTM(unsigned long mid, unsigned long ch, unsigned long data);
  virtual unsigned long ReadTM(unsigned long mid, unsigned long ch);
  virtual void WriteTLT(unsigned long mid, unsigned long data);
  virtual unsigned long ReadTLT(unsigned long mid);
  virtual void WriteSTLT(unsigned long mid, unsigned long ch,
                         unsigned long data);
  virtual unsigned long ReadSTLT(unsigned long mid, unsigned long ch);
  virtual void WriteDSR(unsigned long mid, unsigned long data);
  virtual unsigned long ReadDSR(unsigned long mid);
  virtual void AlignFADC(unsigned long mid);
  virtual void AlignSADC(unsigned long mid);
  virtual void AlignGADC(unsigned long mid);

  // iadc
  virtual void WriteDAQMODE(unsigned long mid, unsigned long data) {}
  virtual unsigned long ReadDAQMODE(unsigned long mid) { return 0; }
  virtual void WriteHV(unsigned long mid, unsigned long ch, float data) {}
  virtual float ReadHV(unsigned long mid, unsigned long ch) { return 0; }
  virtual float ReadTEMP(unsigned long mid, unsigned long ch) { return 0; }
  virtual void AlignIADC(unsigned long mid) {}

  // tcb
  virtual void WritePTRIG(unsigned long data);
  virtual unsigned long ReadPTRIG();
  virtual void WriteTRIGENABLE(unsigned long mid, unsigned long data);
  virtual unsigned long ReadTRIGENABLE(unsigned long mid);
  virtual void WriteMTHRFADC(unsigned long data);
  virtual unsigned long ReadMTHRFADC();
  virtual void WritePSCALEFADC(unsigned long data);
  virtual unsigned long ReadPSCALEFADC();
  virtual void WriteMTHRSADCMU(unsigned long data) {}
  virtual unsigned long ReadMTHRSADCMU() { return 0; }
  virtual void WritePSCALESADCMU(unsigned long data) {}
  virtual unsigned long ReadPSCALESADCMU() { return 0; }
  virtual void WriteMTHRSADCLS(unsigned long data) {}
  virtual unsigned long ReadMTHRSADCLS() { return 0; }
  virtual void WritePSCALESADCLS(unsigned long data) {}
  virtual unsigned long ReadPSCALESADCLS() { return 0; }
  virtual void WriteMTHRIADC(unsigned long data) {}
  virtual unsigned long ReadMTHRIADC() { return 0; }
  virtual void WritePSCALEIADC(unsigned long data) {}
  virtual unsigned long ReadPSCALEIADC() { return 0; }
  virtual void WriteEXTOUT(unsigned long data) {}
  virtual unsigned long ReadEXTOUT() { return 0; }
  virtual void WriteGATEDLY(unsigned long data) {}
  virtual unsigned long ReadGATEDLY() { return 0; }
  virtual void WriteGATEWIDTH(unsigned long data) {}
  virtual unsigned long ReadGATEWIDTH() { return 0; }
  virtual void WriteEXTOUTWIDTH(unsigned long data) {}
  virtual unsigned long ReadEXTOUTWIDTH() { return 0; }
  virtual void SendTRIG();
  virtual void ReadLNSTAT(unsigned long * data);
  virtual void ReadMIDS(unsigned long * data);
  virtual void AlignDRAM(unsigned long mid);

  virtual void WriteTRGSWFADC(unsigned long fadc, unsigned long sadcmu,
                              unsigned long sadcls, unsigned long iadc)
  {
  }
  virtual void WriteTRGSWSADCMU(unsigned long fadc, unsigned long sadcmu,
                                unsigned long sadcls, unsigned long iadc)
  {
  }
  virtual void WriteTRGSWSADCLS(unsigned long fadc, unsigned long sadcmu,
                                unsigned long sadcls, unsigned long iadc)
  {
  }
  virtual void WriteTRGSWIADC(unsigned long fadc, unsigned long sadcmu,
                              unsigned long sadcls, unsigned long iadc)
  {
  }
  virtual unsigned long ReadTRGSWFADC() { return 0; }
  virtual unsigned long ReadTRGSWSADCMU() { return 0; }
  virtual unsigned long ReadTRGSWSADCLS() { return 0; }
  virtual unsigned long ReadTRGSWIADC() { return 0; }

protected:
  int fTCPHandle;
  TString fIPAddress;

  ClassDef(CupMiniTCB, 0)
};

inline void
CupMiniTCB::SetIPAddress(const char * ipaddr)
{
  fIPAddress = ipaddr;
}

#endif
