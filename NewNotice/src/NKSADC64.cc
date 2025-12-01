#include "DAQUtils/ELog.hh"
#include "NewNotice/NKSADC64.hh"

namespace {
constexpr uint16_t kNKSADC64_VENDOR_ID = 0x0547;
constexpr uint16_t kNKSADC64_PRODUCT_ID = 0x1503;

constexpr uint32_t kNKSADC64_REG_BCOUNT = 0x30000000u;
constexpr uint32_t kNKSADC64_ADDR_DATA = 0x40000000u;
} // namespace

NKSADC64::NKSADC64(int sid)
  : _sid(sid),
    _usb(kNKSADC64_VENDOR_ID, kNKSADC64_PRODUCT_ID, sid)
{
}

NKSADC64::~NKSADC64() { Close(); }

int NKSADC64::Open()
{
  int status = _usb.Open();
  if (status < 0) {
    ERROR("failed to open device (sid=%d)", _sid);
    return status;
  }

  status = _usb.ClaimInterface(0);
  if (status < 0) {
    ERROR("failed to claim interface 0 (sid=%d)", _sid);
    _usb.Close();
    return status;
  }

  return 0;
}

void NKSADC64::Close()
{
  _usb.ReleaseInterface(0);
  _usb.Close();
}

int NKSADC64::ReadBCount() const { return _usb.ReadRegI(kNKSADC64_REG_BCOUNT); }

int NKSADC64::ReadData(int bcount, unsigned char * data, unsigned int timeout) const
{
  if (bcount <= 0  || data == nullptr) {
    ERROR("invalid arguments (bcount=%d, data=%p)", bcount, static_cast<void *>(data));
    return -1;
  }

  int count = bcount * 256;
  return _usb.Read(static_cast<uint32_t>(count), kNKSADC64_ADDR_DATA, data, timeout);
}
