#include <filesystem>
#include <thread>

#include "TMessage.h"
#include "TServerSocket.h"

#include "DAQ/CupDAQManager.hh"
#include "DAQConfig/RunConfig.hh"
#include "DAQUtils/ELog.hh"

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

  auto execute_run = [&]()
  {
    bool socketerror = false;

    // Max retries and wait interval (ms)
    const int MAX_RETRIES = 5;
    const int RETRY_WAIT_MS = 1000;

    for (int i = 0; i < daq->GetN(); i++) {
      int id = daq->GetID(i);
      if (id == fDAQID) { continue; }

      std::string ip = daq->GetIPAddr(id);
      int port = daq->GetPort(id);
      std::string daq_name = daq->GetDAQName(id);
      std::string endpoint = "tcp://" + ip + ":" + std::to_string(port);

      auto socket = std::make_unique<zmq::socket_t>(fZMQContext, zmq::socket_type::req);

      // Set socket options for resilient connection
      socket->set(zmq::sockopt::req_relaxed, 1);
      socket->set(zmq::sockopt::rcvtimeo, 2000);
      socket->set(zmq::sockopt::sndtimeo, 2000);
      socket->set(zmq::sockopt::linger, 0); // Avoid hanging on close

      socket->connect(endpoint);

      nlohmann::json ping_json;
      ping_json["command"] = "kQUERYDAQSTATUS";
      std::string ping_str = ping_json.dump();

      bool connected_and_verified = false;

      for (int retry = 0; retry < MAX_RETRIES; retry++) {
        INFO("[%s] Attempting to connect... (%d/%d) at %s", daq_name.c_str(), retry + 1,
             MAX_RETRIES, endpoint.c_str());

        zmq::message_t request(ping_str.size());
        std::memcpy(request.data(), ping_str.c_str(), ping_str.size());

        auto send_res = socket->send(request, zmq::send_flags::none);
        if (!send_res) {
          WARNING("[%s] Send failed, retrying...", daq_name.c_str());
          std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_WAIT_MS));
          continue;
        }

        zmq::message_t reply;
        auto recv_res = socket->recv(reply, zmq::recv_flags::none);
        if (!recv_res) {
          WARNING("[%s] Receive timeout, server might be starting up...", daq_name.c_str());
          std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_WAIT_MS));
          continue;
        }

        // Successfully received a reply
        connected_and_verified = true;
        break;
      }

      if (!connected_and_verified) {
        socketerror = true;
        ERROR("[%s] Failed to connect after %d attempts at %s", daq_name.c_str(), MAX_RETRIES,
              endpoint.c_str());
        // Depending on system requirements, you might 'continue' to check other nodes
        // or 'break' if all nodes are mandatory.
        continue;
      }

      fDAQSocket.push_back(std::move(socket));
      INFO("[%s] Connected and verified at %s", daq_name.c_str(), endpoint.c_str());
    }

    if (socketerror) {
      RUNSTATE::SetError(fRunStatusTCB);
      SendCommandToDAQs("kSETERROR");
      return;
    }

    if (!WaitDAQStatus(RUNSTATE::kBOOTED)) {
      RUNSTATE::SetError(fRunStatusTCB);
      SendCommandToDAQs("kSETERROR");
      return;
    }
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kBOOTED);
    INFO("all DAQs were booted");

    int state = WaitCommand(fDoConfigRunTCB, fDoExitTCB, fRunStatusTCB);
    if (state != 0) {
      if (state == 1) { INFO("run=%d exited by Run Control", fRunNumber); }
      else {
        RUNSTATE::SetError(fRunStatusTCB);
        SendCommandToDAQs("kSETERROR");
      }
      return;
    }

    fTCB = std::make_unique<CupGeneralTCB>();
    fTCB->SetConfig(fConfigList);
    if (fTCB->Open() != 0) {
      RUNSTATE::SetError(fRunStatusTCB);
      SendCommandToDAQs("kSETERROR");
      return;
    }

    if (!fTCB->Config()) {
      RUNSTATE::SetError(fRunStatusTCB);
      SendCommandToDAQs("kSETERROR");
      return;
    }

    int config_port = fDAQPort + PORT_OFFSET::CONFIG;
    auto * configServer = new TServerSocket(config_port, kTRUE);
    if (!configServer->IsValid()) {
      ERROR("Failed to start TServerSocket on port %d", config_port);
      RUNSTATE::SetError(fRunStatusTCB);
      SendCommandToDAQs("kSETERROR");
      return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    SendCommandToDAQs("kCONFIGRUN");

    int expected_clients = 0;
    for (int i = 0; i < daq->GetN(); i++) {
      int id = daq->GetID(i);
      std::string name = daq->GetDAQName(id);
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
      SendCommandToDAQs("kSETERROR");
      return;
    }

    delete configServer;

    if (!WaitDAQStatus(RUNSTATE::kCONFIGURED)) {
      RUNSTATE::SetError(fRunStatusTCB);
      SendCommandToDAQs("kSETERROR");
      return;
    }
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kCONFIGURED);
    INFO("all DAQs were configured");

    th1 = std::thread(&CupDAQManager::TF_SplitOutput, this, true);

    state = WaitCommand(fDoStartRunTCB, fDoExitTCB, fRunStatusTCB);
    if (state != 0) {
      if (state == 1) { INFO("run=%d exited by Run Control", fRunNumber); }
      else {
        RUNSTATE::SetError(fRunStatusTCB);
      }
      fDoEndRunTCB.store(true);
      return;
    }

    fTCB->StartTrigger();
    time(&fStartDatime);
    SendCommandToDAQs("kSTARTRUN");

    if (!WaitDAQStatus(RUNSTATE::kRUNNING)) {
      RUNSTATE::SetError(fRunStatusTCB);
      SendCommandToDAQs("kSETERROR");
      return;
    }
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kRUNNING);
    INFO("all DAQs are running");

    while (true) {
      if (fDoEndRunTCB.load() || IsForcedEndRunFile()) { break; }
      if (!CheckDAQStatus(RUNSTATE::kRUNNING)) {
        RUNSTATE::SetError(fRunStatusTCB);
        SendCommandToDAQs("kSETERROR");
        return;
      }
      if (fDoExitTCB.load()) { return; }
      if (fDoSplitOutputFile.load()) {
        SendCommandToDAQs("kSPLITOUTPUTFILE");
        fDoSplitOutputFile.store(false);
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    fTCB->StopTrigger();
    time(&fEndDatime);
    SendCommandToDAQs("kENDRUN");

    if (!WaitDAQStatus(RUNSTATE::kRUNENDED)) {
      RUNSTATE::SetError(fRunStatusTCB);
      SendCommandToDAQs("kSETERROR");
      return;
    }
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kRUNENDED);
    INFO("run=%06d was ended", fRunNumber);

    if (!WaitDAQStatus(RUNSTATE::kPROCENDED)) {
      RUNSTATE::SetError(fRunStatusTCB);
      SendCommandToDAQs("kSETERROR");
      return;
    }
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kPROCENDED);
    INFO("all DAQ processes are ended");
  };

  execute_run();

  WaitCommand(fDoExitTCB, fDoExitTCB, fRunStatusTCB);

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

  if (fDoSendEvent) {
    std::string target_name = GetADCName(fADCType);
    target_name += "MERGER";

    for (int i = 0; i < daq->GetN(); i++) {
      int id = daq->GetID(i);
      const std::string & daq_name = daq->GetDAQName(id);
      if (daq_name.find(target_name) != std::string::npos) {
        fMergeServerHost = daq->GetIPAddr(id);
        fMergeServerPort = daq->GetPort(id) + PORT_OFFSET::DATA;
      }
    }
  }

  RUNSTATE::SetState(fRunStatus, RUNSTATE::kBOOTED);

  int state = WaitCommand(fDoConfigRun, fDoExit, fRunStatus);
  if (state != 0) {
    if (state > 0) { WARNING("run=%d exited by TCB", fRunNumber); }
    else {
      ERROR("run=%d exit caused error", fRunNumber);
    }
    if (th0.joinable()) { th0.join(); }
    return;
  }

  std::string tcb_ip = daq->GetIPAddr(0);
  int config_port = daq->GetPort(0) + PORT_OFFSET::CONFIG;

  TSocket * configSock = nullptr;
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
    if (th0.joinable()) { th0.join(); }
    return;
  }

  TMessage req(kMESS_STRING);
  TString myName = fDAQName.c_str();
  req.WriteString(myName.Data());
  configSock->Send(req);

  TMessage * mess = nullptr;
  if (configSock->Recv(mess) <= 0 || mess == nullptr) {
    ERROR("Failed to receive fConfigList message from TCB");
    delete configSock;
    RUNSTATE::SetError(fRunStatus);
    if (th0.joinable()) { th0.join(); }
    return;
  }

  fConfigList = (decltype(fConfigList))mess->ReadObject(mess->GetClass());
  delete mess;
  delete configSock;
  INFO("Successfully received updated fConfigList from TCB");

  auto execute_run = [&]()
  {
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
      th5 = std::thread(&CupDAQManager::TF_SendData, this);
    }

    th6 = std::thread(&CupDAQManager::TF_TriggerMon, this);
    th7 = std::thread(&CupDAQManager::TF_ShrinkToFit, this);
    if (fVerboseLevel > 0) { th8 = std::thread(&CupDAQManager::TF_DebugMon, this); }

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

    int state = WaitCommand(fDoStartRun, fDoExit, fRunStatus);
    if (state != 0) {
      if (state > 0) { WARNING("run=%d exited by TCB", fRunNumber); }
      else {
        ERROR("run=%d exit caused error", fRunNumber);
      }
      return;
    }

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
    time(&fStartDatime);

    state = WaitCommand(fDoEndRun, fDoExit, fRunStatus);
    if (state != 0) {
      if (state > 0) { WARNING("run=%d exited by TCB", fRunNumber); }
      else {
        ERROR("run=%d exit caused error", fRunNumber);
      }
      return;
    }

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDING);
    INFO("changed run state to RUNENDING");

    while (true) {
      bool allEnded = fReadStatus.load() == ENDED && fSortStatus.load() == ENDED &&
                      fBuildStatus.load() == ENDED &&
                      (fDoSendEvent ? fSendStatus.load() : fWriteStatus.load()) == ENDED;

      if (allEnded) break;

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
    time(&fEndDatime);
  };

  execute_run();

  if (th8.joinable()) { th8.join(); }
  if (th7.joinable()) { th7.join(); }
  if (th6.joinable()) { th6.join(); }
  if (th5.joinable()) { th5.join(); }
  if (th4.joinable()) { th4.join(); }
  if (th3.joinable()) { th3.join(); }
  if (th2.joinable()) { th2.join(); }

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kPROCENDED);

  if (th1.joinable()) { th1.join(); }

  WaitCommand(fDoExit, fDoExit, fRunStatus);

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
    if (id == fDAQID) { continue; }

    const std::string & daq_name = daq->GetDAQName(id);
    if (daq_name.find(adcname) != std::string::npos) {
      fRecvEventBuffers[id] = std::make_unique<BuiltEventBuffer>();
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

  int state = WaitCommand(fDoConfigRun, fDoExit, fRunStatus);
  if (state != 0) {
    if (state > 0) { WARNING("run=%d exited by TCB", fRunNumber); }
    else {
      ERROR("run=%d exit caused error", fRunNumber);
    }
    if (th0.joinable()) { th0.join(); }
    return;
  }

  auto execute_run = [&]()
  {
    th1 = std::thread(&CupDAQManager::TF_RunManager, this);
    th2 = std::thread(&CupDAQManager::TF_BuildEvent, this);
    th3 = std::thread(&CupDAQManager::TF_WriteEvent, this);
    th4 = std::thread(&CupDAQManager::TF_TriggerMon, this);
    th5 = std::thread(&CupDAQManager::TF_ShrinkToFit, this);
    th6 = std::thread(&CupDAQManager::TF_DataServer, this);
    if (fVerboseLevel > 0) { th7 = std::thread(&CupDAQManager::TF_DebugMon, this); }

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

    state = WaitCommand(fDoStartRun, fDoExit, fRunStatus);
    if (state != 0) {
      if (state > 0) { WARNING("run=%d exited by TCB", fRunNumber); }
      else {
        ERROR("run=%d exit caused error", fRunNumber);
      }
      return;
    }

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
    time(&fStartDatime);

    state = WaitCommand(fDoEndRun, fDoExit, fRunStatus);
    if (state != 0) {
      if (state > 0) { WARNING("run=%d exited by TCB", fRunNumber); }
      else {
        ERROR("run=%d exit caused error", fRunNumber);
      }
      return;
    }

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDING);
    INFO("changed run state to RUNENDING");

    while (true) {
      bool allEnded = fRecvStatus.load() == ENDED && fBuildStatus.load() == ENDED &&
                      fWriteStatus.load() == ENDED;

      if (allEnded) break;

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
    time(&fEndDatime);
  };

  execute_run();

  if (th7.joinable()) { th7.join(); }
  if (th6.joinable()) { th6.join(); }
  if (th5.joinable()) { th5.join(); }
  if (th4.joinable()) { th4.join(); }
  if (th3.joinable()) { th3.join(); }
  if (th2.joinable()) { th2.join(); }

  fRecvEventBuffers.clear();

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kPROCENDED);

  if (th1.joinable()) { th1.join(); }

  WaitCommand(fDoExit, fDoExit, fRunStatus);

  if (th0.joinable()) { th0.join(); }

  INFO("event merger ended");
}

