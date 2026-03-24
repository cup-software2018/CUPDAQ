#include <algorithm>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <iostream>

#include "DAQ/CupDAQManager.hh"

void CupDAQManager::PrintDAQSummary()
{
  const char * pname[4] = {"ReadData", "SortEvent", "BuildEvent", "WriteEvent"};

  std::cout << std::endl;
  std::cout << "====================================================" << std::endl;
  std::cout << Form("%12s %16s %16s", "process", "cpu time[s]", "real time[s]") << std::endl;
  std::cout << "----------------------------------------------------" << std::endl;
  double rt = 0.0;
  double ct = 0.0;
  for (int i = 0; i < 4; ++i) {
    double cputime = fBenchmark->GetCpuTime(pname[i]);
    double realtime = fBenchmark->GetRealTime(pname[i]);
    std::cout << Form("%12s %16.1f %16.1f", pname[i], cputime, realtime) << std::endl;
    ct += cputime;
    rt += realtime;
  }
  std::cout << "----------------------------------------------------" << std::endl;
  std::cout << Form("%12s %16.1f %16.1f", "total", ct, rt) << std::endl;
  std::cout << "====================================================" << std::endl;
  std::cout << std::endl;

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

  double recvDataSize = totalReadDataSize / kDGIGABYTES;
  double outputDataSize = fTotalWrittenDataSize / kDGIGABYTES;

  double trate = liveTime > 0.0 ? fTriggerNumber / liveTime : 0.0;
  double drate = liveTime > 0.0 ? recvDataSize * 1024.0 / liveTime : 0.0;
  double orate = liveTime > 0.0 ? outputDataSize * 1024.0 / liveTime : 0.0;

  std::cout << std::endl;
  std::cout << "************************* DAQ Summary *************************" << std::endl;
  std::cout << Form("%32s", "Run number : ") << fRunNumber << std::endl;
  std::cout << Form("%32s", "Start Time : ") << TDatime(fStartDatime).AsSQLString() << std::endl;
  std::cout << Form("%32s", "End Time : ") << TDatime(fEndDatime).AsSQLString() << std::endl;
  std::cout << std::endl;
  std::cout << Form("%32s", "Live time : ") << Form("%.1f", liveTime) << " [s]" << std::endl;
  std::cout << Form("%32s", "Total number of trigger : ") << Form("%d", fTriggerNumber)
            << std::endl;
  std::cout << Form("%32s", "Trigger rate : ") << Form("%.2f", trate) << " [Hz]" << std::endl;
  std::cout << Form("%32s", "Total number of event : ") << Form("%d", fNBuiltEvent) << std::endl;
  std::cout << Form("%32s", "Software Trigger efficiency : ")
            << Form("%5.2f [%%]", fSoftTrigger->GetEfficiency()) << std::endl;
  std::cout << std::endl;
  std::cout << Form("%32s", "Received data size : ")
            << Form("%.3f GBytes (%.3f MB/sec)", recvDataSize, drate) << std::endl;
  std::cout << Form("%32s", "Written data size : ")
            << Form("%.3f GBytes (%.3f MB/sec)", outputDataSize, orate) << std::endl;
  std::cout << "***************************************************************" << std::endl;
  std::cout << std::endl;
}

