#ifndef TCBConf_hh
#define TCBConf_hh

#include "DAQConfig/AbsConf.hh"

class TCBConf : public AbsConf {
public:
  TCBConf();
  explicit TCBConf(int sid);
  ~TCBConf() override = default;

  void SetTM(int val);
  void SetCW(int val);
  void SetDLY(int val);
  void SetPTRG(int val);

  void SetMTHRF(int val);
  void SetPSCF(int val);
  void SetDTF(int val);
  void SetSWF(int f, int sm, int sl, int i);

  void SetMTHRSM(int val);
  void SetPSCSM(int val);
  void SetDTSM(int val);
  void SetSWSM(int f, int sm, int sl, int i);

  void SetMTHRSL(int val);
  void SetPSCSL(int val);
  void SetDTSL(int val);
  void SetSWSL(int f, int sm, int sl, int i);

  void SetMTHRI(int val);
  void SetPSCI(int val);
  void SetDTI(int val);
  void SetSWI(int f, int sm, int sl, int i);

  void SetTCBTYPE(TCB::TYPE val);

  int TM() const;
  int CW() const;
  int DLY() const;
  int PTRG() const;
  int MTHRF() const;
  int PSCF() const;
  int DTF() const;
  int SWF() const;
  int MTHRSM() const;
  int PSCSM() const;
  int DTSM() const;
  int SWSM() const;
  int MTHRSL() const;
  int PSCSL() const;
  int DTSL() const;
  int SWSL() const;
  int MTHRI() const;
  int PSCI() const;
  int DTI() const;
  int SWI() const;
  TCB::TYPE TCBTYPE() const;

  void SetCID(int, int) override {}
  void SetPID(int, int) override {}
  int CID(int) const override { return 0; }
  int PID(int) const override { return 0; }

  void PrintConf() const override;

private:
  int fTM{};
  int fCW{};
  int fDLY{};
  int fPTRG{};
  int fMTHRF{};
  int fPSCF{};
  int fDTF{};
  int fSWF{};
  int fMTHRSM{};
  int fPSCSM{};
  int fDTSM{};
  int fSWSM{};
  int fMTHRSL{};
  int fPSCSL{};
  int fDTSL{};
  int fSWSL{};
  int fMTHRI{};
  int fPSCI{};
  int fDTI{};
  int fSWI{};
  TCB::TYPE fTCBTYPE{};

  ClassDef(TCBConf, 1)
};

inline void TCBConf::SetTM(int val) { fTM = val; }
inline void TCBConf::SetCW(int val) { fCW = val; }
inline void TCBConf::SetDLY(int val) { fDLY = val; }
inline void TCBConf::SetPTRG(int val) { fPTRG = val; }

inline void TCBConf::SetMTHRF(int val) { fMTHRF = val; }
inline void TCBConf::SetPSCF(int val) { fPSCF = val; }
inline void TCBConf::SetDTF(int val) { fDTF = val; }

inline void TCBConf::SetSWF(int f, int sm, int sl, int i) { fSWF = f * 1 + sm * 2 + sl * 4 + i * 8; }

inline void TCBConf::SetMTHRSM(int val) { fMTHRSM = val; }
inline void TCBConf::SetPSCSM(int val) { fPSCSM = val; }
inline void TCBConf::SetDTSM(int val) { fDTSM = val; }

inline void TCBConf::SetSWSM(int f, int sm, int sl, int i) { fSWSM = f * 1 + sm * 2 + sl * 4 + i * 8; }

inline void TCBConf::SetMTHRSL(int val) { fMTHRSL = val; }
inline void TCBConf::SetPSCSL(int val) { fPSCSL = val; }
inline void TCBConf::SetDTSL(int val) { fDTSL = val; }

inline void TCBConf::SetSWSL(int f, int sm, int sl, int i) { fSWSL = f * 1 + sm * 2 + sl * 4 + i * 8; }

inline void TCBConf::SetMTHRI(int val) { fMTHRI = val; }
inline void TCBConf::SetPSCI(int val) { fPSCI = val; }
inline void TCBConf::SetDTI(int val) { fDTI = val; }

inline void TCBConf::SetSWI(int f, int sm, int sl, int i) { fSWI = f * 1 + sm * 2 + sl * 4 + i * 8; }

inline void TCBConf::SetTCBTYPE(TCB::TYPE val) { fTCBTYPE = val; }

inline int TCBConf::TM() const { return fTM; }
inline int TCBConf::CW() const { return fCW; }
inline int TCBConf::DLY() const { return fDLY; }
inline int TCBConf::PTRG() const { return fPTRG; }
inline int TCBConf::MTHRF() const { return fMTHRF; }
inline int TCBConf::PSCF() const { return fPSCF; }
inline int TCBConf::DTF() const { return fDTF; }
inline int TCBConf::SWF() const { return fSWF; }
inline int TCBConf::MTHRSM() const { return fMTHRSM; }
inline int TCBConf::PSCSM() const { return fPSCSM; }
inline int TCBConf::DTSM() const { return fDTSM; }
inline int TCBConf::SWSM() const { return fSWSM; }
inline int TCBConf::MTHRSL() const { return fMTHRSL; }
inline int TCBConf::PSCSL() const { return fPSCSL; }
inline int TCBConf::DTSL() const { return fDTSL; }
inline int TCBConf::SWSL() const { return fSWSL; }
inline int TCBConf::MTHRI() const { return fMTHRI; }
inline int TCBConf::PSCI() const { return fPSCI; }
inline int TCBConf::DTI() const { return fDTI; }
inline int TCBConf::SWI() const { return fSWI; }
inline TCB::TYPE TCBConf::TCBTYPE() const { return fTCBTYPE; }

#endif
