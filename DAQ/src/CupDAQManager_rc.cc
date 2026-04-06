#include <filesystem>

#include "TMessage.h"
#include "TServerSocket.h"

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
  INFO("TCB controller now starting [run=%d]", fRunNumber);

  auto * daq = static_cast<DAQConf *>(fConfigList->GetDAQConfig());
  fDAQName = daq->GetDAQName(fDAQID);
  fDAQPort = daq->GetPort(fDAQID);

  std::thread th0(&CupDAQManager::TF_MsgServer, this);
  std::thread th1;

  auto execute_run = [&]() {
    bool socketerror = false;

    // prepare client sockets correspond to DAQs (ZeroMQ REQ)
    for (int i = 0; i < daq->GetN(); i++) {
      int id = daq->GetID(i);
      if (id == fDAQID) { continue; }

      std::string ip = daq->GetIPAddr(id);
      int port = daq->GetPort(id);
      std::string daq_name = daq->GetDAQName(id);
      std::string endpoint = "tcp://" + ip + ":" + std::to_string(port);

      auto socket = std::make_unique<zmq::socket_t>(fZMQContext, zmq::socket_type::req);
      socket->set(zmq::sockopt::req_relaxed, 1);
      socket->set(zmq::sockopt::rcvtimeo, 2000);
      socket->set(zmq::sockopt::sndtimeo, 2000);
      socket->connect(endpoint);

      nlohmann::json ping_json;
      ping_json["command"] = "kQUERYDAQSTATUS";
      std::string ping_str = ping_json.dump();

      zmq::message_t request(ping_str.size());
      std::memcpy(request.data(), ping_str.c_str(), ping_str.size());

      // Log exactly before sending to track the crash point
      INFO("[%s] Sending kQUERYDAQSTATUS to %s", daq_name.c_str(), endpoint.c_str());

      auto send_res = socket->send(request, zmq::send_flags::none);
      if (!send_res) {
        socketerror = true;
        ERROR("[%s] Send failed at %s", daq_name.c_str(), endpoint.c_str());
        break;
      }

      // Log before receiving
      INFO("[%s] Waiting for reply from %s", daq_name.c_str(), endpoint.c_str());

      zmq::message_t reply;
      auto recv_res = socket->recv(reply, zmq::recv_flags::none);

      if (!recv_res) {
        socketerror = true;
        ERROR("[%s] Connection failed (timeout) at %s", daq_name.c_str(), endpoint.c_str());
        break;
      }

      fDAQSocket.push_back(std::move(socket));
      INFO("[%s] Connected and verified at %s", daq_name.c_str(), endpoint.c_str());
    }
    if (socketerror) {
      RUNSTATE::SetError(fRunStatusTCB);
      return;
    }

    // checking DAQs' status == kBOOTED
    if (!WaitDAQStatus(RUNSTATE::kBOOTED)) {
      RUNSTATE::SetError(fRunStatusTCB);
      return;
    }
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kBOOTED);
    INFO("all DAQs were booted");

    // wait for config run command
    int state = WaitCommand(fDoConfigRunTCB, fDoExitTCB);
    if (state != 0) {
      if (state == 1) { INFO("run=%d exited by Run Control", fRunNumber); }
      else if (state < 0) {
        RUNSTATE::SetError(fRunStatusTCB);
      }
      return;
    }

    fTCB = std::make_unique<CupGeneralTCB>();
    fTCB->SetConfig(fConfigList);
    if (fTCB->Open() != 0) {
      RUNSTATE::SetError(fRunStatusTCB);
      return;
    }

    if (!fTCB->Config()) {
      RUNSTATE::SetError(fRunStatusTCB);
      return;
    }

    //// generating TServersocket
    int config_port = fDAQPort + PORT_OFFSET::CONFIG;
    auto * configServer = new TServerSocket(config_port, kTRUE);
    if (!configServer->IsValid()) {
      ERROR("Failed to start TServerSocket on port %d", config_port);
      RUNSTATE::SetError(fRunStatusTCB);
      return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    SendCommandToDAQs("kCONFIGRUN");

    int expected_clients = 0;
    for (int i = 0; i < daq->GetN(); i++) {
      int id = daq->GetID(i);
      std::string name = daq->GetDAQName(id);

      // If the name contains "DAQ", it is a client
      if (name.find("DAQ") != std::string::npos) { expected_clients++; }
    }

    INFO("Waiting for %d DAQ clients to connect for configuration...", expected_clients);

    int connected_clients = 0;
    for (int i = 0; i < expected_clients; i++) {
      TSocket * sock = configServer->Accept();
      if (sock && sock->IsValid()) {
        TMessage * req = nullptr;
        if (sock->Recv(req) > 0 && req != nullptr) {
          char client_name[256];
          req->ReadString(client_name, 256);
          INFO("Config Server accepted connection from: %s", client_name);
          delete req;
        }
        else {
          WARNING("Config Server accepted connection from UNKNOWN client");
        }

        TMessage mess(kMESS_OBJECT);
        mess.WriteObject(fConfigList);
        sock->Send(mess);

        delete sock;
        connected_clients++;
      }
    }

    if (connected_clients != expected_clients) {
      ERROR("Not all DAQs connected for configuration (%d/%d)", connected_clients,
            expected_clients);
      delete configServer;
      RUNSTATE::SetError(fRunStatusTCB);
      return;
    }

    delete configServer; // Close server socket after all DAQs received the list

    // checking DAQs' status == kCONFIGURED
    if (!WaitDAQStatus(RUNSTATE::kCONFIGURED)) {
      RUNSTATE::SetError(fRunStatusTCB);
      return;
    }
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kCONFIGURED);
    INFO("all DAQs were configured");

    th1 = std::thread(&CupDAQManager::TF_SplitOutput, this, true);

    // wait for start run command
    state = WaitCommand(fDoStartRunTCB, fDoExitTCB);
    if (state != 0) {
      if (state == 1) { INFO("run=%d exited by Run Control", fRunNumber); }
      else if (state < 0) {
        RUNSTATE::SetError(fRunStatusTCB);
      }
      return;
    }

    fTCB->StartTrigger();
    time(&fStartDatime);

    SendCommandToDAQs("kSTARTRUN");

    // checking DAQs' status == kRUNNING
    if (!WaitDAQStatus(RUNSTATE::kRUNNING)) {
      RUNSTATE::SetError(fRunStatusTCB);
      return;
    }
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kRUNNING);
    INFO("all DAQs are running");

    while (true) {
      if (fDoEndRunTCB || IsForcedEndRunFile()) { break; }
      if (!IsDAQRunning()) { break; }
      if (fDoSplitOutputFile) {
        SendCommandToDAQs("kSPLITOUTPUTFILE");
        fDoSplitOutputFile = false;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    fTCB->StopTrigger();
    time(&fEndDatime);

    SendCommandToDAQs("kENDRUN");

    // checking DAQs' status == kRUNENDED
    if (!WaitDAQStatus(RUNSTATE::kRUNENDED)) {
      RUNSTATE::SetError(fRunStatusTCB);
      return;
    }
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kRUNENDED);
    INFO("run=%06d was ended", fRunNumber);

    if (!WaitDAQStatus(RUNSTATE::kPROCENDED)) {
      RUNSTATE::SetError(fRunStatusTCB);
      return;
    }
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kPROCENDED);
    INFO("all DAQ processes are ended");
  };

  execute_run();

  WaitCommand(fDoExitTCB);

  if (th1.joinable()) { th1.join(); }

  SendCommandToDAQs("kEXIT");
  fDAQSocket.clear();

  if (th0.joinable()) { th0.join(); }

  if (fTCB) { fTCB->Close(); }

  INFO("TCB controller ended");
}

