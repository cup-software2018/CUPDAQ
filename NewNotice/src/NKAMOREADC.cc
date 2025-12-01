#include <chrono>
#include <cstdio>
#include <thread>
#include <unistd.h>

#include "DAQUtils/ELog.hh"
#include "NewNotice/NKAMOREADC.hh"

namespace {
constexpr uint16_t kNKAMOREADC_VENDOR_ID = 0x0547;
constexpr uint16_t kNKAMOREADC_PRODUCT_ID = 0x1000;

constexpr uint32_t kNKAMOREADC_REG_BCOUNT = 0x0Bu;
constexpr uint32_t kNKAMOREADC_ADDR_DATA = 0x40000000u;
} // namespace

NKAMOREADC::NKAMOREADC(int sid)
  : _sid(sid),
    _usb(kNKAMOREADC_VENDOR_ID, kNKAMOREADC_PRODUCT_ID, sid)
{
}

NKAMOREADC::~NKAMOREADC() { Close(); }

int NKAMOREADC::Open()
{
  int status = _usb.Open();
  if (status < 0) {
    ERROR("NKAMOREADC: failed to open device (sid=%d)", _sid);
    return status;
  }

  status = _usb.ClaimInterface(0);
  if (status < 0) {
    ERROR("NKAMOREADC: failed to claim interface 0 (sid=%d)", _sid);
    _usb.Close();
    return status;
  }

  WaitFPGAAndInit();

  return 0;
}

void NKAMOREADC::Close()
{
  _usb.ReleaseInterface(0);
  _usb.Close();
}

int NKAMOREADC::ReadBCount() const { return _usb.ReadRegI(kNKAMOREADC_REG_BCOUNT); }

int NKAMOREADC::ReadData(int bcount, unsigned char * data, unsigned int timeout) const
{
  if (bcount <= 0 || data == nullptr) {
    ERROR("NKAMOREADC: invalid ReadData arguments (bcount=%d, data=%p)", bcount, static_cast<void *>(data));
    return -1;
  }

  int count = bcount * 256;
  return _usb.Read(static_cast<uint32_t>(count), kNKAMOREADC_ADDR_DATA, data, timeout);
}

void NKAMOREADC::WaitFPGAAndInit()
{
  while (true) {
    unsigned char done = _usb.CheckFPGADoneNoAVR();
    if (done) { break; }

    INFO_PROGRESS("FPGA init", ".");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  INFO_PROGRESS_END("done");

  _usb.InitFPGANoAVR();

  ReadBCount();
  ReadBCount();
  ReadBCount();
  ReadBCount();

  INFO("now NKAMOREADC[%d] is ready.", _sid);
}
