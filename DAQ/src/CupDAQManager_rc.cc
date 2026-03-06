#include <filesystem>

#include "TMessage.h"

#include "DAQ/CupDAQManager.hh"
#include "DAQConfig/RunConfig.hh"

void CupDAQManager::Run()
{
  if (IsForcedEndRunFile()) {
    std::error_code ec;
    std::filesystem::remove(kFORCEDENDRUNFILE, ec);
  }

  auto runConfig = std::make_unique<RunConfig>();
  if (fConfigFilename.empty()) {
    ERROR("no config filename");
    return;
  }
  if (!runConfig->ReadConfig(fConfigFilename.c_str())) {
    ERROR("error in reading config file \"%s\"", fConfigFilename.c_str());
    return;
  }
  fConfigList = runConfig->GetConfigs();

  switch (fDAQType) {
    case DAQ::STDDAQ: RC_STDDAQ(); break;
    case DAQ::TCBDAQ: RC_TCBDAQ(); break;
    case DAQ::TCBCTRL: RC_TCBCTRLDAQ(); break;
    case DAQ::TCB: RC_TCB(); break;
    case DAQ::MERGER: RC_MERGER(); break;
    case DAQ::NULLTCB: RC_NullTCB(); break;
    case DAQ::NULLDAQ: RC_NullDAQ(); break;
    case DAQ::NULLMERGER: RC_NullMERGER(); break;
    default: break;
  }
}

void CupDAQManager::RC_TCB()
{
  int state = 0;
  INFO("TCB controller now starting [run=%d]", fRunNumber);

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
    if (socket->GetErrorCode() < 0) {
      socketerror = true;
      ERROR("%s connection failed", socket->GetName());
      break;
    }
    socket->SetName(daq->GetDAQName(id).c_str());
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

  fTCB = new CupGeneralTCB();
  fTCB->SetConfig(fConfigList);
  if (fTCB->Open() != 0) {
    RUNSTATE::SetError(fRunStatusTCB);
    goto TERMINATE;
  }

  if (!fTCB->Config()) {
    RUNSTATE::SetError(fRunStatusTCB);
    goto TERMINATE;
  }

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

  fTCB->StartTrigger();
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
    if (!IsDAQRunning()) { break; }
    if (fDoSplitOutputFile) {
      SendCommandToDAQ(kSPLITOUTPUTFILE);
      fDoSplitOutputFile = false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  fTCB->StopTrigger();
  time(&fEndDatime);

  SendCommandToDAQ(kENDRUN);

  // checking DAQs' status == kRUNENDED
  if (!WaitDAQStatus(RUNSTATE::kRUNENDED)) {
    RUNSTATE::SetError(fRunStatusTCB);
    goto TERMINATE;
  }
  RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kRUNENDED);
  INFO("run=%06d was ended", fRunNumber);

  if (!WaitDAQStatus(RUNSTATE::kPROCENDED)) {
    RUNSTATE::SetError(fRunStatusTCB);
    goto TERMINATE;
  }
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

  if (fTCB) {
    fTCB->Close();
    delete fTCB;
    fTCB = nullptr;
  }

  INFO("TCB controller ended");
}