void CupDAQManager::RC_TCBCTRLDAQ()
{
  fMonitorServerOn = fDoSendEvent ? 0 : 1;
  INFO("TCB controlled DAQ now starting");

  std::thread th0, th1, th2, th3, th4, th5, th6, th7, th8;

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
        fMergeServerPort = daq->GetPort(id) + PORT_OFFSET::DATA;
      }
    }
  }

  if (WaitCommand(fDoConfigRun, fDoExit) != 0) {
    if (th0.joinable()) { th0.join(); }
    WARNING("run=%d exited by TCB", fRunNumber);
    return;
  }

  // ----------------------------------------------
  // Request fConfigList to server and receive
  // ----------------------------------------------
  std::string tcb_ip = daq->GetIPAddr(0);
  int config_port = daq->GetPort(0) + PORT_OFFSET::CONFIG;

  TSocket * configSock = nullptr;

  // Retry logic to ensure connection even if server is slightly delayed
  for (int retry = 0; retry < 10; ++retry) {
    configSock = new TSocket(tcb_ip.c_str(), config_port);
    if (configSock->IsValid()) { break; }

    delete configSock;
    configSock = nullptr;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  if (!configSock || !configSock->IsValid()) {
    ERROR("Failed to connect to TCB Config Server at %s:%d", tcb_ip.c_str(), config_port);
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  // 1. Send Handshake to Server
  TMessage req(kMESS_STRING);
  TString myName = fDAQName.c_str();
  req.WriteString(myName.Data());
  configSock->Send(req);

  // 2. Receive updated fConfigList
  TMessage * mess = nullptr;
  if (configSock->Recv(mess) <= 0 || mess == nullptr) {
    ERROR("Failed to receive fConfigList message from TCB");
    delete configSock;
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  // Read object from TMessage and cast it back
  fConfigList = (decltype(fConfigList))mess->ReadObject(mess->GetClass());

  delete mess;
  delete configSock;
  INFO("Successfully received updated fConfigList from TCB");
  // -------------------------------------------------------------------

  auto execute_run = [&]() {
    if (!AddADC(fConfigList)) {
      RUNSTATE::SetError(fRunStatus);
      return;
    }
    if (!PrepareDAQ()) {
      RUNSTATE::SetError(fRunStatus);
      return;
    }
    if (!OpenDAQ()) {
      RUNSTATE::SetError(fRunStatus);
      return;
    }

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
      return;
    }

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
    time(&fStartDatime);

    if (WaitCommand(fDoEndRun, fRunStatus) != 0) {
      WARNING("run=%d ended by error state", fRunNumber);
    }

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
    time(&fEndDatime);
  };

  execute_run();

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

  std::thread th0, th1, th2, th3, th4, th5, th6, th7;

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

  auto execute_run = [&]() {
    th1 = std::thread(&CupDAQManager::TF_RunManager, this);
    th2 = std::thread(&CupDAQManager::TF_MergeEvent, this);
    th3 = std::thread(&CupDAQManager::TF_WriteEvent, this);
    th4 = std::thread(&CupDAQManager::TF_TriggerMon, this);
    th5 = std::thread(&CupDAQManager::TF_ShrinkToFit, this);
    th6 = std::thread(&CupDAQManager::TF_DataServer, this);

    if (fVerboseLevel > 0) { th7 = std::thread(&CupDAQManager::TF_DebugMon, this); }

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

    if (WaitCommand(fDoStartRun, fDoExit) != 0) {
      WARNING("run=%d exited by TCB", fRunNumber);
      return;
    }

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
    time(&fStartDatime);

    if (WaitCommand(fDoEndRun, fRunStatus) != 0) {
      WARNING("run=%d ended by error state", fRunNumber);
    }

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
    time(&fEndDatime);
  };

  execute_run();

  if (th7.joinable()) th7.join();
  if (th6.joinable()) th6.join();
  if (th5.joinable()) th5.join();
  if (th4.joinable()) th4.join();
  if (th3.joinable()) th3.join();
  if (th2.joinable()) th2.join();

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

  if (!AddADC(fConfigList)) { return; }
  if (!OpenDAQ()) { return; }

  std::thread th1, th2, th3, th4, th5, th6, th7, th8, th9;

  auto execute_run = [&]() {
    if (!ConfigureDAQ()) { return; }
    if (!PrepareDAQ()) { return; }
    if (!InitializeDAQ()) { return; }

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
  };

  execute_run();

  CloseDAQ();

  INFO("STDDAQ ended");
}

void CupDAQManager::RC_TCBDAQ()
{
  INFO("TCBDAQ now starting");

  fTCB = std::make_unique<CupGeneralTCB>();
  fTCB->SetConfig(fConfigList);
  fTCB->SetADCType(fADCType);

  if (fTCB->Open() != 0) {
    fTCB.reset();
    return;
  }

  std::thread th1, th2, th3, th4, th5, th6, th7, th8, th9;

  auto execute_run = [&]() {
    if (!fTCB->Config()) { return; }
    if (!AddADC(fConfigList)) { return; }
    if (!PrepareDAQ()) { return; }
    if (!OpenDAQ()) { return; }

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
  };

  execute_run();

  CloseDAQ();

  if (fTCB) {
    fTCB->Close();
    fTCB.reset();
  }

  INFO("TCBDAQ ended");
}