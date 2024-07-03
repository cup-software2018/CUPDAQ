#include "DAQ/CupDAQManager.hh"
#include <ctime>

using namespace std;

void CupDAQManager::PrintDAQSummary()
{
  const char * pname[4] = {"ReadData", "SortEvent", "BuildEvent", "WriteEvent"};

  cout << endl;
  cout << "====================================================" << endl;
  cout << Form("%12s %16s %16s", "process", "cpu time[s]", "real time[s]")
       << endl;
  cout << "----------------------------------------------------" << endl;
  double rt = 0, ct = 0;
  for (int i = 0; i < 4; i++) {
    double cputime = fBenchmark->GetCpuTime(pname[i]);
    double realtime = fBenchmark->GetRealTime(pname[i]);
    cout << Form("%12s %16.1f %16.1f", pname[i], cputime, realtime) << endl;
    ct += cputime;
    rt += realtime;
  }
  cout << "----------------------------------------------------" << endl;
  cout << Form("%12s %16.1f %16.1f", "total", ct, rt) << endl;
  cout << "====================================================" << endl;
  cout << endl;

  unsigned long totalReadDataSize;
  double liveTime;

  int nadc = GetEntries();
  if (nadc > 0) {
    auto * theADC = (AbsADC *)fCont[0];
    totalReadDataSize = nadc * theADC->GetTotalBCount() * kKILOBYTES;
    liveTime = theADC->GetCurrentTime() / kDONESECOND;
  }
  else {
    totalReadDataSize = fTotalRawDataSize;
    liveTime = difftime(fEndDatime, fStartDatime);
  }

  double recvDataSize = totalReadDataSize / kDGIGABYTES;
  double outputDataSize = fTotalWrittenDataSize / kDGIGABYTES;

  double trate = liveTime > 0 ? fTriggerNumber / liveTime : 0;
  double drate = liveTime > 0 ? recvDataSize * 1024 / liveTime : 0;
  double orate = liveTime > 0 ? outputDataSize * 1024 / liveTime : 0;

  cout << endl;
  cout << "************************* DAQ Summary *************************"
       << endl;
  cout << Form("%32s", "Run number : ") << fRunNumber << endl;
  cout << Form("%32s", "Start Time : ") << TDatime(fStartDatime).AsSQLString()
       << endl;
  cout << Form("%32s", "End Time : ") << TDatime(fEndDatime).AsSQLString()
       << endl;
  cout << endl;
  cout << Form("%32s", "Live time : ") << Form("%.1f", liveTime) << " [s]"
       << endl;
  cout << Form("%32s", "Total number of trigger : ")
       << Form("%d", fTriggerNumber) << endl;
  cout << Form("%32s", "Trigger rate : ") << Form("%.2f", trate) << " [Hz]"
       << endl;
  cout << Form("%32s", "Total number of event : ") << Form("%d", fNBuiltEvent)
       << endl;
  cout << Form("%32s", "Software Trigger efficiency : ")
       << Form("%5.2f [%%]", fSoftTrigger->GetEfficiency()) << endl;
  cout << endl;
  cout << Form("%32s", "Received data size : ")
       << Form("%.3f GBytes (%.3f MB/sec)", recvDataSize, drate) << endl;
  cout << Form("%32s", "Written data size : ")
       << Form("%.3f GBytes (%.3f MB/sec)", outputDataSize, orate) << endl;
  cout << "***************************************************************"
       << endl;
  cout << endl;
}

bool CupDAQManager::ThreadWait(unsigned long & state, bool & exit) const
{
  while (true) {
    if (RUNSTATE::CheckState(state, RUNSTATE::kRUNNING)) { break; }
    else if (RUNSTATE::CheckError(state)) {
      return false;
    }
    if (exit) { return false; }
    gSystem->Sleep(10);
  }
  return true;
}

void CupDAQManager::ThreadSleep(int & sleep, double & perror, double & integral,
                                int size, int tsize, double ki)
{
  if (!RUNSTATE::CheckState(fRunStatus, RUNSTATE::kRUNNING)) return;

  const double Kp = sleep / 20.;
  const double Kd = sleep / 10.;
  const double Ki = ki;
  const double dt = 1;

  const double max = 1000000;
  const double min = -1000000;

  double error = tsize - size;
  double Pout = Kp * error;

  integral += error * dt;
  if (integral < 0) integral = 0;
  double Iout = Ki * integral;

  double derivative = (error - perror) / dt;
  double Dout = Kd * derivative;

  double output = Pout + Iout + Dout;
  output = std::max(min, std::min(max, output));

  sleep = int(output);
  if (sleep < 0) sleep = 0;
  perror = error;

  std::this_thread::sleep_for(std::chrono::microseconds(sleep));
}

unsigned long CupDAQManager::QueryDAQStatus(TSocket * socket) const
{
  char data[kMESSLEN];
  unsigned long mess1, mess2;
  unsigned long mess3, mess4;

  EncodeMsg(data, kQUERYDAQSTATUS);
  if (socket->SendRaw(data, kMESSLEN) < 0) { return 0; }
  memset(data, 0, kMESSLEN);
  if (socket->RecvRaw(data, kMESSLEN) < 0) { return 0; }
  DecodeMsg(data, mess1, mess2, mess3, mess4);

  return mess1;
}

