#include <vector>

#include "AMORESystem/AMORETCB.hh"
#include "DAQUtils/ELog.hh"

ClassImp(AMORETCB)

AMORETCB & AMORETCB::Instance()
{
  static AMORETCB instance;
  return instance;
}

int AMORETCB::Open() { return fNKTCB.Open(); }

void AMORETCB::Close()
{
  fNKTCB.Stop();
  fNKTCB.Reset();
  fNKTCB.Close();
}

bool AMORETCB::Config()
{
  INFO("module configuration start");

  if (fConfigs == nullptr) {
    ERROR("no config list, check configuration");
    return false;
  }

  int nconf = fConfigs->GetEntries();

  if (nconf == 0) {
    ERROR("no module in config list");
    return false;
  }

  int nlinked = CheckLinkStatus();
  if (nlinked == 0) {
    ERROR("no module linked");
    return false;
  }

  fNKTCB.Reset();

  bool retval = true;
  for (int i = 0; i < nconf; i++) {
    auto * conf = static_cast<AbsConf *>(fConfigs->At(i));
    if (!conf->IsEnabled() || !conf->IsLinked()) continue;

    int mid = conf->MID();
    TString name = conf->GetName();

    fNKTCB.WriteDRAMON(mid, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    unsigned long dramon = fNKTCB.ReadDRAMON(mid);
    if (dramon) { INFO("%s[mid=%2lu] DRAM on", name.Data(), mid); }
    else {
      ERROR("%s[mid=%2lu] error occurred during turning DRAM on", name.Data(), mid);
      return false;
    }

    if (name.Contains("AMORETCB")) { retval &= ConfigAMORETCB(static_cast<AMORETCBConf *>(conf)); }
    else if (name.Contains("AMOREADC")) {
      unsigned long bcount = fNKTCB.ReadBCount(mid);
      if (bcount) {
        std::vector<unsigned char> data(bcount * kKILOBYTES);
        fNKTCB.ReadData(mid, bcount, data.data());
      }
      retval &= ConfigAMOREADC(static_cast<AMOREADCConf *>(conf));
    }
    else {
      WARNING("unknown kind of module : %s", name.Data());
      continue;
    }
  }

  if (retval) { INFO("all modules configuration done"); }
  return true;
}

// tcb
void AMORETCB::Reset() { fNKTCB.Reset(); }

void AMORETCB::ResetTimer() { fNKTCB.ResetTimer(); }

void AMORETCB::TriggerStart()
{
  fNKTCB.Reset();
  fNKTCB.ResetTimer();
  fNKTCB.Start();
}

void AMORETCB::TriggerStop()
{
  std::lock_guard<std::mutex> lock(fMutex);
  fNKTCB.Stop();
  fNKTCB.Reset();
}

unsigned long AMORETCB::ReadBCount(unsigned long mid)
{
  std::lock_guard<std::mutex> lock(fMutex);
  return fNKTCB.ReadBCount(mid);
}

int AMORETCB::ReadData(unsigned long mid, unsigned long bcount, unsigned char * data)
{
  std::lock_guard<std::mutex> lock(fMutex);
  return fNKTCB.ReadData(mid, bcount, data);
}

void AMORETCB::WriteSR(unsigned long mid, unsigned long data) { fNKTCB.WriteSR(mid, 0, data); }

unsigned long AMORETCB::ReadSR(unsigned long mid) { return fNKTCB.ReadSR(mid, 0); }

bool AMORETCB::ConfigAMORETCB(AMORETCBConf * conf)
{
  conf->PrintConf();
  std::cout << Form(" ++ AMORETCB register: nothing much to said") << std::endl;

  return true;
}

bool AMORETCB::ConfigAMOREADC(AMOREADCConf * conf)
{
  unsigned long sid = conf->SID();
  unsigned long mid = conf->MID();

  if (!conf->IsEnabled()) {
    INFO("%s[mid=%2lu] is disabled, passed", conf->GetName(), mid);
    return true;
  }

  if (!conf->IsLinked()) {
    ERROR("%s[mid=%2lu] is enabled but not linked", conf->GetName(), mid);
    return false;
  }

  conf->PrintConf();

  unsigned long sr = conf->SR();
  fNKTCB.WriteSR(mid, 0, sr);
  sr = fNKTCB.ReadSR(mid, 0);

  std::cout << Form(" ++ AMOREADC resgister: SID(%d) MID(%d) SR(%d) ", sid, mid, sr) << std::endl;

  return true;
}

int AMORETCB::CheckLinkStatus()
{
  const int nport = 8;

  unsigned long data[2];
  fNKTCB.ReadLNSTAT(data);

  int linked[nport];
  for (unsigned int i = 0; i < nport; i++) {
    linked[i] = (data[0] >> i) & 0x1;
  }

  // get mid
  unsigned long linkedMID[40];
  fNKTCB.ReadMIDS(linkedMID);

  int nlinked = 0;
  for (int i = 0; i < nport; i++) {
    if (linked[i]) {
      AbsConf * conf = fConfigs->FindConfig(ADC::AMOREADC, linkedMID[i]);

      if (!conf) continue;

      conf->SetLink();

      if (conf->IsEnabled()) {
        INFO("AMOREADC[mid=%2ld] found @ %d", linkedMID[i], i + 1);
        nlinked += 1;
      }
      else {
        WARNING("AMOREADC[mid=%2ld] found @ %d, but disabled", linkedMID[i], i + 1);
      }
    }
  }

  return nlinked;
}