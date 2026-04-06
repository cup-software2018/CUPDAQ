#include <iostream>

#include "DAQSystem/AbsTCB.hh"
#include "OnlConsts/adcconsts.hh"

using namespace std;

ClassImp(AbsTCB)

void AbsTCB::WriteRegisterTCB(TCBConf * conf)
{
  WriteTRIGENABLE(0, conf->TM());
  WriteCW(0, 1, conf->CW());
  WriteGATEDLY(conf->DLY());
  WritePTRIG(conf->PTRG());
  WriteMTHRFADC(conf->MTHRF());
  WritePSCALEFADC(conf->PSCF());
  WriteDT(0, 0, conf->DTF());

  if (conf->TCBTYPE() == TCB::V2) {
    WriteMTHRSADCMU(conf->MTHRSM());
    WritePSCALESADCMU(conf->PSCSM());
    WriteDT(0, 1, conf->DTSM());
    WriteMTHRSADCLS(conf->MTHRSL());
    WritePSCALESADCLS(conf->PSCSL());
    WriteDT(0, 2, conf->DTSL());
    WriteMTHRIADC(conf->MTHRI());
    WritePSCALEIADC(conf->PSCI());
    WriteDT(0, 3, conf->DTI());

    int sw = conf->SWF();
    uint32_t f = (uint32_t)TESTBIT(sw, 0);
    uint32_t sm = (uint32_t)TESTBIT(sw, 1);
    uint32_t sl = (uint32_t)TESTBIT(sw, 2);
    uint32_t i = (uint32_t)TESTBIT(sw, 3);
    WriteTRGSWFADC(f, sm, sl, i);

    sw = conf->SWSM();
    f = (uint32_t)TESTBIT(sw, 0);
    sm = (uint32_t)TESTBIT(sw, 1);
    sl = (uint32_t)TESTBIT(sw, 2);
    i = (uint32_t)TESTBIT(sw, 3);
    WriteTRGSWSADCMU(f, sm, sl, i);

    sw = conf->SWSL();
    f = (uint32_t)TESTBIT(sw, 0);
    sm = (uint32_t)TESTBIT(sw, 1);
    sl = (uint32_t)TESTBIT(sw, 2);
    i = (uint32_t)TESTBIT(sw, 3);
    WriteTRGSWSADCLS(f, sm, sl, i);

    sw = conf->SWI();
    f = (uint32_t)TESTBIT(sw, 0);
    sm = (uint32_t)TESTBIT(sw, 1);
    sl = (uint32_t)TESTBIT(sw, 2);
    i = (uint32_t)TESTBIT(sw, 3);
    WriteTRGSWIADC(f, sm, sl, i);
  }
}

void AbsTCB::WriteRegisterFADC(FADCTConf * conf)
{
  uint32_t mid = conf->MID();
  WriteRL(mid, conf->RL());
  WriteDSR(mid, conf->DSR());
  WriteTLT(mid, conf->TLT());

  int nch = conf->NCH();
  for (int i = 0; i < nch; i++) {
    ULong_t cid = conf->CID(i);

    WriteAMODE(mid, cid, conf->AMD(i));
    WritePOL(mid, cid, conf->POL(i));
    WriteCW(mid, cid, conf->CW(i));
    WriteTHR(mid, cid, conf->THR(i));
    WriteDLY(mid, cid, conf->DLY(i));
    WriteDT(mid, cid, conf->DT(i));
    WriteTM(mid, cid, conf->TM(i));
    WritePCT(mid, cid, conf->PCT(i));
    WritePCI(mid, cid, conf->PCI(i));
    WritePWT(mid, cid, conf->PWT(i));
    WritePSW(mid, cid, conf->PSW(i));
    WriteDACOFF(mid, cid, conf->DACOFF(i));
  }

  AlignFADC(mid);
}

void AbsTCB::WriteRegisterSADC(SADCTConf * conf)
{
  uint32_t mid = conf->MID();

  WriteGW(mid, conf->GW());
  WriteCW(mid, 1, conf->CW());
  WritePSW(mid, 1, conf->PSW());
  WriteAMODE(mid, 1, conf->SUBPED());

  int nch = conf->NCH();
  for (int i = 0; i < nch; i++) {
    ULong_t cid = conf->CID(i);
    WriteTHR(mid, cid, conf->THR(i));
    WriteDLY(mid, cid, conf->DLY(i));
  }

  for (int i = 0; i < 8; i++) {
    WriteSTLT(mid, (i + 1) * 4, conf->TLT(i));
  }

  AlignSADC(mid);
}

