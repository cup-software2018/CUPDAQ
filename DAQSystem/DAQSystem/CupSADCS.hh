#ifndef CupSADCS_HH
#define CupSADCS_HH

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"

class CupSADCS : public AbsADC {
public:
  CupSADCS();
  CupSADCS(int sid);
  CupSADCS(AbsConf * conf);
  virtual ~CupSADCS();

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

  void WriteCW(unsigned long data);
  unsigned long ReadCW();
  void WriteDRAMON(unsigned long data);
  unsigned long ReadDRAMON();
  unsigned long ReadPED(unsigned long ch);
  void WriteDLY(unsigned long ch, unsigned long data);
  unsigned long ReadDLY(unsigned long ch);
  void WriteTHR(unsigned long ch, unsigned long data);
  unsigned long ReadTHR(unsigned long ch);
  void WritePSW(unsigned long ch, unsigned long data);
  unsigned long ReadPSW(unsigned long ch);
  void WritePTRIG(unsigned long data);
  unsigned long ReadPTRIG();
  void WriteTM(unsigned long data);
  unsigned long ReadTM();
  void WriteMTHR(unsigned long data);
  unsigned long ReadMTHR();
  void SendTRIG();
  void SendADCRST();
  void SendADCCAL();
  void WriteADCDLY(unsigned long ch, unsigned long data);
  void WriteADCALIGN(unsigned long data);
  unsigned long ReadADCSTAT();
  void WriteBITSLIP(unsigned long ch, unsigned long data);
  void WriteFMUX(unsigned long ch);
  unsigned long ReadFMUX();
  void ArmFADC();
  unsigned long ReadFREADY();
  void ALIGNSADCS();
  void ReadFADCBUF(unsigned long * data);

  ClassDef(CupSADCS, 0)
};

#endif
