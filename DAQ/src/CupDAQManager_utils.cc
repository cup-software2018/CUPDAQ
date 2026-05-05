#include <algorithm>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <thread>

#include "DAQ/CupDAQManager.hh"
#include "DAQUtils/ELog.hh"

void CupDAQManager::PrintDAQSummary()
{
  char buf[128];
  std::string report = "\n";

  // --- benchmark table ---
  report += "====================================================\n";
  snprintf(buf, sizeof(buf), "%12s %16s %16s\n", "process", "cpu time[s]", "real time[s]");
  report += buf;
  report += "----------------------------------------------------\n";

  const char * pname[4] = {"ReadData", "SortEvent", "BuildEvent", "WriteEvent"};
  double ct = 0.0, rt = 0.0;
  for (int i = 0; i < 4; ++i) {
    double cputime  = fBenchmark->GetCpuTime(pname[i]);
    double realtime = fBenchmark->GetRealTime(pname[i]);
    snprintf(buf, sizeof(buf), "%12s %16.1f %16.1f\n", pname[i], cputime, realtime);
    report += buf;
    ct += cputime;
    rt += realtime;
  }
  report += "----------------------------------------------------\n";
  snprintf(buf, sizeof(buf), "%12s %16.1f %16.1f\n", "total", ct, rt);
  report += buf;
  report += "====================================================\n";

  // --- DAQ summary ---
  unsigned long totalReadDataSize;
  double liveTime;

  const std::size_t nadc = GetEntries();
  if (nadc > 0) {
    auto * theADC = static_cast<AbsADC *>(fCont[0]);
    totalReadDataSize = static_cast<unsigned long>(nadc * theADC->GetTotalBCount() * kKILOBYTES);
    liveTime = theADC->GetCurrentTime() / kDONESECOND;
  }
  else {
    totalReadDataSize = fTotalRawDataSize;
    liveTime = std::difftime(fEndDatime, fStartDatime);
  }

  double recvDataSize   = totalReadDataSize / kDGIGABYTES;
  double outputDataSize = fTotalWrittenDataSize / kDGIGABYTES;
  double trate = liveTime > 0.0 ? fTriggerNumber / liveTime : 0.0;
  double drate = liveTime > 0.0 ? recvDataSize   * 1024.0 / liveTime : 0.0;
  double orate = liveTime > 0.0 ? outputDataSize * 1024.0 / liveTime : 0.0;

  report += "\n************************* DAQ Summary *************************\n";
  snprintf(buf, sizeof(buf), "%32s%d\n",      "Run number : ",             fRunNumber);
  report += buf;
  snprintf(buf, sizeof(buf), "%32s%s\n",      "Start Time : ",             TDatime(fStartDatime).AsSQLString());
  report += buf;
  snprintf(buf, sizeof(buf), "%32s%s\n",      "End Time : ",               TDatime(fEndDatime).AsSQLString());
  report += buf;
  report += "\n";
  snprintf(buf, sizeof(buf), "%32s%.1f [s]\n",  "Live time : ",            liveTime);
  report += buf;
  snprintf(buf, sizeof(buf), "%32s%d\n",       "Total number of trigger : ", fTriggerNumber);
  report += buf;
  snprintf(buf, sizeof(buf), "%32s%.2f [Hz]\n", "Trigger rate : ",         trate);
  report += buf;
  snprintf(buf, sizeof(buf), "%32s%d\n",       "Total number of event : ", fNBuiltEvent);
  report += buf;
  report += "\n";
  snprintf(buf, sizeof(buf), "%32s%.3f GBytes (%.3f MB/sec)\n", "Received data size : ",
           recvDataSize, drate);
  report += buf;
  snprintf(buf, sizeof(buf), "%32s%.3f GBytes (%.3f MB/sec)\n", "Written data size : ",
           outputDataSize, orate);
  report += buf;
  report += "***************************************************************\n";

  INFO("%s", report.c_str());

  if (fSoftTrigger) { fSoftTrigger->PrintReport(); }
}

//
// TCB Server utilities
//

nlohmann::json CupDAQManager::SendCommandToDAQ(const std::unique_ptr<zmq::socket_t> & socket_ptr,
                                               const std::string & cmd, std::string & daq_name)
{
  nlohmann::json err_json = {{"status", "error"}};

  if (!socket_ptr) {
    daq_name = "Unknown";
    return err_json;
  }

  nlohmann::json req_json;
  req_json["command"] = cmd;
  std::string req_str = req_json.dump();

  zmq::message_t request(req_str.size());
  std::memcpy(request.data(), req_str.c_str(), req_str.size());

  if (!socket_ptr->send(request, zmq::send_flags::none)) {
    daq_name = "Unknown";
    ERROR("SEND failed for command [%s]", cmd.c_str());
    return err_json;
  }

  zmq::message_t reply;
  if (!socket_ptr->recv(reply, zmq::recv_flags::none)) {
    daq_name = "Unknown";
    ERROR("RECV failed (timeout) for command [%s]", cmd.c_str());
    return err_json;
  }

  std::string rep_str(static_cast<char *>(reply.data()), reply.size());
  nlohmann::json rep_json = nlohmann::json::parse(rep_str, nullptr, false);

  if (rep_json.is_discarded()) {
    daq_name = "Unknown";
    ERROR("CupDAQManager::SendCommandToDAQ: invalid JSON reply for %s", cmd.c_str());
    return err_json;
  }

  daq_name = rep_json.value("name", "Unknown");

  if (rep_json.value("status", "error") != "ok") {
    ERROR("CupDAQManager::SendCommandToDAQ: command %s failed on %s", cmd.c_str(),
          daq_name.c_str());
  }

  return rep_json;
}

