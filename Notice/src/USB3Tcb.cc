#include <algorithm> // for std::min
#include <cstring>
#include <unistd.h>

#include "DAQUtils/ELog.hh"
#include "Notice/USB3Tcb.hh"

namespace {

// Bulk endpoints
constexpr uint8_t kUSB3_SF_READ = 0x82;
constexpr uint8_t kUSB3_SF_WRITE = 0x06;

// Programmer requests
constexpr uint8_t kNKPROGRAMMER_TARGET_USB3 = 0xF3;
constexpr uint8_t kNKPROGRAMMER_RESET_FIFO = 0xD7;
constexpr uint8_t kNKPROGRAMMER_CHECK_DONE = 0xD8;

// Programmer commands
constexpr uint16_t kNKPROGRAMMER_CMD_INIT = 10;
constexpr uint16_t kNKPROGRAMMER_CMD_CHECK_DONE = 11;

// Transfer constants
constexpr int kUSB3_DATA_UNIT = 16384; // 16KB chunk size
constexpr unsigned int kUSB3_DEFAULT_TIMEOUT = 5000;

} // anonymous namespace

USB3Tcb::USB3Tcb()
  : _vendorId(0),
    _productId(0),
    _sid(0)
{
}

USB3Tcb::USB3Tcb(uint16_t vendorId, uint16_t productId, int sid)
  : _vendorId(vendorId),
    _productId(productId),
    _sid(sid)
{
}

USB3Tcb::~USB3Tcb() { Close(); }

int USB3Tcb::Open() { return USBManager::Instance().OpenDevice(_vendorId, _productId, _sid); }

void USB3Tcb::Close() { USBManager::Instance().CloseDevice(_vendorId, _productId, _sid); }

int USB3Tcb::ClaimInterface(int interface)
{
  return USBManager::Instance().ClaimInterface(_vendorId, _productId, _sid, interface);
}

int USB3Tcb::ReleaseInterface(int interface)
{
  return USBManager::Instance().ReleaseInterface(_vendorId, _productId, _sid, interface);
}

int USB3Tcb::Write(uint32_t mid, uint32_t addr, uint32_t data) const
{
  const int length = 12; // Packet size for TCB: Data(4) + Addr(4) + MID(4)
  unsigned char buffer[length];

  // 1. Pack Data
  buffer[0] = static_cast<unsigned char>(data & 0xFFu);
  buffer[1] = static_cast<unsigned char>((data >> 8) & 0xFFu);
  buffer[2] = static_cast<unsigned char>((data >> 16) & 0xFFu);
  buffer[3] = static_cast<unsigned char>((data >> 24) & 0xFFu);

  // 2. Pack Address (MSB 0 for Write)
  buffer[4] = static_cast<unsigned char>(addr & 0xFFu);
  buffer[5] = static_cast<unsigned char>((addr >> 8) & 0xFFu);
  buffer[6] = static_cast<unsigned char>((addr >> 16) & 0xFFu);
  buffer[7] = static_cast<unsigned char>((addr >> 24) & 0x7Fu);

  // 3. Pack Module ID (MID)
  buffer[8] = static_cast<unsigned char>(mid & 0xFFu);
  buffer[9] = static_cast<unsigned char>((mid >> 8) & 0xFFu);
  buffer[10] = static_cast<unsigned char>((mid >> 16) & 0xFFu);
  buffer[11] = static_cast<unsigned char>((mid >> 24) & 0xFFu);

  libusb_device_handle * devh = Handle();
  if (!devh) {
    ERROR("TCB Write failed: Device handle is null.");
    return LIBUSB_ERROR_NO_DEVICE;
  }

  int transferred = 0;
  int stat = libusb_bulk_transfer(devh, kUSB3_SF_WRITE, buffer, length, &transferred,
                                  kUSB3_DEFAULT_TIMEOUT);

  if (stat < 0) {
    ERROR("TCB Write error: %s [sid=%d]", libusb_error_name(stat), _sid);
    return stat;
  }

  usleep(1000); // Hardware stability delay
  return stat;
}

