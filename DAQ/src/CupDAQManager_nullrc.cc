#include "TMessage.h"

#include "DAQ/CupDAQManager.hh"
#include "DAQConfig/RunConfig.hh"

void CupDAQManager::RC_NullTCB()
{
  int state = 0;
  INFO("NULLTCB controller now starting [run=%d]", fRunNumber);

  auto * daq = static_cast<DAQConf *>(fConfigList->GetDAQConfig());
  fDAQName = daq->GetDAQName(fDAQID);
  fDAQPort = daq->GetPort(fDAQID);

  std::thread th0(&CupDAQManager::TF_MsgServer, this);
  std::thread th1;

  //
  // prepare client sockets correspond to DAQs
  //
  bool socketerror = false;
  TSocket * socket = nullptr;

  for (int i = 0; i < daq->GetN(); i++) {
    int id = daq->GetID(i);
    if (id == fDAQID) { continue; }
    socket = new TSocket(daq->GetIPAddr(id).c_str(), daq->GetPort(id));
    socket->SetName(daq->GetDAQName(id).c_str());
    if (socket->GetErrorCode() < 0) {
      socketerror = true;
      ERROR("%s connection failed", socket->GetName());
      break;
    }
    fDAQSocket.push_back(socket);
    INFO("%s connected", socket->GetName());
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
  INFO("all DAQs were booted");

  //
  // wait for config run command
  //
  state = WaitCommand(fDoConfigRunTCB, fDoExitTCB);
  if (state != 0) {
    if (state == 1) { INFO("run=%d exited by Run Control", fRunNumber); }
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
  INFO("all DAQs were configured");

  th1 = std::thread(&CupDAQManager::TF_SplitOutput, this, true);

  //
  // wait for start run command
  //
  state = WaitCommand(fDoStartRunTCB, fDoExitTCB);
  if (state != 0) {
    if (state == 1) { INFO("run=%d exited by Run Control", fRunNumber); }
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
  INFO("all DAQs are running");

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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  //
  // stop trigger
  //
  time(&fEndDatime);
  SendCommandToDAQ(kENDRUN);

  WaitDAQStatus(RUNSTATE::kRUNENDED);
  RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kRUNENDED);
  INFO("run=%06d was ended", fRunNumber);

  WaitDAQStatus(RUNSTATE::kPROCENDED);
  RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kPROCENDED);
  INFO("all DAQ processes are ended");

TERMINATE:
  WaitCommand(fDoExitTCB);

  if (th1.joinable()) { th1.join(); }

  SendCommandToDAQ(kEXIT);

  for (auto * sock : fDAQSocket) {
    if (sock != nullptr) {
      sock->Close();
      delete sock;
    }
  }
  fDAQSocket.clear();

  if (th0.joinable()) { th0.join(); }

  INFO("TCB controller ended");
}

void CupDAQManager::RC_NullDAQ()
{
  fMonitorServerOn = fDoSendEvent ? 0 : 1;
  INFO("TCB controlled DAQ now starting");

  std::thread th0;
  std::thread th1;
  std::thread th2;

  auto * daq = static_cast<DAQConf *>(fConfigList->GetDAQConfig());
  fDAQName = daq->GetDAQName(fDAQID);
  fDAQPort = daq->GetPort(fDAQID);

  th0 = std::thread(&CupDAQManager::TF_MsgServer, this);
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kBOOTED);

  if (WaitCommand(fDoConfigRun, fDoExit) != 0) {
    if (th0.joinable()) { th0.join(); }
    WARNING("run=%d exited by TCB", fRunNumber);
    return;
  }

  fConfigList = nullptr;
  auto * socket = new TSocket(daq->GetIPAddr(0).c_str(), daq->GetPort(0));
  if (socket->GetErrorCode() < 0) {
    delete socket;
    ERROR("failed in TCB connection for configuration file");
    RUNSTATE::SetError(fRunStatus);
    goto TERMINATE;
  }
  else {
    char data[kMESSLEN];
    EncodeMsg(data, kREQUESTCONFIG);
    socket->SendRaw(data, kMESSLEN);

    TMessage * mess = nullptr;
    socket->Recv(mess);
    fConfigList = static_cast<AbsConfList *>(mess->ReadObject(mess->GetClass()));

    socket->Close();
    delete socket;
    delete mess;

    if (!fConfigList) {
      ERROR("null config list received");
      RUNSTATE::SetError(fRunStatus);
      goto TERMINATE;
    }
  }

  // thread
  th1 = std::thread(&CupDAQManager::TF_RunManager, this);
  th2 = std::thread(&CupDAQManager::TF_TriggerMon, this);

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

  if (WaitCommand(fDoStartRun, fDoExit) != 0) {
    WARNING("run=%d exited by TCB", fRunNumber);
    goto TERMINATE;
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
  time(&fStartDatime);

  while (true) {
    if (fDoEndRun) { break; }
    if (RUNSTATE::CheckError(fRunStatus)) { break; }

    fTriggerNumber += 10;
    fTriggerTime += 100 * kMILLISECOND;
    fNBuiltEvent += 10;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
  time(&fEndDatime);

TERMINATE:
  if (th2.joinable()) { th2.join(); }

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kPROCENDED);

  if (th1.joinable()) { th1.join(); }

  WaitCommand(fDoExit);

  if (th0.joinable()) { th0.join(); }
  CloseDAQ();

  INFO("TCB controlled DAQ ended");
}

void CupDAQManager::RC_NullMERGER()
{
  fMonitorServerOn = 1;
  INFO("event merger started");

  std::thread th0;
  std::thread th1;
  std::thread th2;

  auto * daq = static_cast<DAQConf *>(fConfigList->GetDAQConfig());
  fDAQName = daq->GetDAQName(fDAQID);
  fDAQPort = daq->GetPort(fDAQID);

  const char * adcname = GetADCName(fADCType);

  for (int i = 0; i < daq->GetN(); i++) {
    int id = daq->GetID(i);
    if (id == fDAQID) continue;

    const std::string & daq_name = daq->GetDAQName(id);

    if (daq_name.find(adcname) != std::string::npos) {
      INFO("event buffer for %s prepared", daq_name.c_str());
    }
  }

  switch (fADCType) {
    case ADC::FADCT: fADCMode = ADC::FMODE; break;
    case ADC::SADCT: fADCMode = ADC::SMODE; break;
    case ADC::IADCT: {
      auto * conf = static_cast<IADCTConf *>(fConfigList->GetConfig(ADC::IADCT, 0));
      if (conf->RL() > 0) { fADCMode = ADC::FMODE; }
      else {
        fADCMode = ADC::SMODE;
      }
      break;
    }
    default: break;
  }

  const char * adcmode = (fADCMode == ADC::SMODE) ? "SADC mode" : "FADC mode";
  INFO("ADCMode is %s", adcmode);

  th0 = std::thread(&CupDAQManager::TF_MsgServer, this);
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kBOOTED);

  if (WaitCommand(fDoConfigRun, fDoExit) != 0) {
    if (th0.joinable()) { th0.join(); }
    WARNING("run=%d exited by TCB", fRunNumber);
    return;
  }

  th1 = std::thread(&CupDAQManager::TF_RunManager, this);
  th2 = std::thread(&CupDAQManager::TF_TriggerMon, this);

  RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

  if (WaitCommand(fDoStartRun, fDoExit) != 0) {
    WARNING("run=%d exited by TCB", fRunNumber);
    goto TERMINATE;
  }

  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
  time(&fStartDatime);

  while (true) {
    if (fDoEndRun) { break; }
    if (RUNSTATE::CheckError(fRunStatus)) { break; }

    fTriggerNumber += 10;
    fTriggerTime += 100 * kMILLISECOND;
    fNBuiltEvent += 10;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
  time(&fEndDatime);

TERMINATE:
  if (th2.joinable()) { th2.join(); }

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kPROCENDED);

  if (th1.joinable()) { th1.join(); }

  WaitCommand(fDoExit);

  if (th0.joinable()) { th0.join(); }

  INFO("event merger ended");
}
