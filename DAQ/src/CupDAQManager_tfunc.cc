#include <algorithm>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <zmq.hpp>

#include "TMessage.h"
#include "TMonitor.h"
#include "TServerSocket.h"

#include "DAQ/CupDAQManager.hh"
#include "OnlHistogramer/AbsHistogramer.hh"
#include "OnlHistogramer/FADCHistogramer.hh"
#include "OnlHistogramer/SADCHistogramer.hh"

void CupDAQManager::TF_RunManager()
{
  INFO("run manager started");

  if (!WaitState(fRunStatus, RUNSTATE::kCONFIGURED)) {
    WARNING("exited by error state");
    return;
  }

  if (fDAQType == DAQ::TCBDAQ) {
    fTCB->StartTrigger();
    time(&fStartDatime);
    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
  }
  else if (fDAQType == DAQ::STDDAQ) {
    StartTrigger();
    time(&fStartDatime);
    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNNING);
  }

  bool iendrun = false;
  std::unique_lock<std::mutex> mlock(fMonitorMutex, std::defer_lock);

  while (true) {
    if (fDoExit) { break; }

    if (IsForcedEndRunFile()) {
      INFO("daq will be ended by ENDRUN command");
      fDoEndRun = true;
      break;
    }

    if (fSetNEvent > 0 || fSetDAQTime > 0) {
      mlock.lock();
      int triggernumber = fTriggerNumber;
      double triggertime = fTriggerTime / kDONESECOND;
      mlock.unlock();

      if (fSetNEvent > 0 && triggernumber > fSetNEvent) { iendrun = true; }
      if (fSetDAQTime > 0 && triggertime > fSetDAQTime) { iendrun = true; }
      if (iendrun) {
        INFO("daq will be ended by preset condition");
        fDoEndRun = true;
        break;
      }
    }

    if (RUNSTATE::CheckError(fRunStatus)) {
      WARNING("daq will be ended by error");
      fDoEndRun = true;
      break;
    }

    if (fDoEndRun) {
      INFO("daq will be ended by ENDRUN command");
      break;
    }

    if (fDoSplitOutputFile && !fDoSendEvent) {
      if (!OpenNewOutputFile()) { RUNSTATE::SetError(fRunStatus); }
      fDoSplitOutputFile = false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  if (fDAQType == DAQ::TCBDAQ) {
    fTCB->StopTrigger();
    time(&fEndDatime);
    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
  }
  else if (fDAQType == DAQ::STDDAQ) {
    StopTrigger();
    time(&fEndDatime);
    RUNSTATE::SetState(fRunStatus, RUNSTATE::kRUNENDED);
  }

  WaitState(fRunStatus, RUNSTATE::kPROCENDED, false);
  INFO("all processes ended");

  PrintDAQSummary();
  INFO("run manager ended");
}

void CupDAQManager::TF_TriggerMon()
{
  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }

  INFO("trigger monitoring started");

  unsigned int dummynevent = 0;
  double dummytime = 0.0;

  std::unique_lock<std::mutex> mlock(fMonitorMutex, std::defer_lock);

  TStopwatch sw;
  sw.Start();

  auto get_hms = [](double sec) {
    if (sec < 0) sec = 0;

    int h = static_cast<int>(sec) / 3600;
    int m = (static_cast<int>(sec) % 3600) / 60;
    double s = std::fmod(sec, 60.0);

    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%04.1f", h, m, s);
    return std::string(buf);
  };

  while (true) {
    double elapsetime = sw.RealTime();
    sw.Continue();

    if (elapsetime > fTriggerMonTime) {
      sw.Start(true);

      mlock.lock();
      int triggernumber = fTriggerNumber;
      double triggertime = fTriggerTime / kDONESECOND;
      mlock.unlock();

      double dtime = triggertime - dummytime;
      double dnevent = triggernumber - dummynevent;

      double insrate = dtime > 0.0 ? dnevent / dtime : 0.0;
      double accrate = triggertime > 0.0 ? triggernumber / triggertime : 0.0;

      STATS("%5d events triggered [%7d | %7d / %5.1f(%5.1f) Hz / %s]", static_cast<int>(dnevent),
            triggernumber, fNBuiltEvent, insrate, accrate, get_hms(triggertime).c_str());

      dummynevent = static_cast<unsigned int>(triggernumber);
      dummytime = triggertime;
    }

    bool runstate = RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDED) ||
                    RUNSTATE::CheckError(fRunStatus) || fDoExit;
    if (runstate) { break; }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  INFO("trigger monitoring ended");
}