void CupDAQManager::RC_TCBCTRLDAQ()
{
  fMonitorServerOn = fDoSendEvent ? 0 : 1;
  INFO("TCB controlled DAQ now starting");

  std::thread th0;
  std::thread th1;
  std::thread th2;
  std::thread th3;
  std::thread th4;
  std::thread th5;
  std::thread th6;
  std::thread th7;
  std::thread th8;

  auto * daq = static_cast<DAQConf *>(fConfigList->GetDAQConfig());
  fDAQName = daq->GetDAQName(fDAQID);
  fDAQPort = daq->GetPort(fDAQID);

  th0 = std::thread(&CupDAQManager::TF_MsgServer, this);
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kBOOTED);

  if (fDoSendEvent) {
    std::string target_name = GetADCName(fADCType);
    target_name += "MERGER";

    for (int i = 0; i < daq->GetN(); i++) {
      int id = daq->GetID(i);

      const std::string & daq_name = daq->GetDAQName(id);

      if (daq_name.find(target_name) != std::string::npos) {
        fMergeServerIPAddr = daq->GetIPAddr(id);
        fMergeServerPort = daq->GetPort(id);
      }
    }
  }

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

  if (!AddADC(fConfigList)) {
    RUNSTATE::SetError(fRunStatus);
    goto TERMINATE;
  }
  if (!PrepareDAQ()) {
    RUNSTATE::SetError(fRunStatus);
    goto TERMINATE;
  }
  if (!OpenDAQ()) {
    RUNSTATE::SetError(fRunStatus);
    goto TERMINATE;
  }

  // thread
  th1 = std::thread(&CupDAQManager::TF_RunManager, this);
  th2 = std::thread(&CupDAQManager::TF_ReadData, this);
  th3 = std::thread(&CupDAQManager::TF_SortEvent, this);
  th4 = std::thread(&CupDAQManager::TF_BuildEvent, this);
  if (!fDoSendEvent) { th5 = std::thread(&CupDAQManager::TF_WriteEvent, this); }
  else {
    th5 = std::thread(&CupDAQManager::TF_SendEvent, this);
  }
  th6 = std::thread(&CupDAQManager::TF_TriggerMon, this);
  th7 = std::thread(&CupDAQManager::TF_ShrinkToFit, this);
  if (fVerboseLevel > 0) { th8 = std::thread(&CupDAQManager::TF_DebugMon, this); }

  RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

  if (WaitCommand(fDoStartRun, fDoExit) != 0) {
    WARNING("run=%d exited by TCB", fRunNumber);
    goto TERMINATE;
  }

  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
  time(&fStartDatime);

  if (WaitCommand(fDoEndRun, fRunStatus) != 0) {
    WARNING("run=%d ended by error state", fRunNumber);
  }

  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
  time(&fEndDatime);

TERMINATE:
  if (th8.joinable()) th8.join();
  if (th7.joinable()) th7.join();
  if (th6.joinable()) th6.join();
  if (th5.joinable()) th5.join();
  if (th4.joinable()) th4.join();
  if (th3.joinable()) th3.join();
  if (th2.joinable()) th2.join();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kPROCENDED);

  if (th1.joinable()) th1.join();

  WaitCommand(fDoExit);

  if (th0.joinable()) { th0.join(); }
  CloseDAQ();

  INFO("TCB controlled DAQ ended");
}

void CupDAQManager::RC_MERGER()
{
  fMonitorServerOn = 1;
  INFO("event merger started");

  std::thread th0;
  std::thread th1;
  std::thread th2;
  std::thread th3;
  std::thread th4;
  std::thread th5;
  std::thread th6;

  auto * daq = static_cast<DAQConf *>(fConfigList->GetDAQConfig());
  fDAQName = daq->GetDAQName(fDAQID);
  fDAQPort = daq->GetPort(fDAQID);

  const char * adcname = GetADCName(fADCType);

  for (int i = 0; i < daq->GetN(); i++) {
    int id = daq->GetID(i);

    if (id == fDAQID) continue;

    const std::string & daq_name = daq->GetDAQName(id);

    if (daq_name.find(adcname) != std::string::npos) {
      auto evtbuf = std::make_unique<ConcurrentDeque<std::unique_ptr<BuiltEvent>>>();
      fRecvEventBuffer.emplace_back(id, std::move(evtbuf));

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
  th2 = std::thread(&CupDAQManager::TF_MergeEvent, this);
  th3 = std::thread(&CupDAQManager::TF_WriteEvent, this);
  th4 = std::thread(&CupDAQManager::TF_TriggerMon, this);
  th5 = std::thread(&CupDAQManager::TF_ShrinkToFit, this);
  if (fVerboseLevel > 0) { th6 = std::thread(&CupDAQManager::TF_DebugMon, this); }

  RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

  if (WaitCommand(fDoStartRun, fDoExit) != 0) {
    WARNING("run=%d exited by TCB", fRunNumber);
    goto TERMINATE;
  }

  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
  time(&fStartDatime);

  if (WaitCommand(fDoEndRun, fRunStatus) != 0) {
    WARNING("run=%d ended by error state", fRunNumber);
  }

  RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
  time(&fEndDatime);

TERMINATE:
  if (th6.joinable()) th6.join();
  if (th5.joinable()) th5.join();
  if (th4.joinable()) th4.join();
  if (th3.joinable()) th3.join();
  if (th2.joinable()) th2.join();

  for (auto & pair : fRecvEventBuffer) {
    auto * buffer = pair.second.get();
    if (buffer != nullptr) {
      while (!buffer->empty()) {
        buffer->pop_front();
      }
    }
  }
  fRecvEventBuffer.clear();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kPROCENDED);

  if (th1.joinable()) th1.join();

  WaitCommand(fDoExit);
  if (th0.joinable()) { th0.join(); }

  INFO("event merger ended");
}

void CupDAQManager::RC_STDDAQ()
{
  INFO("STDDAQ now starting");

  std::thread th1;
  std::thread th2;
  std::thread th3;
  std::thread th4;
  std::thread th5;
  std::thread th6;
  std::thread th7;
  std::thread th8;
  std::thread th9;

  if (!AddADC(fConfigList)) { return; }

  if (!OpenDAQ()) { return; }
  if (!ConfigureDAQ()) { goto TERMINATE; }
  if (!PrepareDAQ()) { goto TERMINATE; }
  if (!InitializeDAQ()) { goto TERMINATE; }

  // thread
  th1 = std::thread(&CupDAQManager::TF_RunManager, this);
  th2 = std::thread(&CupDAQManager::TF_ReadData, this);
  th3 = std::thread(&CupDAQManager::TF_SortEvent, this);
  th4 = std::thread(&CupDAQManager::TF_BuildEvent, this);
  th5 = std::thread(&CupDAQManager::TF_WriteEvent, this);
  th6 = std::thread(&CupDAQManager::TF_TriggerMon, this);
  th7 = std::thread(&CupDAQManager::TF_SplitOutput, this, false);
  th8 = std::thread(&CupDAQManager::TF_ShrinkToFit, this);
  if (fVerboseLevel > 0) { th9 = std::thread(&CupDAQManager::TF_DebugMon, this); }

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

  if (th9.joinable()) th9.join();
  if (th8.joinable()) th8.join();
  if (th7.joinable()) th7.join();
  if (th6.joinable()) th6.join();
  if (th5.joinable()) th5.join();
  if (th4.joinable()) th4.join();
  if (th3.joinable()) th3.join();
  if (th2.joinable()) th2.join();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kPROCENDED);

  if (th1.joinable()) th1.join();

TERMINATE:
  CloseDAQ();

  INFO("STDDAQ ended");
}