void CupDAQManager::RC_STDDAQ()
{
  INFO("STDDAQ now starting");

  if (!AddADC(fConfigList)) { return; }
  if (!OpenDAQ()) { return; }

  std::thread th1, th2, th3, th4, th5, th6, th7, th8, th9;

  auto execute_run = [&]()
  {
    if (!ConfigureDAQ()) { return; }
    if (!PrepareDAQ()) { return; }
    if (!InitializeDAQ()) { return; }

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

    if (th9.joinable()) { th9.join(); }
    if (th8.joinable()) { th8.join(); }
    if (th7.joinable()) { th7.join(); }
    if (th6.joinable()) { th6.join(); }
    if (th5.joinable()) { th5.join(); }
    if (th4.joinable()) { th4.join(); }
    if (th3.joinable()) { th3.join(); }
    if (th2.joinable()) { th2.join(); }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    RUNSTATE::SetState(fRunStatus, RUNSTATE::kPROCENDED);

    if (th1.joinable()) { th1.join(); }
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

  auto execute_run = [&]()
  {
    if (!fTCB->Config()) { return; }
    if (!AddADC(fConfigList)) { return; }
    if (!PrepareDAQ()) { return; }
    if (!OpenDAQ()) { return; }

    std::thread th1 = std::thread(&CupDAQManager::TF_RunManager, this);
    std::thread th2 = std::thread(&CupDAQManager::TF_ReadData, this);
    std::thread th3 = std::thread(&CupDAQManager::TF_SortEvent, this);
    std::thread th4 = std::thread(&CupDAQManager::TF_BuildEvent, this);
    std::thread th5 = std::thread(&CupDAQManager::TF_WriteEvent, this);
    std::thread th6 = std::thread(&CupDAQManager::TF_TriggerMon, this);
    std::thread th7 = std::thread(&CupDAQManager::TF_SplitOutput, this, false);
    std::thread th8 = std::thread(&CupDAQManager::TF_ShrinkToFit, this);
    std::thread th9;
    if (fVerboseLevel > 0) { th9 = std::thread(&CupDAQManager::TF_DebugMon, this); }
    std::thread th10;
    if (fDoHistograming) { th10 = std::thread(&CupDAQManager::TF_Histogramer, this); }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

    if (th10.joinable()) { th10.join(); }
    if (th9.joinable()) { th9.join(); }
    if (th8.joinable()) { th8.join(); }
    if (th7.joinable()) { th7.join(); }
    if (th6.joinable()) { th6.join(); }
    if (th5.joinable()) { th5.join(); }
    if (th4.joinable()) { th4.join(); }
    if (th3.joinable()) { th3.join(); }
    if (th2.joinable()) { th2.join(); }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    RUNSTATE::SetState(fRunStatus, RUNSTATE::kPROCENDED);

    if (th1.joinable()) { th1.join(); }
  };

  execute_run();

  CloseDAQ();

  if (fTCB) {
    fTCB->Close();
    fTCB.reset();
  }

  INFO("TCBDAQ ended");
}