void CupDAQManager::TF_DebugMon()
{
  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }

  const int nadc_int = GetEntries();
  const double debugmontime = 1.0;

  INFO("debug monitoring started");

  TStopwatch sw;
  sw.Start();

  while (true) {
    double elapsetime = sw.RealTime();
    sw.Continue();

    if (elapsetime > debugmontime) {
      sw.Start(true);

      std::string adcbcountsize;
      std::string adcbufsize;
      std::string sortbufsize;

      for (int i = 0; i < nadc_int; ++i) {
        adcbcountsize += Form("%5d ", fRemainingBCount[i]);

        auto * adc = static_cast<AbsADC *>(fCont[i]);
        adcbufsize += Form("%5d ", adc->Bsize());

        auto * modraw = fADCRawBuffers.at(static_cast<std::size_t>(i));
        sortbufsize += Form("%5d ", static_cast<int>(modraw->size()));
      }

      DEBUG("ADC bcount size: %s", adcbcountsize.c_str());
      DEBUG("ADC buffer size: %s", adcbufsize.c_str());
      DEBUG("sorting buffer size: %s", sortbufsize.c_str());
      DEBUG("building buffer size: %5zu %5zu", fBuiltEventBuffer1.size(),
            fBuiltEventBuffer2.size());
      DEBUG("%.3f(r) %.3f(s) %.3f(b) %.3f(w)", fReadSleep / 1000.0, fSortSleep / 1000.0,
            fBuildSleep / 1000.0, fWriteSleep / 1000.0);
    }

    bool runstate = RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDED) ||
                    RUNSTATE::CheckError(fRunStatus) || fDoExit;
    if (runstate) { break; }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  INFO("debug monitoring ended");
}