void CupDAQManager::RC_TCBDAQ()
{
  INFO("TCBDAQ now starting");

  std::thread th1;
  std::thread th2;
  std::thread th3;
  std::thread th4;
  std::thread th5;
  std::thread th6;
  std::thread th7;
  std::thread th8;
  std::thread th9;

  fTCB = new CupGeneralTCB();
  fTCB->SetConfig(fConfigList);
  fTCB->SetADCType(fADCType);

  if (fTCB->Open() != 0) {
    delete fTCB;
    fTCB = nullptr;
    return;
  }
  if (!fTCB->Config()) { goto TERMINATE; }
  if (!AddADC(fConfigList)) { goto TERMINATE; }
  if (!PrepareDAQ()) { goto TERMINATE; }
  if (!OpenDAQ()) { goto TERMINATE; }

  // thread
  th1 = std::thread(&CupDAQManager::TF_RunManager, this);
  th2 = std::thread(&CupDAQManager::TF_ReadData, this);
  th3 = std::thread(&CupDAQManager::TF_SortEvent, this);
  th4 = std::thread(&CupDAQManager::TF_BuildEvent, this);
  th5 = std::thread(&CupDAQManager::TF_WriteEvent, this);
  th6 = std::thread(&CupDAQManager::TF_TriggerMon, this);
  th7 = std::thread(&CupDAQManager::TF_SplitOutput, this, false);
  th8 = std::thread(&CupDAQManager::TF_ShrinkToFit, this);
  if (fVerboseLevel > 0) { th9 = std::thread(&CupDAQManager::TF_DebugMon, this); }

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

  if (th9.joinable()) th9.join();
  if (th8.joinable()) th8.join();
  if (th7.joinable()) th7.join();
  if (th6.joinable()) th6.join();
  if (th5.joinable()) th5.join();
  if (th4.joinable()) th4.join();
  if (th3.joinable()) th3.join();
  if (th2.joinable()) th2.join();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kPROCENDED);

  if (th1.joinable()) th1.join();

TERMINATE:
  CloseDAQ();

  if (fTCB) {
    fTCB->Close();
    delete fTCB;
    fTCB = nullptr;
  }

  INFO("TCBDAQ ended");
}
