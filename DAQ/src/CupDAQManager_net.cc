#include <algorithm> // Required for std::clamp
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

  auto gettext = [](const std::string & name) -> std::string {
    size_t p = name.find_last_not_of("0123456789");
    return (p == std::string::npos) ? "" : name.substr(0, p + 1);
  };

  std::string daqname = gettext(fDAQName);
  auto * daq = static_cast<DAQConf *>(fConfigList->GetDAQConfig());

  int ndaq = 0;
  for (int i = 0; i < daq->GetN(); ++i) {
    if (daq->GetDAQName(i).find(daqname) != std::string::npos) { ndaq++; }
  }

  if (fVerboseLevel > 1) {
    DEBUG("Parsed DAQ base name '%s', found %d matching nodes.", daqname.c_str(), ndaq);
  }

  if (!ThreadWait(fRunStatus, fDoExit)) {
    WARNING("exited by exit command");
    return;
  }
  INFO("Data Client started, connecting to Data Server...");

  auto socket = std::make_unique<TSocket>(fMergeServerIPAddr.c_str(), fMergeServerPort);

  if (socket->GetErrorCode() < 0) {
    RUNSTATE::SetError(fRunStatus);
    ERROR("connection to Data Server failed at %s:%d", fMergeServerIPAddr.c_str(),
          fMergeServerPort);
    return;
  }

  // Expand TCP send buffer to absorb micro-bursts (8MB)
  socket->SetOption(kSendBuffer, 8 * 1024 * 1024);

  // Set OS-level send/recv timeout to prevent SendObject() from blocking forever.
  // If the server cannot drain its receive buffer in time, SendObject() would
  // block indefinitely due to TCP flow control. 10s timeout breaks that deadlock.
  {
    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    int fd = socket->GetDescriptor();
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
      WARNING("Failed to set SO_SNDTIMEO on send socket");
    }
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
      WARNING("Failed to set SO_RCVTIMEO on send socket");
    }
    if (fVerboseLevel > 1) { DEBUG("Socket send/recv timeout set to %lds", tv.tv_sec); }
  }

  INFO("connection to Data Server succeeded");
  fSendStatus = RUNNING;

  // N-Node Dynamic Bandwidth Optimization (Auto-Scaling)

  int safe_ndaq = (ndaq > 0) ? ndaq : 1;
  const size_t SAFE_TOTAL_BW = 80 * 1024 * 1024;
  size_t node_bw_share = SAFE_TOTAL_BW / safe_ndaq;

  size_t low_watermark_bytes = static_cast<size_t>(node_bw_share * 0.05);
  low_watermark_bytes = std::clamp<size_t>(low_watermark_bytes, 100 * 1024, 2 * 1024 * 1024);

  size_t burst_watermark_bytes = static_cast<size_t>(node_bw_share * 0.25);
  burst_watermark_bytes =
      std::clamp<size_t>(burst_watermark_bytes, 1 * 1024 * 1024, 5 * 1024 * 1024);

  const size_t panic_watermark_bytes = 50 * 1024 * 1024;

  // Dynamic Jittering

  int jitter_step = 50 / safe_ndaq;
  int jitter_ms = (fDAQID % safe_ndaq) * jitter_step;

  int base_flush_timeout_ms = 50;
  int my_flush_timeout_ms = base_flush_timeout_ms + jitter_ms;

  INFO("Dynamic Settings - Low WM: %zu KB, Burst: %zu KB, Timeout: %d ms",
       low_watermark_bytes / 1024, burst_watermark_bytes / 1024, my_flush_timeout_ms);

  // Preparation for Event Batching

  double moving_avg_event_size = 100 * 1024.0;

  TObjArray batchArray;
  std::vector<std::shared_ptr<BuiltEvent>> keep_alive_buffer;
  keep_alive_buffer.reserve(1000);

  auto last_flush_time = std::chrono::steady_clock::now();

  // Main Transmission Loop

  size_t current_batch_bytes = 0;

  while (true) {
    if (fDoExit || RUNSTATE::CheckError(fRunStatus)) { break; }

    size_t estimated_buffer_bytes =
        fBuiltEventBuffer1.size() * static_cast<size_t>(moving_avg_event_size);

    size_t target_batch_bytes = low_watermark_bytes;
    int current_timeout_ms = my_flush_timeout_ms;

    if (estimated_buffer_bytes > panic_watermark_bytes) {
      WARNING("PANIC! Buffer size exceeded 50MB. Risk of Out-Of-Memory.");
    }

    if (estimated_buffer_bytes > low_watermark_bytes) {
      target_batch_bytes = burst_watermark_bytes;
      current_timeout_ms = 0;
    }

    while (!fBuiltEventBuffer1.empty() && current_batch_bytes < target_batch_bytes) {
      auto bevent_opt = fBuiltEventBuffer1.pop_front();
      if (bevent_opt.has_value()) {
        std::shared_ptr<BuiltEvent> bevent_ptr = bevent_opt.value();
        keep_alive_buffer.push_back(bevent_ptr);
        batchArray.Add(bevent_ptr.get());

        size_t ev_size = bevent_ptr->GetSize();
        current_batch_bytes += ev_size;

        moving_avg_event_size = (moving_avg_event_size * 0.9) + (ev_size * 0.1);
      }
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(now - last_flush_time).count();

    bool should_send = false;
    bool is_timeout = false;

    if (!keep_alive_buffer.empty()) {
      if (current_batch_bytes >= target_batch_bytes) { should_send = true; }
      else if (elapsed_ms >= current_timeout_ms) {
        should_send = true;
        is_timeout = true;
      }
      else if (fBuildStatus == ENDED && fBuiltEventBuffer1.empty()) {
        should_send = true; // Flush any remaining data before exiting
      }
    }

    if (should_send) {
      if (fVerboseLevel > 1) {
        DEBUG("Flushing: %zu bytes (estimated), %d events. Trigger: %s (Elapsed: %lld ms)",
              current_batch_bytes, batchArray.GetEntriesFast(),
              is_timeout ? "TIMEOUT" : "SIZE LIMIT", elapsed_ms);
      }

      auto send_start = std::chrono::steady_clock::now();
      int state = socket->SendObject(&batchArray);
      auto send_end = std::chrono::steady_clock::now();
      long long send_dur = std::chrono::duration_cast<std::chrono::milliseconds>(send_end - send_start).count();

      INFO("[DEBUG-TF_SendData] SendObject(%d events, est %zu B) took %lld ms. State: %d", 
           batchArray.GetEntriesFast(), current_batch_bytes, send_dur, state);


      if (state < 0) {
        // state == -4 (EWOULDBLOCK/EAGAIN) means the OS send timeout fired —
        // i.e. the server's receive buffer was full and we waited 10 s.
        // Any negative value is treated as a fatal error.
        RUNSTATE::SetError(fRunStatus);
        ERROR("SendObject failed (state=%d). Possible causes: server buffer full "
              "(TCP flow-control timeout), network error, or server crash.",
              state);
        break;
      }

      if (fVerboseLevel > 1) {
        DEBUG("Send complete. Socket returned %d bytes (actual serialized size, "
              "estimated was %zu bytes)",
              state, current_batch_bytes);
      }

      last_flush_time = std::chrono::steady_clock::now();

      batchArray.Clear();
      keep_alive_buffer.clear();
      current_batch_bytes = 0;
    }
    else {
      if (fBuildStatus == ENDED && fBuiltEventBuffer1.empty()) { break; }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

  fSendStatus = ENDED;
  INFO("Data Client disconnected and ended");
}

void CupDAQManager::TF_DataServer()
{
  int data_port = fDAQPort + PORT_OFFSET::DATA;
  std::string name = fDAQName;

  auto server_socket = std::make_unique<TServerSocket>(data_port, kTRUE);

  if (!server_socket->IsValid()) {
    ERROR("ROOT socket failed to bind on port %d", data_port);
    RUNSTATE::SetError(fRunStatus);
    return;
  }

  server_socket->SetOption(kRecvBuffer, 32 * 1024 * 1024);

  std::vector<std::unique_ptr<TSocket>> client_sockets;

  auto monitor = std::make_unique<TMonitor>();
  monitor->Add(server_socket.get());

  INFO("ROOT Data Server started on port %d", data_port);

  while (true) {
    if (fDoExit) { break; }

    TSocket * active_socket = monitor->Select(1000);
    if (active_socket == (TSocket *)-1) {
      // Select() timed out. Exit if the merger has finished and all
      // DAQ clients have already disconnected — no more data will arrive.
      if (fMergeStatus == ENDED && client_sockets.empty()) { break; }
      continue;
    }

    if (active_socket->IsA() == TServerSocket::Class()) {
      TSocket * raw_client = server_socket->Accept();
      if (raw_client) {
        monitor->Add(raw_client);
        client_sockets.push_back(std::unique_ptr<TSocket>(raw_client));
        INFO("New ROOT client connected");
        if (fVerboseLevel > 1) {
          DEBUG("Total active client connections: %zu", client_sockets.size());
        }
      }
    }
    else {
      TMessage * raw_mess = nullptr;

      auto recv_start = std::chrono::steady_clock::now();
      if (active_socket->Recv(raw_mess) <= 0 || raw_mess == nullptr) {
        INFO("ROOT client disconnected");

        monitor->Remove(active_socket);
        client_sockets.erase(std::remove_if(client_sockets.begin(), client_sockets.end(),
                                            [&](const std::unique_ptr<TSocket> & p) {
                                              return p.get() == active_socket;
                                            }),
                             client_sockets.end());

        if (fVerboseLevel > 1) {
          DEBUG("Remaining client connections: %zu", client_sockets.size());
        }
        continue;
      }
      auto recv_end = std::chrono::steady_clock::now();
      long long recv_dur = std::chrono::duration_cast<std::chrono::milliseconds>(recv_end - recv_start).count();

      std::unique_ptr<TMessage> mess(raw_mess);

      if (fVerboseLevel > 1) {
        DEBUG("Received message of class: %s",
              mess->GetClass() ? mess->GetClass()->GetName() : "UNKNOWN");
      }

      if (mess->GetClass() && mess->GetClass()->InheritsFrom(TObjArray::Class())) {
        auto read_start = std::chrono::steady_clock::now();
        auto * array = static_cast<TObjArray *>(mess->ReadObject(mess->GetClass()));
        auto read_end = std::chrono::steady_clock::now();
        long long read_dur = std::chrono::duration_cast<std::chrono::milliseconds>(read_end - read_start).count();

        if (array) {
          int entries = array->GetEntriesFast();

          INFO("[DEBUG-TF_DataServer] Recv() took %lld ms, ReadObject() took %lld ms. Extracted TObjArray with %d entries.", 
               recv_dur, read_dur, entries);


          int last_daqid = -1;
          ConcurrentDeque<std::shared_ptr<BuiltEvent>> * target_queue = nullptr;

          for (int i = 0; i < entries; ++i) {
            auto * raw_event = static_cast<BuiltEvent *>(array->At(i));
            if (!raw_event) continue;

            std::shared_ptr<BuiltEvent> event(raw_event);
            int current_daqid = event->GetDAQID();

            if (current_daqid != last_daqid) {
              target_queue = nullptr;

              {
                std::lock_guard<std::mutex> lock(fRecvBufferMutex);
                for (auto & buf : fRecvEventBuffer) {
                  if (buf.first == current_daqid) {
                    target_queue = buf.second.get();
                    last_daqid = current_daqid;
                    if (fVerboseLevel > 1) {
                      DEBUG("Switched target queue to DAQID: %d", current_daqid);
                    }
                    break;
                  }
                }
              }
            }

            if (target_queue) { target_queue->push_back(event); }
            else {
              WARNING("Event dropped: Unknown DAQID %d", current_daqid);
            }
          }

          array->SetOwner(kFALSE);
          delete array;
        }
      }
      else if (mess->GetClass() && mess->GetClass()->InheritsFrom(BuiltEvent::Class())) {
        auto * raw_event = static_cast<BuiltEvent *>(mess->ReadObject(mess->GetClass()));

        std::shared_ptr<BuiltEvent> event(raw_event);
        int daqid = event->GetDAQID();
        ConcurrentDeque<std::shared_ptr<BuiltEvent>> * target_queue = nullptr;

        if (fVerboseLevel > 1) { DEBUG("Processing single BuiltEvent for DAQID: %d", daqid); }

        {
          std::lock_guard<std::mutex> lock(fRecvBufferMutex);
          for (auto & buf : fRecvEventBuffer) {
            if (buf.first == daqid) {
              target_queue = buf.second.get();
              break;
            }
          }
        }

        if (target_queue) { target_queue->push_back(event); }
        else {
          WARNING("Event dropped: Unknown DAQID %d", daqid);
        }
      }
      else {
        WARNING("Received unknown TMessage format");
      }
    }
  }

  INFO("ROOT Data Server ended");
}