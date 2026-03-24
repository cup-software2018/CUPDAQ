#include "TMessage.h"

#include "DAQ/CupDAQManager.hh"
#include "DAQConfig/RunConfig.hh"

void CupDAQManager::RC_NullTCB()
{
  INFO("NULLTCB controller now starting [run=%d]", fRunNumber);

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

      try {
        auto socket = std::make_unique<zmq::socket_t>(fZMQContext, zmq::socket_type::req);
        socket->set(zmq::sockopt::rcvtimeo, 2000);
        socket->set(zmq::sockopt::sndtimeo, 2000);
        socket->connect(endpoint);

        nlohmann::json ping_json;
        ping_json["command"] = "kQUERYDAQSTATUS";
        std::string ping_str = ping_json.dump();

        zmq::message_t request(ping_str.size());
        std::memcpy(request.data(), ping_str.c_str(), ping_str.size());

        if (!socket->send(request, zmq::send_flags::none)) {
          throw std::runtime_error("Send failed");
        }

        zmq::message_t reply;
        if (!socket->recv(reply, zmq::recv_flags::none)) {
          socketerror = true;
          ERROR("%s connection failed (timeout) at %s", daq_name.c_str(), endpoint.c_str());
          break;
        }

        fDAQSocket.push_back(std::move(socket));
        INFO("%s connected and verified at %s", daq_name.c_str(), endpoint.c_str());
      }
      catch (const std::exception & e) {
        socketerror = true;
        ERROR("%s connection failed at %s: %s", daq_name.c_str(), endpoint.c_str(), e.what());
        break;
      }
    }

    if (socketerror) {
      RUNSTATE::SetError(fRunStatusTCB);
      return;
    }

    if (!WaitDAQStatus(RUNSTATE::kBOOTED)) {
      RUNSTATE::SetError(fRunStatusTCB);
      return;
    }
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kBOOTED);
    INFO("all DAQs were booted");

    int state = WaitCommand(fDoConfigRunTCB, fDoExitTCB);
    if (state != 0) {
      if (state == 1) { INFO("run=%d exited by Run Control", fRunNumber); }
      else if (state < 0) {
        RUNSTATE::SetError(fRunStatusTCB);
      }
      return;
    }

    SendCommandToDAQs("kCONFIGRUN");

    if (!WaitDAQStatus(RUNSTATE::kCONFIGURED)) {
      RUNSTATE::SetError(fRunStatusTCB);
      return;
    }
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kCONFIGURED);
    INFO("all DAQs were configured");

    th1 = std::thread(&CupDAQManager::TF_SplitOutput, this, true);

    state = WaitCommand(fDoStartRunTCB, fDoExitTCB);
    if (state != 0) {
      if (state == 1) { INFO("run=%d exited by Run Control", fRunNumber); }
      else if (state < 0) {
        RUNSTATE::SetError(fRunStatusTCB);
      }
      return;
    }

    time(&fStartDatime);
    SendCommandToDAQs("kSTARTRUN");

    if (!WaitDAQStatus(RUNSTATE::kRUNNING)) {
      RUNSTATE::SetError(fRunStatusTCB);
      return;
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
        SendCommandToDAQs("kSPLITOUTPUTFILE");
        fDoSplitOutputFile = false;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    time(&fEndDatime);
    SendCommandToDAQs("kENDRUN");

    WaitDAQStatus(RUNSTATE::kRUNENDED);
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kRUNENDED);
    INFO("run=%06d was ended", fRunNumber);

    WaitDAQStatus(RUNSTATE::kPROCENDED);
    RUNSTATE::SetState(fRunStatusTCB, RUNSTATE::kPROCENDED);
    INFO("all DAQ processes are ended");
  };

  execute_run();

  WaitCommand(fDoExitTCB);

  if (th1.joinable()) { th1.join(); }

  SendCommandToDAQs("kEXIT");
  fDAQSocket.clear();

  if (th0.joinable()) { th0.join(); }

  INFO("TCB controller ended");
}

void CupDAQManager::RC_NullDAQ()
{
  fMonitorServerOn = fDoSendEvent ? 0 : 1;
  INFO("TCB controlled DAQ now starting");

  std::thread th0, th1, th2;

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

  auto execute_run = [&]() {
    th1 = std::thread(&CupDAQManager::TF_RunManager, this);
    th2 = std::thread(&CupDAQManager::TF_TriggerMon, this);

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

    if (WaitCommand(fDoStartRun, fDoExit) != 0) {
      WARNING("run=%d exited by TCB", fRunNumber);
      return;
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
  };

  execute_run();

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

  std::thread th0, th1, th2;

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

  auto execute_run = [&]() {
    th1 = std::thread(&CupDAQManager::TF_RunManager, this);
    th2 = std::thread(&CupDAQManager::TF_TriggerMon, this);

    RUNSTATE::SetState(fRunStatus, RUNSTATE::kCONFIGURED);

    if (WaitCommand(fDoStartRun, fDoExit) != 0) {
      WARNING("run=%d exited by TCB", fRunNumber);
      return;
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
  };

  execute_run();

  if (th2.joinable()) { th2.join(); }

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  RUNSTATE::SetState(fRunStatus, RUNSTATE::kPROCENDED);

  if (th1.joinable()) { th1.join(); }

  WaitCommand(fDoExit);

  if (th0.joinable()) { th0.join(); }

  INFO("event merger ended");
}