void CupDAQManager::SendCommandToDAQs(const std::string & cmd)
{
  INFO("Broadcasting command [%s] to all connected DAQs", cmd.c_str());

  for (auto & socket_ptr : fDAQSocket) {
    if (!socket_ptr) continue;

    std::string daq_name;
    SendCommandToDAQ(socket_ptr, cmd, daq_name);
    INFO("Sent command [%s] to DAQ [%s]", cmd.c_str(), daq_name.c_str());
  }

  INFO("Finished broadcasting command [%s]", cmd.c_str());
}

unsigned long CupDAQManager::QueryDAQStatus(const std::unique_ptr<zmq::socket_t> & socket_ptr,
                                            std::string & daq_name)
{
  nlohmann::json reply = SendCommandToDAQ(socket_ptr, "kQUERYDAQSTATUS", daq_name);

  if (reply.value("status", "error") == "ok") { return reply.value("run_status", 0ul); }

  return 0;
}

bool CupDAQManager::WaitDAQStatus(RUNSTATE::STATE state)
{
  while (true) {
    bool all_ready = true;
    for (auto & socket_ptr : fDAQSocket) {
      if (!socket_ptr) continue;

      std::string daq_name;
      unsigned long daqstate = QueryDAQStatus(socket_ptr, daq_name);

      if (daqstate == 0) {
        ERROR("%s connection down", daq_name.c_str());
        socket_ptr.reset();
        return false;
      }
      if (RUNSTATE::CheckError(daqstate)) {
        ERROR("%s got error", daq_name.c_str());
        return false;
      }
      if (!RUNSTATE::CheckState(daqstate, state)) all_ready = false;
    }
    if (all_ready) return true;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

bool CupDAQManager::CheckDAQStatus()
{
  bool retval = true;
  for (auto & socket_ptr : fDAQSocket) {
    if (!socket_ptr) continue;

    std::string daq_name;
    unsigned long daqstate = QueryDAQStatus(socket_ptr, daq_name);

    if (daqstate == 0) {
      ERROR("%s connection down", daq_name.c_str());
      socket_ptr.reset();
      retval = false;
    }
    else if (RUNSTATE::CheckError(daqstate)) {
      ERROR("error in %s", daq_name.c_str());
      retval = false;
    }
  }

  return retval;
}

bool CupDAQManager::CheckDAQStatus(RUNSTATE::STATE state)
{
  bool retval = true;
  for (auto & socket_ptr : fDAQSocket) {
    if (!socket_ptr) continue;

    std::string daq_name;
    unsigned long daqstate = QueryDAQStatus(socket_ptr, daq_name);

    if (daqstate == 0) {
      ERROR("%s connection down", daq_name.c_str());
      socket_ptr.reset();
      retval = false;
    }
    else if (!RUNSTATE::CheckState(daqstate, state)) {
      ERROR("%s is not in expected state", daq_name.c_str());
      if (RUNSTATE::CheckError(daqstate)) { ERROR("error in %s", daq_name.c_str()); }
      retval = false;
    }
  }

  return retval;
}

//
// TCB/DAQ client Utilities
//

bool CupDAQManager::WaitRunState(std::atomic<unsigned long> & state, RUNSTATE::STATE pstate,
                                 std::atomic<bool> & exit)
{
  while (true) {
    if (RUNSTATE::CheckState(state, pstate)) break;
    if (RUNSTATE::CheckError(state)) return false;
    if (exit.load()) return false;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return true;
}

int CupDAQManager::WaitCommand(std::atomic<bool> & command, std::atomic<bool> & exit,
                               std::atomic<unsigned long> & state)
{
  while (true) {
    if (exit.load()) { return 1; }
    if (!CheckDAQStatus()) { return -1; }           // for tcb, daq will return true
    if (RUNSTATE::CheckError(state)) { return -1; } // for daq
    if (command.load()) { break; }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return 0;
}

void CupDAQManager::ThreadSleep(int & sleep, double & perror, double & integral, int size,
                                int tsize, double ki)
{
  if (!RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNNING)) { return; }

  const double Kp = sleep / 20.0;
  const double Kd = sleep / 10.0;
  const double Ki = ki;
  const double dt = 1.0;

  const double max = 1000000.0;
  const double min = -1000000.0;

  double error = static_cast<double>(tsize - size);
  double Pout = Kp * error;

  integral += error * dt;
  if (integral < 0.0) { integral = 0.0; }
  double Iout = Ki * integral;

  double derivative = (error - perror) / dt;
  double Dout = Kd * derivative;

  double output = Pout + Iout + Dout;
  output = std::max(min, std::min(max, output));

  sleep = static_cast<int>(output);
  if (sleep < 0) { sleep = 0; }
  perror = error;

  std::this_thread::sleep_for(std::chrono::microseconds(sleep));
}

//
// Misc utilities
//

bool CupDAQManager::IsForcedEndRunFile(bool useRC)
{
  if (!useRC) {
    if (std::filesystem::exists(kFORCEDENDRUNFILE)) { return true; }
  }

  return false;
}

void CupDAQManager::StartBenchmark(const char * name)
{
  std::lock_guard<std::mutex> lock(fBenchmarkMutex);
  fBenchmark->Start(name);
}

void CupDAQManager::StopBenchmark(const char * name)
{
  std::lock_guard<std::mutex> lock(fBenchmarkMutex);
  fBenchmark->Stop(name);
}