int USB3Tcb::Read(uint32_t mid, uint32_t count, uint32_t addr, unsigned char * data,
                  unsigned int timeout) const
{
  const int length = 12;
  unsigned char buffer[length];

  // 1. Pack Count
  buffer[0] = static_cast<unsigned char>(count & 0xFFu);
  buffer[1] = static_cast<unsigned char>((count >> 8) & 0xFFu);
  buffer[2] = static_cast<unsigned char>((count >> 16) & 0xFFu);
  buffer[3] = static_cast<unsigned char>((count >> 24) & 0xFFu);

  // 2. Pack Address (MSB 1 for Read)
  buffer[4] = static_cast<unsigned char>(addr & 0xFFu);
  buffer[5] = static_cast<unsigned char>((addr >> 8) & 0xFFu);
  buffer[6] = static_cast<unsigned char>((addr >> 16) & 0xFFu);
  buffer[7] = static_cast<unsigned char>((addr >> 24) & 0x7Fu);
  buffer[7] |= 0x80u; // Set Read Flag

  // 3. Pack Module ID (MID)
  buffer[8] = static_cast<unsigned char>(mid & 0xFFu);
  buffer[9] = static_cast<unsigned char>((mid >> 8) & 0xFFu);
  buffer[10] = static_cast<unsigned char>((mid >> 16) & 0xFFu);
  buffer[11] = static_cast<unsigned char>((mid >> 24) & 0xFFu);

  libusb_device_handle * devh = Handle();
  if (!devh) {
    ERROR("TCB Read failed: Device handle is null.");
    return LIBUSB_ERROR_NO_DEVICE;
  }

  int transferred = 0;
  // Send Command
  int stat = libusb_bulk_transfer(devh, kUSB3_SF_WRITE, buffer, length, &transferred, 5000);
  if (stat < 0) {
    ERROR("TCB Read command error: %s [sid=%d]", libusb_error_name(stat), _sid);
    return stat;
  }

  // Receive Data
  // Convert word count to byte count
  int total_bytes = static_cast<int>(count * 4);
  int received_bytes = 0;

  // Chunked read loop (handling 16KB units or remaining bytes)
  while (received_bytes < total_bytes) {
    int chunk_size = std::min(total_bytes - received_bytes, kUSB3_DATA_UNIT);

    stat = libusb_bulk_transfer(devh, kUSB3_SF_READ, data + received_bytes, chunk_size,
                                &transferred, timeout);
    if (stat < 0) {
      ERROR("TCB Read data error: %s [sid=%d] offset=%d chunk=%d transferred=%d timeout=%u stat=%d",
            libusb_error_name(stat), _sid, received_bytes, chunk_size, transferred, timeout, stat);
      return stat;
    }
    received_bytes += transferred;
  }

  return 0;
}

int USB3Tcb::WriteControl(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char * data,
                          uint16_t wLength) const
{
  libusb_device_handle * devh = Handle();
  if (!devh) {
    ERROR("WriteControl failed: Device handle is null.");
    return LIBUSB_ERROR_NO_DEVICE;
  }

  int stat =
      libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT, bRequest,
                              wValue, wIndex, data, wLength, kUSB3_DEFAULT_TIMEOUT);
  if (stat < 0) { ERROR("WriteControl error: %s [sid=%d]", libusb_error_name(stat), _sid); }
  return stat;
}

int USB3Tcb::ReadControl(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char * data,
                         uint16_t wLength) const
{
  libusb_device_handle * devh = Handle();
  if (!devh) {
    ERROR("ReadControl failed: Device handle is null.");
    return LIBUSB_ERROR_NO_DEVICE;
  }

  int stat =
      libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, bRequest,
                              wValue, wIndex, data, wLength, kUSB3_DEFAULT_TIMEOUT);
  if (stat < 0) { ERROR("ReadControl error: %s [sid=%d]", libusb_error_name(stat), _sid); }
  return stat;
}

uint32_t USB3Tcb::ReadReg(uint32_t mid, uint32_t addr, int * status) const
{
  uint32_t data_container = 0;
  // Read 1 word (4 bytes)
  int ret = Read(mid, 1u, addr, reinterpret_cast<unsigned char *>(&data_container));

  if (ret < 0) {
    ERROR("ReadReg failed (mid=%u, addr=0x%X): %s", mid, addr, libusb_error_name(ret));
    if (status) *status = ret;
    return 0xFFFFFFFF; // Error sentinel
  }

  if (status) *status = 0;
  return data_container;
}

int USB3Tcb::ReadRegI(uint32_t mid, uint32_t addr) const
{
  return static_cast<int>(ReadReg(mid, addr));
}

uint64_t USB3Tcb::ReadRegL(uint32_t mid, uint32_t addr, int * status) const
{
  uint64_t data_container = 0;
  // Read 2 words (8 bytes)
  int ret = Read(mid, 2u, addr, reinterpret_cast<unsigned char *>(&data_container));

  if (ret < 0) {
    ERROR("ReadRegL failed (mid=%u, addr=0x%X): %s", mid, addr, libusb_error_name(ret));
    if (status) *status = ret;
    return 0xFFFFFFFFFFFFFFFF; // Error sentinel
  }

  if (status) *status = 0;
  return data_container;
}

unsigned char USB3Tcb::CheckFPGADone() const
{
  unsigned char data = 0;
  ReadControl(kNKPROGRAMMER_TARGET_USB3, kNKPROGRAMMER_CMD_CHECK_DONE, 0, &data, 1);
  return data;
}

void USB3Tcb::InitFPGA() const
{
  unsigned char data = 0;
  WriteControl(kNKPROGRAMMER_TARGET_USB3, kNKPROGRAMMER_CMD_INIT, 0, &data, 1);
}

unsigned char USB3Tcb::CheckFPGADoneNoAVR() const
{
  unsigned char data = 0;
  ReadControl(kNKPROGRAMMER_CHECK_DONE, 0, 0, &data, 1);
  return data;
}

void USB3Tcb::InitFPGANoAVR() const
{
  unsigned char data = 0;
  WriteControl(kNKPROGRAMMER_RESET_FIFO, 0, 0, &data, 1);
}