#include <chrono>
#include <set>
#include <string>
#include <zmq.hpp>

#include "TMessage.h"
#include "TObjArray.h"

#include "DAQ/CupDAQManager.hh"
#include "DAQUtils/ELog.hh"

void CupDAQManager::TF_SendData()
{
  double perror = 0;
  double integral = 0;

  fSendStatus.store(READY);

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }

  INFO("started");

  zmq::context_t context(1);
  zmq::socket_t socket(context, zmq::socket_type::dealer);

  std::string identity = std::to_string(fDAQID);
  socket.set(zmq::sockopt::routing_id, identity);
  socket.set(zmq::sockopt::rcvtimeo, 200);

  std::string endpoint = "tcp://" + fMergeServerHost + ":" + std::to_string(fMergeServerPort);
  socket.connect(endpoint);
  INFO("connecting to %s (DAQID=%d)", endpoint.c_str(), fDAQID);

  fSendStatus.store(RUNNING);

  constexpr int TARGET_BYTES = 1 * 1024 * 1024;
  constexpr auto TIME_THRESHOLD = std::chrono::milliseconds(100);

  int batchSize = 0;
  int eventSize = 0;

  TObjArray batch;
  batch.SetOwner(kTRUE);
  int batchCount = 0;
  auto lastFlushTime = std::chrono::steady_clock::now();

  auto flushBatch = [&]() -> bool {
    if (batchCount == 0) { return true; }

    TMessage msg(kMESS_OBJECT);
    msg.WriteObject(&batch);

    zmq::message_t empty(0);
    zmq::message_t zmqmsg(msg.Buffer(), static_cast<size_t>(msg.Length()));

    if (fVerboseLevel > 1) {
      DEBUG("flushing batch (count=%d, size=%d bytes)", batchCount, static_cast<int>(msg.Length()));
    }

    socket.send(empty, zmq::send_flags::sndmore);
    auto result = socket.send(zmqmsg, zmq::send_flags::none);

    if (!result) {
      ERROR("batch send failed (batchCount=%d)", batchCount);
      return false;
    }

    if (fVerboseLevel > 1) {
      DEBUG("batch sent (count=%d, size=%d bytes)", batchCount, static_cast<int>(msg.Length()));
    }

    batch.Delete();
    batchCount = 0;
    lastFlushTime = std::chrono::steady_clock::now();
    return true;
  };

  while (true) {
    if (fDoExit.load() || RUNSTATE::CheckError(fRunStatus)) {
      INFO("exit condition met (fDoExit=%d, error=%d)", (int)fDoExit.load(),
           (int)RUNSTATE::CheckError(fRunStatus));
      break;
    }

    if (fBuildStatus.load() == ENDED && fBuiltEventBuffer1.empty()) {
      INFO("build ended and buffer empty, flushing remaining batch (count=%d)", batchCount);
      if (!flushBatch()) {
        RUNSTATE::SetError(fRunStatus);
        fSendStatus.store(ERROR);
      }
      break;
    }

    auto opt = fBuiltEventBuffer1.pop_front(std::chrono::milliseconds(50));
    if (opt) {
      const auto & event = *opt;

      if (eventSize == 0) {
        eventSize = event->GetSize();
        batchSize = std::max(1, TARGET_BYTES / eventSize);
        INFO("event size=%d bytes, batch size set to %d events", eventSize, batchSize);
      }

      if (fVerboseLevel > 1) {
        DEBUG("adding event to batch (trigNum=%u, batchCount=%d/%d)", event->GetTriggerNumber(),
              batchCount + 1, batchSize);
      }

      batch.Add(new BuiltEvent(*event));
      batchCount += 1;
    }

    bool batchFull = (batchSize > 0 && batchCount >= batchSize);
    bool timeLimitReached = (std::chrono::steady_clock::now() - lastFlushTime) > TIME_THRESHOLD;

    if ((batchFull || timeLimitReached) && batchCount > 0) {
      if (fVerboseLevel > 1) {
        DEBUG("flush triggered (batchFull=%d, timeLimitReached=%d)", (int)batchFull,
              (int)timeLimitReached);
      }
      if (!flushBatch()) {
        RUNSTATE::SetError(fRunStatus);
        fSendStatus.store(ERROR);
        break;
      }
    }
  }

  batch.Delete();

  INFO("sending disconnect signal to DataServer");
  zmq::message_t empty(0);
  zmq::message_t disconnect(0);
  socket.send(empty, zmq::send_flags::sndmore);
  socket.send(disconnect, zmq::send_flags::none);

  if (fSendStatus.load() != ERROR) { fSendStatus.store(ENDED); }
  INFO("ended");
}

