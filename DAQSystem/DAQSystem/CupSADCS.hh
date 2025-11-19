#ifndef CupSADCS_HH
#define CupSADCS_HH

#include "DAQConfig/AbsConf.hh"
#include "DAQSystem/AbsADC.hh"

class CupSADCS : public AbsADC {
public:
  CupSADCS();
  CupSADCS(int sid);
  CupSADCS(AbsConf * conf);
  ~CupSADCS() override = default;

  int Open() override;
  void Close() override;

  bool Configure() override;
  bool Initialize() override;
  void StartTrigger() override;
  void StopTrigger() override;

  int ReadBCount() override;
  int ReadData(int count, unsigned char * data) override;
  int ReadData(int count) override;

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

private:
  void UpdateTriggerAndTime(const unsigned char * tempdata);

  ClassDef(CupSADCS, 0)
};

#endif