void AbsTCB::WriteRegisterIADC(IADCTConf * conf)
{
  uint32_t mid = conf->MID();

  int mode = conf->MODE();
  WriteRL(mid, conf->RL());
  WriteGW(mid, conf->GW());
  WriteCW(mid, 1, conf->CW());
  WritePSW(mid, 1, conf->PSW());
  WriteDAQMODE(mid, mode);

  int nch = conf->NCH();
  for (int i = 0; i < nch; i++) {
    ULong_t cid = conf->CID(i);
    WriteTHR(mid, cid, conf->THR(i));
    WriteDLY(mid, cid, conf->DLY(i));
  }

  for (int i = 0; i < 10; i++) {
    WriteSTLT(mid, i + 1, conf->TLT(i));
  }

  for (int i = 0; i < 5; i++) {
    WriteHV(mid, i + 1, conf->HV(i));
  }

  AlignIADC(mid);
}

// print register
void AbsTCB::PrintRegisterTCB(TCBConf * conf)
{
  if (conf->TCBTYPE() == TCB::V2) {
    uint32_t sw = ReadTRGSWFADC();
    int f = (int)TESTBIT(sw, 0);
    int sm = (int)TESTBIT(sw, 1);
    int sl = (int)TESTBIT(sw, 2);
    int i = (int)TESTBIT(sw, 3);
    const char * swf = Form("%d %d %d %d", f, sm, sl, i);

    sw = ReadTRGSWSADCMU();
    f = (int)TESTBIT(sw, 0);
    sm = (int)TESTBIT(sw, 1);
    sl = (int)TESTBIT(sw, 2);
    i = (int)TESTBIT(sw, 3);
    const char * swsm = Form("%d %d %d %d", f, sm, sl, i);

    sw = ReadTRGSWSADCLS();
    f = (int)TESTBIT(sw, 0);
    sm = (int)TESTBIT(sw, 1);
    sl = (int)TESTBIT(sw, 2);
    i = (int)TESTBIT(sw, 3);
    const char * swsl = Form("%d %d %d %d", f, sm, sl, i);

    sw = ReadTRGSWIADC();
    f = (int)TESTBIT(sw, 0);
    sm = (int)TESTBIT(sw, 1);
    sl = (int)TESTBIT(sw, 2);
    i = (int)TESTBIT(sw, 3);
    const char * swi = Form("%d %d %d %d", f, sm, sl, i);

    cout << Form(" ++ TCB register: SID(0) TRGON(%u) CW(%u) DLY(%u) "
                 "PTRIG(%u) \n"
                 "                  MTHRF(%u)  PSCF(%u)  DTF(%u)  TSWF(%s) \n"
                 "                  MTHRSM(%u) PSCSM(%u) DTSM(%u) TSWSM(%s) "
                 "\n"
                 "                  MTHRSL(%u) PSCSL(%u) DTSL(%u) TSWSL(%s) "
                 "\n"
                 "                  MTHRI(%u)  PSCI(%u)  DTI(%u)  TSWI(%s) ",
                 ReadTRIGENABLE(0), ReadCW(0, 1), ReadGATEDLY(), ReadPTRIG(), ReadMTHRFADC(),
                 ReadPSCALEFADC(), ReadDT(0, 0), swf, ReadMTHRSADCMU(), ReadPSCALESADCMU(),
                 ReadDT(0, 1), swsm, ReadMTHRSADCLS(), ReadPSCALESADCLS(), ReadDT(0, 2), swsl,
                 ReadMTHRIADC(), ReadPSCALEIADC(), ReadDT(0, 3), swi)
         << endl;
  }
  else {
    cout << Form(" ++ TCB register: SID(0) TRGON(%u) CW(%u) DLY(%u) "
                 "PTRIG(%u) \n"
                 "                  MTHR(%u) PSC(%u) DT(%u)",
                 ReadTRIGENABLE(0), ReadCW(0, 1), ReadGATEDLY(), ReadPTRIG(), ReadMTHRFADC(),
                 ReadPSCALEFADC(), ReadDT(0, 0))
         << endl;
  }
}