void CupDAQManager::TF_DataServer()
{
  int data_port = fDAQPort + PORT_OFFSET::DATA;
  std::string name = fDAQName;

  fRecvStatus.store(READY);

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }

  INFO("started");

  zmq::context_t context(1);
  zmq::socket_t socket(context, zmq::socket_type::router);

  socket.set(zmq::sockopt::router_handover, 1);
  socket.set(zmq::sockopt::rcvtimeo, 200);

  std::string endpoint = "tcp://*:" + std::to_string(data_port);
  socket.bind(endpoint);
  INFO("listening on %s", endpoint.c_str());

  std::set<std::string> connectedClients;
  int nExpected = static_cast<int>(fRecvEventBuffers.size());

  fRecvStatus.store(RUNNING);

  bool everConnected = false;
  bool isShuttingDown = false;
  std::chrono::steady_clock::time_point shutdownStartTime;
  const auto SHUTDOWN_TIMEOUT = std::chrono::seconds(10);

  while (true) {
    if (RUNSTATE::CheckError(fRunStatus)) {
      ERROR("error state detected, exiting");
      break;
    }

    if (fDoExit.load() && !isShuttingDown) {
      isShuttingDown = true;
      shutdownStartTime = std::chrono::steady_clock::now();
      INFO("shutdown initiated, waiting for clients to disconnect (connected: %zu/%d)",
           connectedClients.size(), nExpected);
    }

    if (isShuttingDown) {
      if (std::chrono::steady_clock::now() - shutdownStartTime > SHUTDOWN_TIMEOUT) {
        WARNING("shutdown timeout reached, forcing exit (remaining clients: %zu)",
                connectedClients.size());
        break;
      }
    }

    if (everConnected && connectedClients.empty()) {
      INFO("all clients disconnected, exiting gracefully");
      break;
    }

    if (!everConnected && isShuttingDown) {
      INFO("no clients were connected, exiting");
      break;
    }

    zmq::message_t identity;
    zmq::message_t empty;
    zmq::message_t zmqmsg;

    auto result = socket.recv(identity);
    if (!result) {
      if (fVerboseLevel > 1) { DEBUG("recv timeout, no message received"); }
      continue;
    }

    if (identity.more()) { (void)socket.recv(empty); }
    else {
      WARNING("invalid ROUTER frame (no delimiter)");
      continue;
    }

    if (empty.more()) { (void)socket.recv(zmqmsg); }
    else {
      WARNING("invalid ROUTER frame (no payload)");
      continue;
    }

    bool has_more = zmqmsg.more();
    while (has_more) {
      zmq::message_t dummy;
      (void)socket.recv(dummy);
      has_more = dummy.more();
      WARNING("discarded extra multipart frame from client");
    }

    std::string clientId(identity.data<char>(), identity.size());

    auto [it, inserted] = connectedClients.insert(clientId);
    if (inserted) {
      everConnected = true;
      INFO("client connected DAQID=%s (%zu/%d)", clientId.c_str(), connectedClients.size(),
           nExpected);
    }

    if (zmqmsg.size() == 0) {
      connectedClients.erase(it);
      INFO("client disconnected DAQID=%s (remaining: %zu/%d)", clientId.c_str(),
           connectedClients.size(), nExpected);
      continue;
    }

    if (fVerboseLevel > 1) {
      DEBUG("received message from DAQID=%s (size=%zu bytes)", clientId.c_str(), zmqmsg.size());
    }

    TMessage msg(kMESS_OBJECT);
    msg.SetBuffer(zmqmsg.data<char>(), static_cast<UInt_t>(zmqmsg.size()), kFALSE);
    msg.SetReadMode();
    msg.Reset();

    auto * arr = static_cast<TObjArray *>(msg.ReadObject(TObjArray::Class()));
    if (!arr) {
      ERROR("deserialization failed");
      continue;
    }

    int nEvents = arr->GetEntriesFast();
    if (fVerboseLevel > 1) {
      DEBUG("deserialized batch from DAQID=%s (nEvents=%d)", clientId.c_str(), nEvents);
    }

    for (int i = 0; i < nEvents; i++) {
      auto * ev = static_cast<BuiltEvent *>(arr->At(i));
      if (!ev) {
        ERROR("null event at index %d in batch", i);
        continue;
      }

      int daqId = ev->GetDAQID();
      if (fVerboseLevel > 1) {
        DEBUG("processing event [%d/%d] (daqId=%d, trigNum=%u)", i + 1, nEvents, daqId,
              ev->GetTriggerNumber());
      }

      auto bufIt = fRecvEventBuffers.find(daqId);
      if (bufIt == fRecvEventBuffers.end()) {
        ERROR("unknown DAQID=%d, dropping event", daqId);
        continue;
      }

      bufIt->second->push_back(std::make_shared<BuiltEvent>(*ev));
    }

    arr->SetOwner(kTRUE);
    arr->Delete();
    delete arr;
  }

  fRecvStatus.store(ENDED);
  INFO("ended");
}