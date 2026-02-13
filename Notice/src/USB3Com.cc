#include <algorithm> // for std::min
#include <cstring>
#include <unistd.h>

#include "DAQUtils/ELog.hh"
#include "Notice/USB3Com.hh"

namespace {

// Bulk endpoints addresses
constexpr uint8_t kUSB3_SF_READ = 0x82;  // IN Endpoint
constexpr uint8_t kUSB3_SF_WRITE = 0x06; // OUT Endpoint

// Programmer requests
constexpr uint8_t kNKPROGRAMMER_TARGET_USB3 = 0xF3;
constexpr uint8_t kNKPROGRAMMER_RESET_FIFO = 0xD7;
constexpr uint8_t kNKPROGRAMMER_CHECK_DONE = 0xD8;
constexpr uint8_t kNKPROGRAMMER_RESET_EP2 = 0xE2;
constexpr uint8_t kNKPROGRAMMER_RESET_EP6 = 0xE6;

// Programmer commands
constexpr uint16_t kNKPROGRAMMER_CMD_INIT = 10;
constexpr uint16_t kNKPROGRAMMER_CMD_CHECK_DONE = 11;

// Transfer constants
constexpr int kUSB3_BULK_BLOCK = 4096;               // Block size in bytes
constexpr int kUSB3_DATA_UNIT = 16384;               // Data unit size for chunked reads
constexpr unsigned int kUSB3_DEFAULT_TIMEOUT = 1000; // ms

} // anonymous namespace

USB3Com::USB3Com()
  : _vendorId(0),
    _productId(0),
    _sid(0)
{
}

USB3Com::USB3Com(uint16_t vendorId, uint16_t productId, int sid)
  : _vendorId(vendorId),
    _productId(productId),
    _sid(sid)
{
}

USB3Com::~USB3Com() { Close(); }

int USB3Com::Open() { return USBManager::Instance().OpenDevice(_vendorId, _productId, _sid); }

void USB3Com::Close() { USBManager::Instance().CloseDevice(_vendorId, _productId, _sid); }

int USB3Com::ClaimInterface(int interface)
{
  return USBManager::Instance().ClaimInterface(_vendorId, _productId, _sid, interface);
}

int USB3Com::ReleaseInterface(int interface)
{
  return USBManager::Instance().ReleaseInterface(_vendorId, _productId, _sid, interface);
}

int USB3Com::Write(uint32_t addr, uint32_t data) const
{
  const int length = 8;
  unsigned char buffer[length];

  // Pack data (Little Endian)
  buffer[0] = static_cast<unsigned char>(data & 0xFFu);
  buffer[1] = static_cast<unsigned char>((data >> 8) & 0xFFu);
  buffer[2] = static_cast<unsigned char>((data >> 16) & 0xFFu);
  buffer[3] = static_cast<unsigned char>((data >> 24) & 0xFFu);

  // Pack address
  buffer[4] = static_cast<unsigned char>(addr & 0xFFu);
  buffer[5] = static_cast<unsigned char>((addr >> 8) & 0xFFu);
  buffer[6] = static_cast<unsigned char>((addr >> 16) & 0xFFu);
  buffer[7] = static_cast<unsigned char>((addr >> 24) & 0x7Fu); // MSB 0 for Write

  libusb_device_handle * devh = Handle();
  if (!devh) {
    ERROR("Write failed: Device handle is null.");
    return LIBUSB_ERROR_NO_DEVICE;
  }

  int transferred = 0;
  int stat = libusb_bulk_transfer(devh, kUSB3_SF_WRITE, buffer, length, &transferred,
                                  kUSB3_DEFAULT_TIMEOUT);

  if (stat < 0) {
    ERROR("Write bulk transfer error: %s [sid=%d]", libusb_error_name(stat), _sid);
    return stat;
  }

  // Delay for hardware stability (required by some FPGA FSMs)
  usleep(1000);
  return stat;
}

