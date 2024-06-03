#include <thread>

#include "TMessage.h"

#include "DAQ/CupDAQManager.hh"
#include "DAQConfig/RunConfig.hh"

void CupDAQManager::RC_NullTCB()
{
  int state = 0;
  fLog->Info("CupDAQManager::RC_NullTCB",
             "NULLTCB controller now starting [run=%d]", fRunNumber);

  DAQConf * daq = (DAQConf *)fConfigList->GetDAQConfig();
  fDAQName = daq->GetDAQName(fDAQID);
  fDAQPort = daq->GetPort(fDAQID);

  std::thread th0 = std::thread(&CupDAQManager::TF_MsgServer, this);
  std::thread th1;

  //
  // prepare client sockets correspond to DAQs
  //
  bool socketerror = false;
  TSocket * socket = nullptr;

  for (int i = 0; i < daq->GetN(); i++) {
    int id = daq->GetID(i);
    if (id == fDAQID) continue;
    socket = new TSocket(daq->GetIPAddr(id).c_str(), daq->GetPort(id));
    socket->SetName(daq->GetDAQName(id).c_str());
    if (socket->GetErrorCode() < 0) {
      socketerror = true;
      fLog->Error("CupDAQManager::RC_NullTCB", "%s connection failed",
                  socket->GetName());
      break;
    }
    fDAQSocket.push_back(socket);
    fLog->Info("CupDAQManager::RC_NullTCB", "%s connected", socket->GetName());
  }

  if (socketerror) {
    RUNSTATE::SetError(fRunStatusTCB);
    goto TERMINATE;
  }

  // checking DAQs' status == kBOOTED
  if (!WaitDAQStatus(RUNSTATE::kBOOTED)) {
    RUNSTATE::SetError(fRunStatusTCB);
    goto TERMINATE;
  }
  RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kBOOTED);
  fLog->Info("CupDAQManager::RC_NullTCB", "all DAQs were booted");

  //
  // wait for config run command
  //
  state = WaitCommand(fDoConfigRunTCB, fDoExitTCB);
  if (state != 0) {
    if (state == 1) {
      fLog->Info("CupDAQManager::RC_NullTCB", "run=%d exited by Run Control",
                 fRunNumber);
    }
    else if (state < 0) {
      RUNSTATE::SetError(fRunStatusTCB);
    }
    goto TERMINATE;
  }

  //
  // configuration
  //
  SendCommandToDAQ(kCONFIGRUN);

  // checking DAQs' status == kCONFIGURED
  if (!WaitDAQStatus(RUNSTATE::kCONFIGURED)) {
    RUNSTATE::SetError(fRunStatusTCB);
    goto TERMINATE;
  }
  RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kCONFIGURED);
  fLog->Info("CupDAQManager::RC_NullTCB", "all DAQs were configured");

  th1 = std::thread(&CupDAQManager::TF_SplitOutput, this, true);

  //
  // wait for start run command
  //
  state = WaitCommand(fDoStartRunTCB, fDoExitTCB);
  if (state != 0) {
    if (state == 1) {
      fLog->Info("CupDAQManager::RC_NullTCB", "run=%d exited by Run Control",
                 fRunNumber);
    }
    else if (state < 0) {
      RUNSTATE::SetError(fRunStatusTCB);
    }
    goto TERMINATE;
  }

  //
  // start trigger
  //
  time(&fStartDatime);
  SendCommandToDAQ(kSTARTRUN);

  // checking DAQs' status == kRUNNING
  if (!WaitDAQStatus(RUNSTATE::kRUNNING)) {
    RUNSTATE::SetError(fRunStatusTCB);
    goto TERMINATE;
  }
  RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kRUNNING);
  fLog->Info("CupDAQManager::RC_NullTCB", "all DAQs are running");

  while (true) {
    if (fDoEndRunTCB || IsForcedEndRunFile()) { break; }
    if (!IsDAQRunning()) {
      RUNSTATE::SetError(fRunStatusTCB);
      break;
    }
    if (fDoSplitOutputFile) {
      SendCommandToDAQ(kSPLITOUTPUTFILE);
      fDoSplitOutputFile = false;
    }
    gSystem->Sleep(100);
  }

  //
  // stop trigger
  //
  time(&fEndDatime);
  SendCommandToDAQ(kENDRUN);

  WaitDAQStatus(RUNSTATE::kRUNENDED);
  RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kRUNENDED);
  fLog->Info("CupDAQManager::RC_NullTCB", "run=%06d was ended", fRunNumber);

  WaitDAQStatus(RUNSTATE::kPROCENDED);
  RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kPROCENDED);
  fLog->Info("CupDAQManager::RC_NullTCB", "all DAQ processes are ended");

TERMINATE:
  WaitCommand(fDoExitTCB);

  if (th1.joinable()) { th1.join(); }

  SendCommandToDAQ(kEXIT);

  for (auto * sock : fDAQSocket) {
    sock->Close();
    delete sock;
  }

  th0.join();

  fLog->Info("CupDAQManager::RC_NullTCB", "TCB controller ended");
}