void AbsTCB::PrintRegisterFADC(FADCTConf * conf)
{
  int sid = conf->SID();
  int mid = conf->MID();
  int nch = conf->NCH();

  uint32_t rRL = ReadRL(mid);
  uint32_t rTLT = ReadTLT(mid);
  uint32_t rDSR = ReadDSR(mid);

  cout << Form(" ++ FADC register: SID(%d) MID(%1d) NCH(%1d) RL(%u) DSR(%u) TLT(%X) ", sid, mid,
               nch, rRL, rDSR, rTLT)
       << endl;

  cout << " -----------------------------------------------" << endl;
  cout << "        CID : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8d", conf->CID(i));
  }
  cout << endl;
  cout << "        POL : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8u", ReadPOL(mid, conf->CID(i)));
  }
  cout << endl;
  cout << "     DACOFF : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8u", ReadDACOFF(mid, conf->CID(i)));
  }
  cout << endl;
  cout << "      AMODE : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8u", ReadAMODE(mid, conf->CID(i)));
  }
  cout << endl;
  cout << "        DLY : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8u", ReadDLY(mid, conf->CID(i)));
  }
  cout << endl;
  cout << "      DTIME : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8u", ReadDT(mid, conf->CID(i)));
  }
  cout << endl;
  cout << "         CW : ";
  for (int i = 0; i < nch; i++)
    cout << Form("%8u", ReadCW(mid, conf->CID(i)));
  cout << endl;
  cout << "         TM : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8u", ReadTM(mid, conf->CID(i)));
  }
  cout << endl;
  cout << "        THR : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8u", ReadTHR(mid, conf->CID(i)));
  }
  cout << endl;
  cout << "        PCT : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8u", ReadPCT(mid, conf->CID(i)));
  }
  cout << endl;
  cout << "        PCI : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8u", ReadPCI(mid, conf->CID(i)));
  }
  cout << endl;
  cout << "        PWT : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8u", ReadPWT(mid, conf->CID(i)));
  }
  cout << endl;
  cout << "        PSW : ";
  for (int i = 0; i < nch; i++) {
    cout << Form("%8u", ReadPSW(mid, conf->CID(i)));
  }
  cout << endl;
  cout << " -----------------------------------------------" << endl;
  cout << endl;
}

void AbsTCB::PrintRegisterSADC(SADCTConf * conf)
{
  int sid = conf->SID();
  int mid = conf->MID();
  int nch = conf->NCH();

  uint32_t rCW = ReadCW(mid, 1);
  uint32_t rGW = ReadGW(mid);
  uint32_t rPSW = ReadPSW(mid, 1);
  uint32_t rPEDSUB = ReadAMODE(mid, 1);

  cout << Form(" ++ SADCT register: SID(%d) MID(%d) NCH(%1d) CW(%u) GW(%u) "
               "PSW(%u) SUBPED(%u)",
               sid, mid, nch, rCW, rGW, rPSW, rPEDSUB)
       << endl;
  cout << Form("                    TLT1(%X) TLT2(%X) TLT3(%X) TLT4(%X)", ReadSTLT(mid, 1 * 4),
               ReadSTLT(mid, 2 * 4), ReadSTLT(mid, 3 * 4), ReadSTLT(mid, 4 * 4))
       << endl;
  cout << Form("                    TLT5(%X) TLT6(%X) TLT7(%X) TLT8(%X)", ReadSTLT(mid, 5 * 4),
               ReadSTLT(mid, 6 * 4), ReadSTLT(mid, 7 * 4), ReadSTLT(mid, 8 * 4))
       << endl;
  cout << " -----------------------------------------------" << endl;
  for (int j = 0; j < 4; j++) {
    if (j > 0) cout << endl;
    cout << "     CID : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++) {
      cout << Form("%6d", conf->CID(i));
    }
    cout << endl;
    cout << "     THR : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++) {
      cout << Form("%6u", ReadTHR(mid, conf->CID(i)));
    }
    cout << endl;
    cout << "     DLY : ";
    for (int i = 8 * j; i < 8 * (j + 1); i++) {
      cout << Form("%6u", ReadDLY(mid, conf->CID(i)));
    }
    cout << endl;
  }
  cout << endl;
  cout << " -----------------------------------------------" << endl;
  cout << endl;
}

