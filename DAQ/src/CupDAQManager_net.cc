#include <chrono>
#include <sys/socket.h> // for SO_SNDTIMEO / SO_RCVTIMEO
#include <sys/time.h>
#include <thread>

#include "TMessage.h"
#include "TMonitor.h"
#include "TServerSocket.h"
#include "TSocket.h"

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

  std::unique_ptr<TSocket> socket;
  while (!fDoExit && !RUNSTATE::CheckError(fRunStatus)) {
    auto * s = new TSocket(fMergeServerHost.c_str(), fMergeServerPort);
    if (s->IsValid()) {
      socket.reset(s);
      INFO("connected to %s:%d", fMergeServerHost.c_str(), fMergeServerPort);
      break;
    }
    delete s;
    WARNING("connection failed, retrying in 2 s...");
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }
  if (!socket) {
    fSendStatus = ERROR;
    return;
  }

  fSendStatus = RUNNING;

  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }
    if (fBuildStatus == ENDED && fBuiltEventBuffer1.empty()) { break; }

    int totalsize = static_cast<int>(fBuiltEventBuffer1.size());

    StartBenchmark("TF_SendData");

    auto opt = fBuiltEventBuffer1.pop_front(std::chrono::milliseconds(200));
    if (opt) {
      const auto & event = *opt;

      TMessage msg(kMESS_OBJECT);
      msg.WriteObject(event.get());

      bool sent = false;
      while (!fDoExit && !RUNSTATE::CheckError(fRunStatus)) {
        if (socket && socket->IsValid()) {
          if (socket->Send(msg) > 0) {
            sent = true;
            break;
          }
          WARNING("send failed (trigNum=%u), reconnecting...", event->GetTriggerNumber());
          socket->Close();
          socket.reset();
        }
        auto * s = new TSocket(fMergeServerHost.c_str(), fMergeServerPort);
        if (s->IsValid()) {
          socket.reset(s);
          INFO("reconnected to %s:%d", fMergeServerHost.c_str(), fMergeServerPort);
        }
        else {
          delete s;
          std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
      }

      if (!sent) {
        RUNSTATE::SetError(fRunStatus);
        fSendStatus = ERROR;
        break;
      }

      totalsize -= 1;
    }

    StopBenchmark("TF_SendData");
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

  auto serverSocket = std::make_unique<TServerSocket>(data_port, true);
  if (!serverSocket->IsValid()) {
    ERROR("failed to bind port %d", data_port);
    fRecvStatus = ERROR;
    return;
  }
  INFO("listening on port %d", data_port);

  fRecvStatus = RUNNING;

  std::vector<std::thread> clientThreads;

  while (!fDoExit && !RUNSTATE::CheckError(fRunStatus)) {
    TSocket * raw = static_cast<TSocket *>(serverSocket->Accept());
    if (!raw || !raw->IsValid()) {
      if (!fDoExit) ERROR("accept error");
      break;
    }
    INFO("client connected from %s", raw->GetInetAddress().GetHostName());

    auto sock = std::shared_ptr<TSocket>(raw);

    clientThreads.emplace_back([this, sock]() {
      while (!fDoExit && !RUNSTATE::CheckError(fRunStatus)) {
        TMessage * rawMsg = nullptr;
        int bytes = sock->Recv(rawMsg);

        if (bytes <= 0 || rawMsg == nullptr) {
          INFO("client disconnected");
          break;
        }

        std::unique_ptr<TMessage> msg(rawMsg);
        auto * ev = static_cast<BuiltEvent *>(msg->ReadObject(BuiltEvent::Class()));

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
    });
  }

  for (auto & t : clientThreads)
    if (t.joinable()) t.join();

  fRecvStatus = ENDED;
}