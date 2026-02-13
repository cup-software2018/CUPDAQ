#include <chrono>
#include <cstdio>
#include <thread>
#include <unistd.h>

#include "DAQUtils/ELog.hh"
#include "Notice/NKAMOREADC.hh"

namespace {
constexpr uint16_t kVENDOR_ID = 0x0547;
constexpr uint16_t kPRODUCT_ID = 0x1000;

// Register Addresses
constexpr uint32_t kAddr_BCOUNT = 0x0Bu;
constexpr uint32_t kAddr_DATA = 0x40000000u;

// Constants
constexpr uint32_t kBlockSize = 256u; // Bytes per block
} // namespace

NKAMOREADC::NKAMOREADC()
  : _sid(0),
    _usb(kVENDOR_ID, kPRODUCT_ID, 0)
{
}

NKAMOREADC::NKAMOREADC(int sid)
  : _sid(sid),
    _usb(kVENDOR_ID, kPRODUCT_ID, sid)
{
}

NKAMOREADC::~NKAMOREADC() { Close(); }

void NKAMOREADC::SetSID(int sid)
{
  _sid = sid;
  _usb.Set(kVENDOR_ID, kPRODUCT_ID, _sid);
}

int NKAMOREADC::Open()
{
  int status = _usb.Open();
  if (status < 0) {
    ERROR("NKAMOREADC: Failed to open device (sid=%d)", _sid);
    return status;
  }

  status = _usb.ClaimInterface(0);
  if (status < 0) {
    ERROR("NKAMOREADC: Failed to claim interface 0 (sid=%d)", _sid);
    _usb.Close();
    return status;
  }

  if (!WaitFPGAAndInit()) {
    ERROR("NKAMOREADC: FPGA initialization failed (Timeout) [sid=%d]", _sid);
    _usb.ReleaseInterface(0);
    _usb.Close();
    return -1;
  }

  return 0;
}

void NKAMOREADC::Close()
{
  _usb.ReleaseInterface(0);
  _usb.Close();
}

uint32_t NKAMOREADC::ReadBCount() const
{
  // ReadReg now returns uint32_t directly (based on improved USB3Com)
  return _usb.ReadReg(kAddr_BCOUNT);
}

int NKAMOREADC::ReadData(uint32_t bcount, unsigned char * data, unsigned int timeout) const
{
  if (bcount == 0 || data == nullptr) {
    ERROR("NKAMOREADC: Invalid ReadData args (bcount=%u, data=%p)", bcount,
          static_cast<void *>(data));
    return -1;
  }

  // Calculate total bytes safely
  uint32_t total_bytes = bcount * kBlockSize;

  return _usb.Read(total_bytes, kAddr_DATA, data, timeout);
}

bool NKAMOREADC::WaitFPGAAndInit()
{
  int retry_count = 0;
  const int max_retries = 100; // 100 * 100ms = 10 seconds timeout

  while (retry_count < max_retries) {
    unsigned char done = _usb.CheckFPGADoneNoAVR();
    if (done) { break; }

    if (retry_count % 10 == 0) { // Log every second
      INFO_PROGRESS("FPGA initializing...", ".");
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    retry_count++;
  }

  if (retry_count >= max_retries) {
    INFO_PROGRESS_END("FAILED");
    return false;
  }

  INFO_PROGRESS_END("Done");

  _usb.InitFPGANoAVR();

  // Dummy reads to flush/stabilize the FPGA state
  for (int i = 0; i < 4; ++i) {
    ReadBCount();
  }

  INFO("NKAMOREADC[%d] is ready.", _sid);
  return true;
}