void CupDAQManager::TF_MsgServer()
{
  int port = fDAQPort;
  std::string name = fDAQName;
  bool istcb = (fDAQID == 0);

  // Initialize ZeroMQ context and socket in ROUTER mode instead of REP
  zmq::context_t context(1);
  zmq::socket_t zmq_socket(context, zmq::socket_type::router);

  // Set receive timeout to 1000ms.
  // This allows the while loop to check for exit flags periodically without blocking forever.
  zmq_socket.set(zmq::sockopt::rcvtimeo, 1000);

  std::string endpoint = "tcp://*:" + std::to_string(port);

  try {
    zmq_socket.bind(endpoint);
  }
  catch (const zmq::error_t & e) {
    ERROR("[%s] socket bind failed on port %d: %s", name.c_str(), port, e.what());
    istcb ? RUNSTATE::SetError(fRunStatusTCB) : RUNSTATE::SetError(fRunStatus);
    return;
  }

  INFO("[%s] Message Server started on port %d (ROUTER Mode)", name.c_str(), port);

  // Main Event Loop
  while (true) {
    // 1. Check exit flags
    if (fDoExit || fDoExitTCB) { break; }

    zmq::message_t identity;
    zmq::message_t delimiter;
    zmq::message_t request;

    // 2. Wait for a message from clients
    // In ROUTER mode, the first frame is ALWAYS the routing identity
    auto res = zmq_socket.recv(identity, zmq::recv_flags::none);

    // If timeout occurs (no message for 1000ms), loop again to check exit flags
    if (!res) { continue; }

    // Read the empty delimiter frame (mandatory in REQ-ROUTER pattern)
    if (identity.more()) { (void)zmq_socket.recv(delimiter, zmq::recv_flags::none); }
    else {
      WARNING("[%s] Invalid ROUTER frame received (no delimiter)", name.c_str());
      continue;
    }

    // Read the actual payload frame (JSON data)
    if (delimiter.more()) { (void)zmq_socket.recv(request, zmq::recv_flags::none); }
    else {
      WARNING("[%s] Invalid ROUTER frame received (no payload)", name.c_str());
      continue;
    }

    // Flush any remaining parts properly to prevent lingering messages
    bool has_more = request.more();
    while (has_more) {
      zmq::message_t dummy;
      (void)zmq_socket.recv(dummy, zmq::recv_flags::none);
      has_more = dummy.more();
      WARNING("[%s] Discarded extra multipart frame from client", name.c_str());
    }

    // 3. Parse the received message as JSON
    std::string msg_str(static_cast<char *>(request.data()), request.size());
    nlohmann::json req_json = nlohmann::json::parse(msg_str, nullptr, false);
    nlohmann::json rep_json;

    if (req_json.is_discarded()) {
      rep_json["status"] = "error";
      rep_json["message"] = "Invalid JSON format received";
      WARNING("[%s] Invalid JSON received from client", name.c_str());
    }
    else {
      std::string command = req_json.value("command", "UNKNOWN");
      rep_json["status"] = "ok"; // Default success status
      rep_json["name"] = name;   // Default success status

      // -------------------------------------------------------------------
      // Status & Information Queries
      // -------------------------------------------------------------------
      if (command == "kQUERYDAQSTATUS") {
        rep_json["run_status"] = istcb ? fRunStatusTCB : fRunStatus;
      }
      else if (command == "kQUERYRUNINFO") {
        rep_json["run_number"] = fRunNumber;
        rep_json["subrun_number"] = fSubRunNumber;
        rep_json["start_time"] = fStartDatime;
        rep_json["end_time"] = fEndDatime;
      }
      else if (command == "kQUERYTRGINFO") {
        // Protect shared variables with the existing monitor mutex
        std::lock_guard<std::mutex> lock(fMonitorMutex);
        rep_json["nevent"] = static_cast<unsigned long>(fTriggerNumber);
        rep_json["trgtime"] = static_cast<unsigned long>(fTriggerTime);
      }
      else if (command == "kQUERYMONITOR") {
        rep_json["monitor_server_on"] = fMonitorServerOn;
      }

      // -------------------------------------------------------------------
      // Run Control Commands
      // -------------------------------------------------------------------
      else if (command == "kCONFIGRUN") {
        istcb ? fDoConfigRunTCB = true : fDoConfigRun = true;
        INFO("[%s] CONFIGRUN command received", name.c_str());
      }
      else if (command == "kSTARTRUN") {
        istcb ? fDoStartRunTCB = true : fDoStartRun = true;
        INFO("[%s] STARTRUN command received", name.c_str());
      }
      else if (command == "kENDRUN") {
        istcb ? fDoEndRunTCB = true : fDoEndRun = true;
        INFO("[%s] ENDRUN command received", name.c_str());
      }
      else if (command == "kSPLITOUTPUTFILE") {
        istcb ? fDoSplitOutputFileTCB = true : fDoSplitOutputFile = true;
        INFO("[%s] SPLITOUTPUTFILE command received", name.c_str());
      }
      else if (command == "kSETERROR") {
        RUNSTATE::SetError(fRunStatus);
        INFO("[%s] SETERROR command received", name.c_str());
      }
      else if (command == "kEXIT") {
        istcb ? fDoExitTCB = true : fDoExit = true;
        INFO("[%s] EXIT command received", name.c_str());
      }
      // -------------------------------------------------------------------
      // Unknown Command
      // -------------------------------------------------------------------
      else {
        rep_json["status"] = "error";
        rep_json["message"] = "Unknown command string";
        WARNING("[%s] Unknown command [%s] received", name.c_str(), command.c_str());
      }
    }

    // 4. Send the JSON reply back to the client using ROUTER envelope structure
    std::string rep_str = rep_json.dump();
    zmq::message_t reply(rep_str.size());
    std::memcpy(reply.data(), rep_str.c_str(), rep_str.size());

    // Send the routing identity first
    zmq_socket.send(identity, zmq::send_flags::sndmore);

    // Send the empty delimiter
    zmq_socket.send(delimiter, zmq::send_flags::sndmore);

    // Send the actual payload
    zmq_socket.send(reply, zmq::send_flags::none);
  }

  INFO("[%s] Message Server ended cleanly", name.c_str());
}