void CupDAQManager::SendCommandToDAQ(unsigned long cmd) const
{
  char data[kMESSLEN];
  EncodeMsg(data, cmd);

  for (auto * socket : fDAQSocket) {
    if (socket == nullptr) continue;
    socket->SendRaw(data, kMESSLEN);
  }
}

bool CupDAQManager::WaitDAQStatus(RUNSTATE::STATE state) const
{
  while (true) {
    bool totalstate = true;
    for (auto * socket : fDAQSocket) {
      if (socket == nullptr) continue;
      unsigned long daqstate = QueryDAQStatus(socket);
      if (daqstate == 0) {
        fLog->Error("CupDAQManager::WaitDAQStatus", "%s connection down",
                    socket->GetName());
        socket->Close();
        delete socket;
        socket = nullptr;
        return false;
      }
      if (RUNSTATE::CheckError(daqstate)) {
        fLog->Error("CupDAQManager::WaitDAQStatus", "%s got error",
                    socket->GetName());
        return false;
      }
      if (!RUNSTATE::CheckState(daqstate, state)) { totalstate &= false; }
    }
    if (totalstate) break;

    gSystem->Sleep(10);
  }
  return true;
}

bool CupDAQManager::IsDAQRunning() const
{
  bool retval = true;
  for (auto * socket : fDAQSocket) {
    if (socket == nullptr) continue;
    unsigned long daqstate = QueryDAQStatus(socket);
    if (daqstate == 0) {
      fLog->Error("CupDAQManager::IsDAQRunning", "%s connection down",
                  socket->GetName());
      socket->Close();
      delete socket;
      socket = nullptr;
      retval = false;
    }
    else if (!RUNSTATE::CheckState(daqstate, RUNSTATE::kRUNNING)) {
      fLog->Error("CupDAQManager::IsDAQRunning", "%s is not running",
                  socket->GetName());
      if (RUNSTATE::CheckError(daqstate))
        fLog->Error("CupDAQManager::IsDAQRunning", "error in %s",
                    socket->GetName());
      retval = false;
    }
  }
  return retval;
}

bool CupDAQManager::IsDAQFail() const
{
  bool retval = false;
  for (auto * socket : fDAQSocket) {
    if (socket == nullptr) continue;
    unsigned long daqstate = QueryDAQStatus(socket);
    if (daqstate == 0) {
      fLog->Error("CupDAQManager::IsDAQFail", "%s connection down",
                  socket->GetName());
      socket->Close();
      delete socket;
      socket = nullptr;
      retval = true;
    }
    else if (RUNSTATE::CheckError(daqstate)) {
      fLog->Error("CupDAQManager::IsDAQFail", "error in %s", socket->GetName());
      retval = true;
    }
  }
  return retval;
}

bool CupDAQManager::WaitState(unsigned long & state, RUNSTATE::STATE pstate,
                              bool errorexit) const
{
  while (true) {
    if (RUNSTATE::CheckState(state, pstate)) break;
    if (errorexit && RUNSTATE::CheckError(state)) return false;
    gSystem->Sleep(10);
  }
  return true;
}

int CupDAQManager::WaitCommand(bool & isgo) const
{
  while (true) {
    if (IsDAQFail()) return -1;
    if (isgo) break;
    gSystem->Sleep(10);
  }
  return 0;
}

int CupDAQManager::WaitCommand(bool & isgo, bool & exit) const
{
  while (true) {
    if (IsDAQFail()) return -1;
    if (exit) return 1;
    if (isgo) break;
    gSystem->Sleep(10);
  }
  return 0;
}

int CupDAQManager::WaitCommand(bool & isgo, unsigned long & state) const
{
  while (true) {
    if (IsDAQFail()) return -1;
    if (RUNSTATE::CheckError(state)) return 1;
    if (isgo) break;
    gSystem->Sleep(10);
  }
  return 0;
}

void CupDAQManager::EncodeMsg(char * buffer, unsigned long message1,
                              unsigned long message2, unsigned long message3,
                              unsigned long message4) const
{
  memset(buffer, 0, kMESSLEN);

  for (int i = 0; i < 8; i++) {
    buffer[i] = (message1 >> 8 * i) & 0xFF;
    buffer[i + 8] = (message2 >> 8 * i) & 0xFF;
    buffer[i + 16] = (message3 >> 8 * i) & 0xFF;
    buffer[i + 24] = (message4 >> 8 * i) & 0xFF;
  }
}

void CupDAQManager::DecodeMsg(char * buffer, unsigned long & message1,
                              unsigned long & message2,
                              unsigned long & message3,
                              unsigned long & message4) const
{
  message1 = 0;
  message2 = 0;
  message3 = 0;
  message4 = 0;

  for (int i = 0; i < 8; i++) {
    message1 += (unsigned long)((buffer[i] & 0xFF) << 8 * i);
    message2 += (unsigned long)((buffer[i + 8] & 0xFF) << 8 * i);
    message3 += (unsigned long)((buffer[i + 16] & 0xFF) << 8 * i);
    message4 += (unsigned long)((buffer[i + 24] & 0xFF) << 8 * i);
  }
}

bool CupDAQManager::IsForcedEndRunFile(bool useRC) const
{
  if (!useRC) {
    std::ifstream status;
    status.open(kFORCEDENDRUNFILE);
    if (status.is_open()) {
      status.close();
      return true;
    }
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