#include "AMOREDAQ/AMORETCB.hh"

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