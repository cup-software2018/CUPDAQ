#ifndef CupTCB_HH
#define CupTCB_HH

#include <mutex>

#include "Notice/usb3tcbroot.hh"

#include "DAQConfig/AmoreADCConf.hh"
#include "DAQConfig/FADCTConf.hh"
#include "DAQConfig/SADCTConf.hh"
#include "DAQConfig/TCBConf.hh"

#include "DAQSystem/AbsTCB.hh"

class CupTCB : public AbsTCB {
public:
  CupTCB();
  virtual ~CupTCB();

  static CupTCB * Instance();
  static void DeleteInstance();

  virtual int Open();
  virtual void Close();

  // tcb
  virtual void Reset();
  virtual void ResetTIMER();
  virtual void TriggerStart();
  virtual void TriggerStop();
  virtual unsigned long ReadRUN(unsigned long mid);
  virtual unsigned long ReadBCOUNT(unsigned long mid);
  virtual int ReadDATA(unsigned long mid, unsigned long bcount,
                       unsigned char * data);

  // tcb, fadc, sadc, amoreadc
  virtual void WriteLT(unsigned long mid, char * data, int len);
  virtual void WriteCW(unsigned long mid, unsigned long ch, unsigned long data);
  virtual unsigned long ReadCW(unsigned long mid, unsigned long ch);

  // m64adc only
  virtual void WriteGW(unsigned long mid, unsigned long data);
  virtual unsigned long ReadGW(unsigned long mid);

  // fadc, sadc, amoreadc
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
  virtual void WriteAMOREDLY(unsigned long mid, unsigned long ch,
                             unsigned long data);
  virtual unsigned long ReadAMOREDLY(unsigned long mid, unsigned long ch);
  virtual void WriteTHR(unsigned long mid, unsigned long ch,
                        unsigned long data);
  virtual unsigned long ReadTHR(unsigned long mid, unsigned long ch);

  // fadc, sadc
  virtual void WritePOL(unsigned long mid, unsigned long ch,
                        unsigned long data);
  virtual unsigned long ReadPOL(unsigned long mid, unsigned long ch);
  virtual void WritePSW(unsigned long mid, unsigned long ch,
                        unsigned long data);
  virtual unsigned long ReadPSW(unsigned long mid, unsigned long ch);

  // fadc
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
  virtual void WriteTM(unsigned long mid, unsigned long ch, unsigned long data);
  virtual unsigned long ReadTM(unsigned long mid, unsigned long ch);
  virtual void WriteTLT(unsigned long mid, unsigned long data);
  virtual unsigned long ReadTLT(unsigned long mid);
  virtual void WriteSTLT(unsigned long mid, unsigned long ch,
                         unsigned long data);
  virtual unsigned long ReadSTLT(unsigned long mid, unsigned long ch);
  virtual void WriteZEROSUP(unsigned long mid, unsigned long ch,
                            unsigned long data);
  virtual unsigned long ReadZEROSUP(unsigned long mid, unsigned long ch);
  virtual void WriteZSFD(unsigned long mid, unsigned long data);
  virtual unsigned long ReadZSFD(unsigned long mid);
  virtual void WriteDSR(unsigned long mid, unsigned long data);
  virtual unsigned long ReadDSR(unsigned long mid);
  virtual void AlignFADC(unsigned long mid);

  // sadc
  virtual void WriteFMUX(unsigned long mid, unsigned long ch);
  virtual unsigned long ReadFMUX(unsigned long mid);
  virtual void WritePSS(unsigned long mid, unsigned long ch,
                        unsigned long data);
  virtual unsigned long ReadPSS(unsigned long mid, unsigned long ch);
  virtual void ArmFADC(unsigned long mid);
  virtual unsigned long ReadFREADY(unsigned long mid);
  virtual void AlignSADC(unsigned long mid);

  // gadc
  virtual void AlignGADC(unsigned long mid);

  // iadc
  virtual void WriteDAQMODE(unsigned long mid, unsigned long data);
  virtual unsigned long ReadDAQMODE(unsigned long mid);
  virtual void WriteHV(unsigned long mid, unsigned long ch, float data);
  virtual float ReadHV(unsigned long mid, unsigned long ch);
  virtual float ReadTEMP(unsigned long mid, unsigned long ch);
  virtual void AlignIADC(unsigned long mid);

  // amoreadc
  virtual void WriteRT(unsigned long mid, unsigned long ch, unsigned long data);
  virtual unsigned long ReadRT(unsigned long mid, unsigned long ch);
  virtual void WriteST(unsigned long mid, unsigned long ch, unsigned long data);
  virtual unsigned long ReadST(unsigned long mid, unsigned long ch);
  virtual void WritePT(unsigned long mid, unsigned long ch, unsigned long data);
  virtual unsigned long ReadPT(unsigned long mid, unsigned long ch);
  virtual void WriteSR(unsigned long mid, unsigned long ch, unsigned long data);
  virtual unsigned long ReadSR(unsigned long mid, unsigned long ch);
  virtual void WriteDACGAIN(unsigned long mid, unsigned long ch,
                            unsigned long data);
  virtual unsigned long ReadDACGAIN(unsigned long mid, unsigned long ch);

  // tcb
  virtual void WriteRUNNO(unsigned long data);
  virtual unsigned long ReadRUNNO();
  // virtual void WriteTRIGDLY(unsigned long data);
  // virtual unsigned long ReadTRIGDLY();
  virtual void WritePTRIG(unsigned long data);
  virtual unsigned long ReadPTRIG();
  virtual void WriteTRIGENABLE(unsigned long mid, unsigned long data);
  virtual unsigned long ReadTRIGENABLE(unsigned long mid);
  virtual void WriteMTHRFADC(unsigned long data);
  virtual unsigned long ReadMTHRFADC();
  virtual void WritePSCALEFADC(unsigned long data);
  virtual unsigned long ReadPSCALEFADC();
  virtual void WriteMTHRSADCMU(unsigned long data);
  virtual unsigned long ReadMTHRSADCMU();
  virtual void WritePSCALESADCMU(unsigned long data);
  virtual unsigned long ReadPSCALESADCMU();
  virtual void WriteMTHRSADCLS(unsigned long data);
  virtual unsigned long ReadMTHRSADCLS();
  virtual void WritePSCALESADCLS(unsigned long data);
  virtual unsigned long ReadPSCALESADCLS();
  virtual void WriteMTHRAMOREADC(unsigned long data);
  virtual unsigned long ReadMTHRAMOREADC();
  virtual void WritePSCALEAMOREADC(unsigned long data);
  virtual unsigned long ReadPSCALEAMOREADC();
  virtual void WriteEXTOUT(unsigned long data);
  virtual unsigned long ReadEXTOUT();
  virtual void WriteGATEDLY(unsigned long data);
  virtual unsigned long ReadGATEDLY();
  virtual void WriteGATEWIDTH(unsigned long data);
  virtual unsigned long ReadGATEWIDTH();
  virtual void WriteEXTOUTWIDTH(unsigned long data);
  virtual unsigned long ReadEXTOUTWIDTH();
  virtual void SendTRIG();
  virtual void ReadLNSTAT(unsigned long * data);
  virtual void ReadMIDS(unsigned long * data);
  virtual void AlignDRAM(unsigned long mid);

protected:
  static CupTCB * fTCB;

  int fSID;
  usb3tcbroot * fUSB;

  std::mutex * fMutex;

  ClassDef(CupTCB, 0)
};

#endif
