// Notice/USB3Com.cc
#include <unistd.h>

#include "DAQUtils/ELog.hh"
#include "Notice/USB3Com.hh"

namespace {

// Bulk endpoints
constexpr uint8_t kUSB3_SF_READ = 0x82;
constexpr uint8_t kUSB3_SF_WRITE = 0x06;

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
constexpr int kUSB3_BULK_BLOCK = 4096;
constexpr int kUSB3_DATA_UNIT = 16384;

constexpr unsigned int kUSB3_DEFAULT_TIMEOUT = 1000;

} // namespace

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

  buffer[0] = static_cast<unsigned char>(data & 0xFFu);
  buffer[1] = static_cast<unsigned char>((data >> 8) & 0xFFu);
  buffer[2] = static_cast<unsigned char>((data >> 16) & 0xFFu);
  buffer[3] = static_cast<unsigned char>((data >> 24) & 0xFFu);

  buffer[4] = static_cast<unsigned char>(addr & 0xFFu);
  buffer[5] = static_cast<unsigned char>((addr >> 8) & 0xFFu);
  buffer[6] = static_cast<unsigned char>((addr >> 16) & 0xFFu);
  buffer[7] = static_cast<unsigned char>((addr >> 24) & 0x7Fu);

  libusb_device_handle * devh = Handle();
  if (!devh) {
    ERROR("could not get device handle for the device");
    return -1;
  }

  int stat = 0;
  int transferred = 0;

  stat = libusb_bulk_transfer(devh, kUSB3_SF_WRITE, buffer, length, &transferred, kUSB3_DEFAULT_TIMEOUT);
  if (stat < 0) {
    ERROR("write error:%s [sid=%d]", libusb_error_name(stat), _sid);
    return stat;
  }

  usleep(1000);
  return stat;
}

int USB3Com::Read(uint32_t count, uint32_t addr, unsigned char * data, unsigned int timeout) const
{
  const int length = 8;
  unsigned char buffer[length];

  buffer[0] = static_cast<unsigned char>(count & 0xFFu);
  buffer[1] = static_cast<unsigned char>((count >> 8) & 0xFFu);
  buffer[2] = static_cast<unsigned char>((count >> 16) & 0xFFu);
  buffer[3] = static_cast<unsigned char>((count >> 24) & 0xFFu);

  buffer[4] = static_cast<unsigned char>(addr & 0xFFu);
  buffer[5] = static_cast<unsigned char>((addr >> 8) & 0xFFu);
  buffer[6] = static_cast<unsigned char>((addr >> 16) & 0xFFu);
  buffer[7] = static_cast<unsigned char>((addr >> 24) & 0x7Fu);
  buffer[7] = static_cast<unsigned char>(buffer[7] | 0x80u); // read flag

  libusb_device_handle * devh = Handle();
  if (!devh) {
    ERROR("could not get device handle for the device");
    return -1;
  }

  int stat = 0;
  int transferred = 0;

  stat = libusb_bulk_transfer(devh, kUSB3_SF_WRITE, buffer, length, &transferred, timeout);
  if (stat < 0) {
    ERROR("write error:%s [sid=%d]", libusb_error_name(stat), _sid);
    return stat;
  }

  int nbulk = static_cast<int>(count / kUSB3_BULK_BLOCK);
  int remains = static_cast<int>(count % kUSB3_BULK_BLOCK);

  for (int loop = 0; loop < nbulk; ++loop) {
    stat = libusb_bulk_transfer(devh, kUSB3_SF_READ, data + loop * kUSB3_DATA_UNIT, kUSB3_DATA_UNIT, &transferred,
                                timeout);
    if (stat < 0) {
      ERROR("read error:%s [sid=%d]", libusb_error_name(stat), _sid);
      return stat;
    }
  }

  if (remains > 0) {
    stat =
        libusb_bulk_transfer(devh, kUSB3_SF_READ, data + nbulk * kUSB3_DATA_UNIT, remains * 4, &transferred, timeout);
    if (stat < 0) {
      ERROR("read error:%s [sid=%d]", libusb_error_name(stat), _sid);
      return stat;
    }
  }

  return 0;
}

int USB3Com::WriteControl(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char * data,
                          uint16_t wLength) const
{
  libusb_device_handle * devh = Handle();
  if (!devh) {
    ERROR("could not get device handle for the device");
    return -1;
  }

  int stat = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT, bRequest, wValue, wIndex,
                                     data, wLength, kUSB3_DEFAULT_TIMEOUT);

  if (stat < 0) { ERROR("could not make write request:%s [sid=%d]", libusb_error_name(stat), _sid); }

  return stat;
}

int USB3Com::ReadControl(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char * data,
                         uint16_t wLength) const
{
  libusb_device_handle * devh = Handle();
  if (!devh) {
    ERROR("could not get device handle for the device");
    return -1;
  }

  int stat = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, bRequest, wValue, wIndex,
                                     data, wLength, kUSB3_DEFAULT_TIMEOUT);

  if (stat < 0) { ERROR("could not make read request:%s [sid=%d]", libusb_error_name(stat), _sid); }

  return stat;
}

unsigned int USB3Com::ReadReg(uint32_t addr) const
{
  unsigned char data[4];

  int status = Read(1u, addr, data);
  if (status < 0) {
    ERROR("read error:%s [sid=%d]", libusb_error_name(status), _sid);
    return static_cast<unsigned int>(status);
  }

  return (static_cast<unsigned int>(data[0])) | (static_cast<unsigned int>(data[1]) << 8) |
         (static_cast<unsigned int>(data[2]) << 16) | (static_cast<unsigned int>(data[3]) << 24);
}

int USB3Com::ReadRegI(uint32_t addr) const { return static_cast<int>(ReadReg(addr)); }

unsigned long USB3Com::ReadRegL(uint32_t addr) const
{
  unsigned char data[8];

  int status = Read(2u, addr, data);
  if (status < 0) {
    ERROR("read error:%s [sid=%d]", libusb_error_name(status), _sid);
    return static_cast<unsigned long>(status);
  }

  return (static_cast<unsigned long>(data[0])) | (static_cast<unsigned long>(data[1]) << 8) |
         (static_cast<unsigned long>(data[2]) << 16) | (static_cast<unsigned long>(data[3]) << 24) |
         (static_cast<unsigned long>(data[4]) << 32) | (static_cast<unsigned long>(data[5]) << 40) |
         (static_cast<unsigned long>(data[6]) << 48) | (static_cast<unsigned long>(data[7]) << 56);
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