void CupDAQManager::TF_DataServer()
{
  int data_port = fDAQPort + PORT_OFFSET::DATA;
  std::string name = fDAQName;

  auto server_socket = std::make_unique<TServerSocket>(data_port, kTRUE);

  if (!server_socket->IsValid()) {
    ERROR("[%s] ROOT socket failed to bind on port %d", name.c_str(), data_port);
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  std::vector<std::unique_ptr<TSocket>> client_sockets;

  auto monitor = std::make_unique<TMonitor>();
  monitor->Add(server_socket.get());

  INFO("[%s] ROOT Data Server started on port %d", name.c_str(), data_port);

  while (true) {
    if (fDoExit) { break; }

    TSocket * active_socket = monitor->Select(1000);
    if (active_socket == (TSocket *)-1) { continue; }

    if (active_socket->IsA() == TServerSocket::Class()) {
      TSocket * raw_client = server_socket->Accept();
      if (raw_client) {
        monitor->Add(raw_client);
        client_sockets.push_back(std::unique_ptr<TSocket>(raw_client));
        INFO("[%s] New ROOT client connected", name.c_str());
      }
    }
    else {
      TMessage * raw_mess = nullptr;

      // If client disconnects or an error occurs
      if (active_socket->Recv(raw_mess) <= 0 || raw_mess == nullptr) {
        INFO("[%s] ROOT client disconnected", name.c_str());

        monitor->Remove(active_socket);

        client_sockets.erase(std::remove_if(client_sockets.begin(), client_sockets.end(),
                                            [&](const std::unique_ptr<TSocket> & p) {
                                              return p.get() == active_socket;
                                            }),
                             client_sockets.end());
        continue;
      }

      std::unique_ptr<TMessage> mess(raw_mess);

      // Handle BuiltEvent object
      if (mess->GetClass() && mess->GetClass()->InheritsFrom(BuiltEvent::Class())) {
        auto * event = static_cast<BuiltEvent *>(mess->ReadObject(mess->GetClass()));
        int daqid = event->GetDAQID();

        // Critical: Protect the event buffer from concurrent access
        std::lock_guard<std::mutex> lock(fRecvBufferMutex);
        for (auto & buf : fRecvEventBuffer) {
          if (buf.first == daqid) {
            buf.second->push_back(std::unique_ptr<BuiltEvent>(event));
            break;
          }
        }
      }
      else {
        WARNING("[%s] Received unknown TMessage format", name.c_str());
      }
    }
  }

  INFO("[%s] ROOT Data Server ended", name.c_str());
}

void CupDAQManager::TF_ShrinkToFit()
{
  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }

  INFO("shrink buffer memory to fit started");

  TStopwatch sw;
  sw.Start();

  const int nadc_int = GetEntries();

  while (true) {
    double elapsetime = sw.RealTime();
    sw.Continue();

    if (elapsetime >= 10.0) {
      sw.Start(true);

      for (int i = 0; i < nadc_int; ++i) {
        auto * adc = static_cast<AbsADC *>(fCont[i]);
        adc->Bshrink_to_fit();

        auto * modbuffer = fADCRawBuffers.at(static_cast<std::size_t>(i));
        modbuffer->shrink_to_fit();
      }

      fBuiltEventBuffer1.shrink_to_fit();
      fBuiltEventBuffer2.shrink_to_fit();

      if (!fRecvEventBuffer.empty()) {
        for (auto & buf : fRecvEventBuffer) {
          buf.second->shrink_to_fit();
        }
      }

      if (fVerboseLevel > 0) { INFO("shrink buffer memory to fit"); }
    }

    bool runstate = RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDED) ||
                    RUNSTATE::CheckError(fRunStatus) || fDoExit;
    if (runstate) { break; }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  INFO("shrink buffer memory to fit ended");
}

void CupDAQManager::TF_SplitOutput(bool ontcb)
{
  TStopwatch sw;

  if (ontcb) {
    if (!ThreadWait(fRunStatusTCB, fDoExitTCB)) { return; }
    INFO("output splitter started (TCB)");

    sw.Start();
    while (true) {
      double elapsetime = sw.RealTime();
      sw.Continue();
      if (elapsetime >= fOutputSplitTime) {
        sw.Start(true);
        SendCommandToDAQs("kSPLITOUTPUTFILE");
        fSubRunNumber += 1;

        if (fVerboseLevel >= 1) { INFO("output file will be split"); }
      }
      if (fDoEndRunTCB || RUNSTATE::CheckError(fRunStatusTCB)) { break; }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
  else {
    if (!ThreadWait(fRunStatus, fDoExit)) { return; }
    INFO("output splitter started");

    sw.Start();
    while (true) {
      double elapsetime = sw.RealTime();
      sw.Continue();
      if (elapsetime >= fOutputSplitTime) {
        sw.Start(true);
        fDoSplitOutputFile = true;
        fSubRunNumber += 1;

        if (fVerboseLevel >= 1) { INFO("output file will be split"); }
      }
      bool runstate = RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNENDED) ||
                      RUNSTATE::CheckError(fRunStatus) || fDoExit;
      if (runstate) { break; }

      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  INFO("output splitter ended");
}