void CupDAQManager::RC_NullDAQ()
{
  fMonitorServerOn = fDoSendEvent ? 0 : 1;
  fLog->Info("CupDAQManager::RC_NullDAQ", "TCB controlled DAQ now starting");

  std::thread th0;
  std::thread th1;
  std::thread th2;

  DAQConf * daq = (DAQConf *)fConfigList->GetDAQConfig();
  fDAQName = daq->GetDAQName(fDAQID);
  fDAQPort = daq->GetPort(fDAQID);

  th0 = std::thread(&CupDAQManager::TF_MsgServer, this);
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kBOOTED);

  if (WaitCommand(fDoConfigRun, fDoExit) != 0) {
    th0.join();
    fLog->Warning("CupDAQManager::RC_NullDAQ", "run=%d exited by TCB",
                  fRunNumber);
    return;
  }

  fConfigList = nullptr;
  auto * socket = new TSocket(daq->GetIPAddr(0).c_str(), daq->GetPort(0));
  if (socket->GetErrorCode() < 0) {
    delete socket;
    fLog->Error("CupDAQManager::RC_NullDAQ",
                "failed in TCB connection for configuration file");
    RUNSTATE::SetError(fRunStatus);
    goto TERMINATE;
  }
  else {
    char data[kMESSLEN];
    EncodeMsg(data, kREQUESTCONFIG);
    socket->SendRaw(data, kMESSLEN);

    TMessage * mess;
    socket->Recv(mess);
    fConfigList = (AbsConfList *)mess->ReadObject(mess->GetClass());

    socket->Close();
    delete socket;
    delete mess;

    if (!fConfigList) {
      fLog->Error("CupDAQManager::RC_NullDAQ", "null config list received");
      RUNSTATE::SetError(fRunStatus);
      goto TERMINATE;
    }
  }

  // thread
  th1 = std::thread(&CupDAQManager::TF_RunManager, this);
  th2 = std::thread(&CupDAQManager::TF_TriggerMon, this);

  gSystem->Sleep(2000);
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

  if (WaitCommand(fDoStartRun, fDoExit) != 0) {
    fLog->Warning("CupDAQManager::RC_NullDAQ", "run=%d exited by TCB",
                  fRunNumber);
    goto TERMINATE;
  }

  gSystem->Sleep(2000);
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
  time(&fStartDatime);

  while (true) {
    if (fDoEndRun) break;
    if (RUNSTATE::CheckError(fRunStatus)) break;

    fTriggerNumber += 10;
    fTriggerTime += 100 * kMILLISECOND;
    fNBuiltEvent += 10;

    gSystem->Sleep(100);
  }

  gSystem->Sleep(2000);
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
  time(&fEndDatime);

TERMINATE:
  if (th2.joinable()) th2.join();

  gSystem->Sleep(2000);
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kPROCENDED);

  if (th1.joinable()) th1.join();

  WaitCommand(fDoExit);

  th0.join();
  CloseDAQ();

  fLog->Info("CupDAQManager::RC_NullDAQ", "TCB controlled DAQ ended");
}

void CupDAQManager::RC_NullMERGER()
{
  fMonitorServerOn = 1;
  fLog->Info("CupDAQManager::RC_NullMERGER", "event merger started");

  std::thread th0;
  std::thread th1;
  std::thread th2;

  DAQConf * daq = (DAQConf *)fConfigList->GetDAQConfig();
  fDAQName = daq->GetDAQName(fDAQID);
  fDAQPort = daq->GetPort(fDAQID);

  TString adcname = GetADCName(fADCType);

  for (int i = 0; i < daq->GetN(); i++) {
    int id = daq->GetID(i);
    if (id != fDAQID && TString(daq->GetDAQName(id)).Contains(adcname)) {
      fLog->Info("CupDAQManager::RC_NullMERGER", "event buffer for %s prepared",
                 daq->GetDAQName(id).c_str());
    }
  }

  switch (fADCType) {
    case ADC::FADCT: fADCMode = ADC::FMODE; break;
    case ADC::SADCT: fADCMode = ADC::SMODE; break;
    case ADC::IADCT: {
      IADCTConf * conf = (IADCTConf *)fConfigList->GetConfig(ADC::IADCT, 0);
      if (conf->RL() > 0) fADCMode = ADC::FMODE;
      else fADCMode = ADC::SMODE;
      break;
    }
    default: break;
  }

  const char * adcmode = (fADCMode == ADC::SMODE) ? "SADC mode" : "FADC mode";
  fLog->Info("CupDAQManager::RC_NullMERGER", "ADCMode is %s", adcmode);

  th0 = std::thread(&CupDAQManager::TF_MsgServer, this);
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kBOOTED);

  if (WaitCommand(fDoConfigRun, fDoExit) != 0) {
    th0.join();
    fLog->Warning("CupDAQManager::RC_NullMERGER", "run=%d exited by TCB",
                  fRunNumber);
    return;
  }

  th1 = std::thread(&CupDAQManager::TF_RunManager, this);
  th2 = std::thread(&CupDAQManager::TF_TriggerMon, this);

  RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

  if (WaitCommand(fDoStartRun, fDoExit) != 0) {
    fLog->Warning("CupDAQManager::RC_NullMERGER", "run=%d exited by TCB",
                  fRunNumber);
    goto TERMINATE;
  }

  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
  time(&fStartDatime);

  while (true) {
    if (fDoEndRun) break;
    if (RUNSTATE::CheckError(fRunStatus)) break;

    fTriggerNumber += 10;
    fTriggerTime += 100 * kMILLISECOND;
    fNBuiltEvent += 10;

    gSystem->Sleep(100);
  }

  gSystem->Sleep(2000);
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
  time(&fEndDatime);

TERMINATE:
  if (th2.joinable()) th2.join();

  gSystem->Sleep(2000);
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kPROCENDED);

  if (th1.joinable()) th1.join();

  WaitCommand(fDoExit);
  th0.join();

  fLog->Info("CupDAQManager::RC_NullMERGER", "event merger ended");
}