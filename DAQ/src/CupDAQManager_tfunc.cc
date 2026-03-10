#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

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

  const int max_clients = 10;

  int client_socket[max_clients];
  auto ** root_socket = new TSocket *[max_clients];
  for (int i = 0; i < max_clients; ++i) {
    client_socket[i] = -1;
    root_socket[i] = nullptr;
  }

  int master_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (master_socket == 0) {
    ERROR("[%s] socket failed", name.c_str());
    istcb ? RUNSTATE::SetError(fRunStatusTCB) : RUNSTATE::SetError(fRunStatus);
    delete[] root_socket;
    return;
  }

  int opt = 1;
  if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&opt),
                 sizeof(opt)) < 0) {
    ERROR("[%s] setsockopt failed", name.c_str());
    istcb ? RUNSTATE::SetError(fRunStatusTCB) : RUNSTATE::SetError(fRunStatus);
    close(master_socket);
    delete[] root_socket;
    return;
  }

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(static_cast<uint16_t>(port));

  socklen_t addrlen = sizeof(address);

  if (bind(master_socket, reinterpret_cast<struct sockaddr *>(&address), addrlen) < 0) {
    ERROR("[%s] socket bind failed on port %d", name.c_str(), port);
    istcb ? RUNSTATE::SetError(fRunStatusTCB) : RUNSTATE::SetError(fRunStatus);
    close(master_socket);
    delete[] root_socket;
    return;
  }
  INFO("[%s] message server on port %d", name.c_str(), port);

  if (listen(master_socket, 3) < 0) {
    ERROR("[%s] socket listen error", name.c_str());
    istcb ? RUNSTATE::SetError(fRunStatusTCB) : RUNSTATE::SetError(fRunStatus);
    close(master_socket);
    delete[] root_socket;
    return;
  }

  INFO("[%s] message server start", name.c_str());

  char buffer[kMESSLEN];
  timeval tv{};
  fd_set readfds;

  std::unique_lock<std::mutex> mlock(fMonitorMutex, std::defer_lock);

  while (true) {
    if (fDoExit || fDoExitTCB) { break; }

    FD_ZERO(&readfds);
    FD_SET(master_socket, &readfds);

    int max_sd = master_socket;

    for (int sd : client_socket) {
      if (sd > 0) { FD_SET(sd, &readfds); }
      if (sd > max_sd) { max_sd = sd; }
    }

    tv.tv_sec = 1;
    tv.tv_usec = 0;

    int activity = select(max_sd + 1, &readfds, nullptr, nullptr, &tv);
    if (activity == 0) { continue; }
    else if (activity < 0 && errno != EINTR) {
      // DEBUG("select error occurred for [%s]", name.c_str());
      continue;
    }

    if (FD_ISSET(master_socket, &readfds)) {
      int new_socket =
          accept(master_socket, reinterpret_cast<struct sockaddr *>(&address), &addrlen);

      if (new_socket < 0) {
        ERROR("[%s]: accept error occurred", name.c_str());
        istcb ? RUNSTATE::SetError(fRunStatusTCB) : RUNSTATE::SetError(fRunStatus);
        close(master_socket);
        for (int i = 0; i < max_clients; ++i) {
          if (root_socket[i]) { delete root_socket[i]; }
        }
        delete[] root_socket;
        return;
      }

      INFO("[%s]: new client connection, socket fd: %d, ip: %s, port: %d", name.c_str(), new_socket,
           inet_ntoa(address.sin_addr), ntohs(address.sin_port));

      for (int i = 0; i < max_clients; ++i) {
        if (client_socket[i] == -1) {
          client_socket[i] = new_socket;
          root_socket[i] = new TSocket(new_socket);
          INFO("[%s]: adding to list of sockets at %d", name.c_str(), i);
          break;
        }
      }
    }

    for (int i = 0; i < max_clients; ++i) {
      int sd = client_socket[i];

      if (sd > 0 && FD_ISSET(sd, &readfds)) {
        std::memset(buffer, 0, kMESSLEN);
        int valread = read(sd, buffer, kMESSLEN);

        getpeername(sd, reinterpret_cast<struct sockaddr *>(&address), &addrlen);

        if (valread == 0) {
          INFO("[%s]: host disconnected, socket fd: %d, ip: %s, port: %d", name.c_str(), sd,
               inet_ntoa(address.sin_addr), ntohs(address.sin_port));

          close(sd);
          client_socket[i] = -1;
          delete root_socket[i];
          root_socket[i] = nullptr;
        }
        else {
          unsigned long command = 0;
          unsigned long dummy = 0;
          DecodeMsg(buffer, command, dummy, dummy, dummy);

          switch (command) {
            case kQUERYDAQSTATUS: {
              EncodeMsg(buffer, istcb ? fRunStatusTCB : fRunStatus);
              root_socket[i]->SendRaw(buffer, kMESSLEN);
              break;
            }
            case kQUERYRUNINFO: {
              EncodeMsg(buffer, static_cast<unsigned long>(fRunNumber),
                        static_cast<unsigned long>(fSubRunNumber),
                        static_cast<unsigned long>(fStartDatime),
                        static_cast<unsigned long>(fEndDatime));
              root_socket[i]->SendRaw(buffer, kMESSLEN);
              break;
            }
            case kQUERYTRGINFO: {
              unsigned long nevent = 0;
              unsigned long daqtime = 0;

              mlock.lock();
              nevent = static_cast<unsigned long>(fTriggerNumber);
              daqtime = fTriggerTime;
              mlock.unlock();

              EncodeMsg(buffer, nevent, daqtime);
              root_socket[i]->SendRaw(buffer, kMESSLEN);
              break;
            }
            case kREQUESTCONFIG: {
              root_socket[i]->SendObject(fConfigList);
              INFO("[%s] sent config list to DAQ", name.c_str());
              break;
            }
            case kSPLITOUTPUTFILE: {
              if (istcb) { fDoSplitOutputFileTCB = true; }
              else {
                fDoSplitOutputFile = true;
              }
              INFO("[%s] output file split command received", name.c_str());
              break;
            }
            case kRECVEVENT: {
              TMessage * mess = nullptr;
              if (root_socket[i]->Recv(mess) > 0 && mess != nullptr) {
                auto * event = static_cast<BuiltEvent *>(mess->ReadObject(mess->GetClass()));
                int daqid = event->GetDAQID();
                for (auto & buf : fRecvEventBuffer) {
                  if (buf.first == daqid) {
                    buf.second->push_back(std::unique_ptr<BuiltEvent>(event));
                    break;
                  }
                }
                delete mess;
              }
              else {
                WARNING("[%s] error in event sender [ip=%s, port=%d]", name.c_str(),
                        inet_ntoa(address.sin_addr), ntohs(address.sin_port));
              }
              break;
            }
            case kSETERROR: {
              RUNSTATE::SetError(fRunStatus);
              break;
            }
            case kQUERYMONITOR: {
              EncodeMsg(buffer, static_cast<unsigned long>(fMonitorServerOn));
              root_socket[i]->SendRaw(buffer, kMESSLEN);
              break;
            }
            case kCONFIGRUN: {
              if (istcb) { fDoConfigRunTCB = true; }
              else {
                fDoConfigRun = true;
              }
              INFO("[%s] CONFIGRUN command received", name.c_str());
              break;
            }
            case kSTARTRUN: {
              if (istcb) { fDoStartRunTCB = true; }
              else {
                fDoStartRun = true;
              }
              INFO("[%s] STARTRUN command received", name.c_str());
              break;
            }
            case kENDRUN: {
              if (istcb) { fDoEndRunTCB = true; }
              else {
                fDoEndRun = true;
              }
              INFO("[%s] ENDRUN command received", name.c_str());
              break;
            }
            case kEXIT: {
              if (istcb) { fDoExitTCB = true; }
              else {
                fDoExit = true;
              }
              INFO("[%s] EXIT command received", name.c_str());
              break;
            }
            default: {
              WARNING("[%s] unknown command [%lu] received", name.c_str(), command);
              break;
            }
          }
        }
      }
    }
  }

  close(master_socket);

  for (int i = 0; i < max_clients; ++i) {
    if (root_socket[i]) { delete root_socket[i]; }
  }
  delete[] root_socket;

  INFO("[%s] message server ended", name.c_str());
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
        SendCommandToDAQ(kSPLITOUTPUTFILE);
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