int USB3Com::Read(uint32_t count, uint32_t addr, unsigned char * data, unsigned int timeout) const
{
  const int length = 8;
  unsigned char buffer[length];

  // Pack count
  buffer[0] = static_cast<unsigned char>(count & 0xFFu);
  buffer[1] = static_cast<unsigned char>((count >> 8) & 0xFFu);
  buffer[2] = static_cast<unsigned char>((count >> 16) & 0xFFu);
  buffer[3] = static_cast<unsigned char>((count >> 24) & 0xFFu);

  // Pack address
  buffer[4] = static_cast<unsigned char>(addr & 0xFFu);
  buffer[5] = static_cast<unsigned char>((addr >> 8) & 0xFFu);
  buffer[6] = static_cast<unsigned char>((addr >> 16) & 0xFFu);
  buffer[7] = static_cast<unsigned char>((addr >> 24) & 0x7Fu);
  buffer[7] |= 0x80u; // MSB 1 for Read

  libusb_device_handle * devh = Handle();
  if (!devh) {
    ERROR("Read failed: Device handle is null.");
    return LIBUSB_ERROR_NO_DEVICE;
  }

  int transferred = 0;
  // 1. Send Read Command
  int stat = libusb_bulk_transfer(devh, kUSB3_SF_WRITE, buffer, length, &transferred, timeout);
  if (stat < 0) {
    ERROR("Read command transfer error: %s [sid=%d]", libusb_error_name(stat), _sid);
    return stat;
  }

  // 2. Receive Data
  // Calculate total bytes: count is in 32-bit words, so bytes = count * 4
  int total_bytes = static_cast<int>(count * 4);
  int received_bytes = 0;

  // Perform chunked reads (16KB units) to satisfy potential hardware FIFO requirements
  while (received_bytes < total_bytes) {
    int chunk_size = std::min(total_bytes - received_bytes, kUSB3_DATA_UNIT);

    stat = libusb_bulk_transfer(devh, kUSB3_SF_READ, data + received_bytes, chunk_size,
                                &transferred, timeout);
    if (stat < 0) {
      ERROR("Read data transfer error: %s [sid=%d] at offset %d", libusb_error_name(stat), _sid,
            received_bytes);
      return stat;
    }
    received_bytes += transferred;
  }

  return 0;
}

int USB3Com::WriteControl(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char * data,
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

int USB3Com::ReadControl(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char * data,
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

uint32_t USB3Com::ReadReg(uint32_t addr, int * status) const
{
  uint32_t data_container = 0;
  // Read 1 word (4 bytes)
  int ret = Read(1u, addr, reinterpret_cast<unsigned char *>(&data_container));

  if (ret < 0) {
    ERROR("ReadReg failed at address 0x%X: %s [sid=%d]", addr, libusb_error_name(ret), _sid);
    if (status) *status = ret;
    return 0xFFFFFFFF; // Return sentinel value on error
  }

  if (status) *status = 0;
  return data_container; // System is Little Endian, so direct cast usually works.
}

int USB3Com::ReadRegI(uint32_t addr) const
{
  // Note: This function suppresses the error status.
  // Returns 0xFFFFFFFF (casted to -1) on failure, which might be ambiguous.
  return static_cast<int>(ReadReg(addr));
}

uint64_t USB3Com::ReadRegL(uint32_t addr, int * status) const
{
  uint64_t data_container = 0;
  // Read 2 words (8 bytes)
  int ret = Read(2u, addr, reinterpret_cast<unsigned char *>(&data_container));

  if (ret < 0) {
    ERROR("ReadRegL failed at address 0x%X: %s [sid=%d]", addr, libusb_error_name(ret), _sid);
    if (status) *status = ret;
    return 0xFFFFFFFFFFFFFFFF; // Return sentinel value on error
  }

  if (status) *status = 0;
  return data_container;
}

unsigned char USB3Com::CheckFPGADone() const
{
  unsigned char data = 0;
  ReadControl(kNKPROGRAMMER_TARGET_USB3, kNKPROGRAMMER_CMD_CHECK_DONE, 0, &data, 1);
  return data;
}

void USB3Com::InitFPGA() const
{
  unsigned char data = 0;
  WriteControl(kNKPROGRAMMER_TARGET_USB3, kNKPROGRAMMER_CMD_INIT, 0, &data, 1);
}

unsigned char USB3Com::CheckFPGADoneNoAVR() const
{
  unsigned char data = 0;
  ReadControl(kNKPROGRAMMER_CHECK_DONE, 0, 0, &data, 1);
  return data;
}

void USB3Com::InitFPGANoAVR() const
{
  unsigned char data = 0;
  WriteControl(kNKPROGRAMMER_RESET_FIFO, 0, 0, &data, 1);
}

void USB3Com::ResetEP2() const
{
  unsigned char data = 0;
  WriteControl(kNKPROGRAMMER_RESET_EP2, 0, 0, &data, 0);
}

void USB3Com::ResetEP6() const
{
  unsigned char data = 0;
  WriteControl(kNKPROGRAMMER_RESET_EP6, 0, 0, &data, 0);
}