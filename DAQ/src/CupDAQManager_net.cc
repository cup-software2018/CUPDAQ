#include <chrono>
#include <set>
#include <string>
#include <thread>
#include <zmq.hpp>

#include "TMessage.h"

#include "DAQ/CupDAQManager.hh"
#include "DAQConfig/DAQConf.hh"
#include "DAQUtils/ELog.hh"

void CupDAQManager::TF_SendData()
{
  double perror = 0;
  double integral = 0;

  fSendStatus = READY;
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

  fSendStatus = RUNNING;

  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }
    if (fBuildStatus == ENDED && fBuiltEventBuffer1.empty()) { break; }

    int totalsize = static_cast<int>(fBuiltEventBuffer1.size());

    auto opt = fBuiltEventBuffer1.pop_front(std::chrono::milliseconds(200));
    if (opt) {
      const auto & event = *opt;

      TMessage msg(kMESS_OBJECT);
      msg.WriteObject(event.get());

      zmq::message_t empty(0);
      zmq::message_t zmqmsg(msg.Buffer(), static_cast<size_t>(msg.Length()));

      socket.send(empty, zmq::send_flags::sndmore);
      auto result = socket.send(zmqmsg, zmq::send_flags::none);

      if (!result) {
        ERROR("send failed (trigNum=%u)", event->GetTriggerNumber());
        RUNSTATE::SetError(fRunStatus);
        fSendStatus = ERROR;
        break;
      }

      totalsize -= 1;
    }

    ThreadSleep(fSendSleep, perror, integral, totalsize);
  }

  fSendStatus = ENDED;
}

void CupDAQManager::TF_DataServer()
{
  int data_port = fDAQPort + PORT_OFFSET::DATA;
  std::string name = fDAQName;

  fRecvStatus = READY;
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

  fRecvStatus = RUNNING;

  while (!fDoExit && !RUNSTATE::CheckError(fRunStatus)) {
    zmq::message_t identity;
    zmq::message_t empty;
    zmq::message_t zmqmsg;

    auto result = socket.recv(identity);
    if (!result) { continue; }

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

    if (connectedClients.find(clientId) == connectedClients.end()) {
      connectedClients.insert(clientId);
      INFO("client connected DAQID=%s (%zu/%d)", clientId.c_str(), connectedClients.size(),
           nExpected);
    }

    if (zmqmsg.size() == 0) {
      connectedClients.erase(clientId);
      INFO("client disconnected DAQID=%s", clientId.c_str());
      continue;
    }

    TMessage msg(kMESS_OBJECT);
    msg.SetBuffer(zmqmsg.data<char>(), static_cast<UInt_t>(zmqmsg.size()), kFALSE);
    msg.SetReadMode();
    msg.Reset();

    auto * ev = static_cast<BuiltEvent *>(msg.ReadObject(BuiltEvent::Class()));
    if (!ev) {
      ERROR("deserialization failed");
      continue;
    }

    int daqId = ev->GetDAQID();

    auto it = fRecvEventBuffers.find(daqId);
    if (it == fRecvEventBuffers.end()) {
      ERROR("unknown DAQID=%d, dropping event", daqId);
      delete ev;
      continue;
    }

    it->second->push_back(std::shared_ptr<BuiltEvent>(ev));
  }

  fRecvStatus = ENDED;
}