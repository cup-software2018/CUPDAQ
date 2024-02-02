#ifndef CupGADCS_HH
#define CupGADCS_HH

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"

class CupGADCS : public AbsADC {
public:
  CupGADCS();
  CupGADCS(int sid);
  CupGADCS(AbsConf * config);
  virtual ~CupGADCS();

  virtual int Open();
  virtual void Close();

  virtual bool Configure();
  virtual bool Initialize();
  virtual void StartTrigger();
  virtual void StopTrigger();

  virtual int ReadBCount();
  virtual int ReadData(int count, unsigned char * data);
  virtual int ReadData(int count);

  void Reset();
  void ResetTIMER();

  void WriteCW(unsigned long ch, unsigned long data);
  unsigned long ReadCW(unsigned long ch);
  void WriteRL(unsigned long data);
  unsigned long ReadRL();
  void WriteDACOFF(unsigned long ch, unsigned long data);
  unsigned long ReadDACOFF(unsigned long ch);
  void MeasurePED(unsigned long ch);
  unsigned long ReadPED(unsigned long ch);
  void WriteDLY(unsigned long ch, unsigned long data);
  unsigned long ReadDLY(unsigned long ch);
  void WriteTHR(unsigned long ch, unsigned long data);
  unsigned long ReadTHR(unsigned long ch);
  void WritePOL(unsigned long ch, unsigned long data);
  unsigned long ReadPOL(unsigned long ch);
  void WritePSW(unsigned long ch, unsigned long data);
  unsigned long ReadPSW(unsigned long ch);
  //  void WriteAMODE(unsigned long ch, unsigned long data);
  //  unsigned long ReadAMODE(unsigned long ch);
  void WritePCT(unsigned long ch, unsigned long data);
  unsigned long ReadPCT(unsigned long ch);
  void WritePCI(unsigned long ch, unsigned long data);
  unsigned long ReadPCI(unsigned long ch);
  void WritePWT(unsigned long ch, unsigned long data);
  unsigned long ReadPWT(unsigned long ch);
  void WriteDT(unsigned long ch, unsigned long data);
  unsigned long ReadDT(unsigned long ch);
  void WritePTRIG(unsigned long data);
  unsigned long ReadPTRIG();
  void WriteTRIGENABLE(unsigned long data);
  unsigned long ReadTRIGENABLE();
  void WriteTM(unsigned long ch, unsigned long data);
  unsigned long ReadTM(unsigned long ch);
  void WriteTLT(unsigned long data);
  unsigned long ReadTLT();
  void WriteZEROSUP(unsigned long ch, unsigned long data);
  unsigned long ReadZEROSUP(unsigned long ch);
  // void WritePSCALE(unsigned long data);
  // unsigned long ReadPSCALE();
  void WriteDSR(unsigned long data);
  unsigned long ReadDSR();
  void SendTRIG();

  ClassDef(CupGADCS, 0)
};

#endif