void AbsTCB::PrintRegisterIADC(IADCTConf * conf)
{
  int sid = conf->SID();
  int mid = conf->MID();
  int nch = conf->NCH();

  uint32_t rRL = ReadRL(mid);
  uint32_t rCW = ReadCW(mid, 1);
  uint32_t rGW = ReadGW(mid);
  uint32_t rPSW = ReadPSW(mid, 1);
  uint32_t rMODE = ReadDAQMODE(mid);

  if (rMODE > 0) rGW = 0;
  else rRL = 0;

  cout << Form(" ++ IADCT register: SID(%d) MID(%1d) NCH(%1d) MODE(%u) "
               "RL(%u) CW(%u) GW(%u) PSW(%u)",
               sid, mid, nch, rMODE, rRL, rCW, rGW, rPSW)
       << endl;
  cout << Form("                    TLT1(%X) TLT2(%X) TLT3(%X) TLT4(%X) "
               "TLT5(%X)",
               ReadSTLT(mid, 1), ReadSTLT(mid, 2), ReadSTLT(mid, 3), ReadSTLT(mid, 4),
               ReadSTLT(mid, 4))
       << endl;
  cout << Form("                    TLT6(%X) TLT7(%X) TLT8(%X) TLT9(%X) "
               "TLT10(%X)",
               ReadSTLT(mid, 5), ReadSTLT(mid, 6), ReadSTLT(mid, 7), ReadSTLT(mid, 8),
               ReadSTLT(mid, 4))
       << endl;
  cout << Form("                    HV1(%.1f) HV2(%.1f) HV3(%.1f) HV4(%.1f) "
               "HV5(%.1f)",
               ReadHV(mid, 1), ReadHV(mid, 2), ReadHV(mid, 3), ReadHV(mid, 4), ReadHV(mid, 5))
       << endl;
  cout << " -----------------------------------------------" << endl;
  for (int j = 0; j < 4; j++) {
    if (j > 0) cout << endl;
    cout << "     CID : ";
    for (int i = 10 * j; i < 10 * (j + 1); i++) {
      cout << Form("%6d", conf->CID(i));
    }
    cout << endl;
    cout << "     THR : ";
    for (int i = 10 * j; i < 10 * (j + 1); i++) {
      cout << Form("%6u", ReadTHR(mid, conf->CID(i)));
    }
    cout << endl;
    cout << "     DLY : ";
    for (int i = 10 * j; i < 10 * (j + 1); i++) {
      cout << Form("%6u", ReadDLY(mid, conf->CID(i)));
    }
    cout << endl;
  }
  cout << " -----------------------------------------------" << endl;
  cout << endl;
}

void AbsTCB::MeasurePedestalFADC(FADCTConf * conf)
{
  int mid = conf->MID();
  int nch = conf->NCH();

  cout << Form("  [mid=%d]  ", mid) << flush;
  for (int i = 0; i < nch; i++) {
    ULong_t cid = conf->CID(i);
    MeasurePED(mid, cid);
    cout << Form("%4u  ", ReadPED(mid, cid)) << flush;
  }
  cout << endl;
}

void AbsTCB::MeasurePedestalSADC(SADCTConf * conf)
{
  int mid = conf->MID();
  int nch = conf->NCH();

  cout << Form("  [mid=%d]  ", mid) << endl;
  MeasurePED(mid, 1);
  for (int i = 0; i < nch; i++) {
    int cid = conf->CID(i);
    cout << Form("ch%02d = %4u  ", cid, ReadPED(mid, cid)) << flush;
    if (cid % 8 == 0) { cout << endl; }
  }
  if (nch % 8 != 0) { cout << endl; }
  cout << endl;
}

void AbsTCB::MeasurePedestalIADC(IADCTConf * conf)
{
  int mid = conf->MID();
  int nch = conf->NCH();

  cout << Form("  [mid=%d]  ", mid) << endl;
  MeasurePED(mid, 1);
  for (int i = 0; i < nch; i++) {
    int cid = conf->CID(i);
    cout << Form("ch%02d = %4u  ", cid, ReadPED(mid, cid)) << flush;
    if (cid % 10 == 0) { cout << endl; }
  }
  if (nch % 10 != 0) { cout << endl; }
  cout << endl;
}