bool CupDAQManager::ThreadWait(unsigned long & state, bool & exit)
{
  while (true) {
    if (RUNSTATE::CheckState(state, RUNSTATE::kRUNNING)) { break; }
    if (RUNSTATE::CheckError(state)) { return false; }
    if (exit) { return false; }
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return true;
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

nlohmann::json CupDAQManager::SendCommandToDAQ(const std::unique_ptr<zmq::socket_t> & socket_ptr,
                                               const std::string & cmd,
                                               std::string & daq_name)
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
    return err_json;
  }

  zmq::message_t reply;
  if (!socket_ptr->recv(reply, zmq::recv_flags::none)) {
    daq_name = "Unknown";
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

nlohmann::json CupDAQManager::SendCommandToDAQ(const std::unique_ptr<zmq::socket_t> & socket_ptr,
                                               const std::string & cmd)
{
  std::string dummy_name;
  return SendCommandToDAQ(socket_ptr, cmd, dummy_name);
}

void CupDAQManager::SendCommandToDAQs(const std::string & cmd)
{
  for (auto & socket_ptr : fDAQSocket) {
    if (!socket_ptr) { continue; }

    SendCommandToDAQ(socket_ptr, cmd);
  }
}

unsigned long CupDAQManager::QueryDAQStatus(const std::unique_ptr<zmq::socket_t> & socket_ptr,
                                            std::string & daq_name)
{
  nlohmann::json reply = SendCommandToDAQ(socket_ptr, "kQUERYDAQSTATUS", daq_name);

  if (reply.value("status", "error") == "ok") { return reply.value("run_status", 0ul); }

  return 0;
}

unsigned long CupDAQManager::QueryDAQStatus(const std::unique_ptr<zmq::socket_t> & socket_ptr)
{
  std::string dummy_name;
  return QueryDAQStatus(socket_ptr, dummy_name);
}


bool CupDAQManager::WaitDAQStatus(RUNSTATE::STATE state)
{
  while (true) {
    bool totalstate = true;
    for (auto & socket_ptr : fDAQSocket) {
      if (!socket_ptr) continue;

      std::string daq_name;
      unsigned long daqstate = QueryDAQStatus(socket_ptr, daq_name);

      if (daqstate == 0) {
        Error("%s connection down", daq_name.c_str());
        socket_ptr.reset();
        return false;
      }
      if (RUNSTATE::CheckError(daqstate)) {
        Error("%s got error", daq_name.c_str());
        return false;
      }
      if (!RUNSTATE::CheckState(daqstate, state)) { totalstate = false; }
    }
    if (totalstate) break;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return true;
}

bool CupDAQManager::IsDAQRunning()
{
  bool retval = true;
  for (auto & socket_ptr : fDAQSocket) {
    if (!socket_ptr) continue;

    std::string daq_name;
    unsigned long daqstate = QueryDAQStatus(socket_ptr, daq_name);

    if (daqstate == 0) {
      Error("%s connection down", daq_name.c_str());
      socket_ptr.reset();
      retval = false;
    }
    else if (!RUNSTATE::CheckState(daqstate, RUNSTATE::kRUNNING)) {
      Error("%s is not running", daq_name.c_str());
      if (RUNSTATE::CheckError(daqstate)) { Error("error in %s", daq_name.c_str()); }
      retval = false;
    }
  }

  return retval;
}

bool CupDAQManager::IsDAQFail()
{
  bool retval = false;
  for (auto & socket_ptr : fDAQSocket) {
    if (!socket_ptr) continue;

    std::string daq_name;
    unsigned long daqstate = QueryDAQStatus(socket_ptr, daq_name);

    if (daqstate == 0) {
      Error("%s connection down", daq_name.c_str());
      socket_ptr.reset();
      retval = true;
    }
    else if (RUNSTATE::CheckError(daqstate)) {
      Error("error in %s", daq_name.c_str());
      retval = true;
    }
  }

  return retval;
}

bool CupDAQManager::WaitState(unsigned long & state, RUNSTATE::STATE pstate, bool errorexit)
{
  while (true) {
    if (RUNSTATE::CheckState(state, pstate)) break;
    if (errorexit && RUNSTATE::CheckError(state)) return false;

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return true;
}

int CupDAQManager::WaitCommand(bool & isgo)
{
  while (true) {
    if (IsDAQFail()) { return -1; }
    if (isgo) { break; }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return 0;
}

int CupDAQManager::WaitCommand(bool & isgo, bool & exit)
{
  while (true) {
    if (IsDAQFail()) { return -1; }
    if (exit) { return 1; }
    if (isgo) { break; }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return 0;
}

int CupDAQManager::WaitCommand(bool & isgo, unsigned long & state)
{
  while (true) {
    if (IsDAQFail()) { return -1; }
    if (RUNSTATE::CheckError(state)) { return 1; }
    if (isgo) { break; }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  return 0;
}

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
