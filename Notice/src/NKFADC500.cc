#include "DAQUtils/ELog.hh"
#include "Notice/NKFADC500.hh"

namespace {
constexpr uint16_t kNKFADC500_VENDOR_ID = 0x0547;
constexpr uint16_t kNKFADC500_PRODUCT_ID = 0x1502;

constexpr uint32_t kNKFADC500_REG_BCOUNT = 0x30000000u;
constexpr uint32_t kNKFADC500_ADDR_DATA = 0x40000000u;
} // namespace

NKFADC500::NKFADC500() {}

NKFADC500::NKFADC500(int sid)
  : _sid(sid),
    _usb(kNKFADC500_VENDOR_ID, kNKFADC500_PRODUCT_ID, sid)
{
}

NKFADC500::~NKFADC500() { Close(); }


void NKFADC500::SetSID(int sid)
{
  _sid = sid;
  _usb.Set(kNKFADC500_VENDOR_ID, kNKFADC500_PRODUCT_ID, _sid);
}

int NKFADC500::Open()
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

void NKFADC500::Close()
{
  _usb.ReleaseInterface(0);
  _usb.Close();
}

int NKFADC500::ReadBCount() const { return _usb.ReadRegI(kNKFADC500_REG_BCOUNT); }

int NKFADC500::ReadData(int bcount, unsigned char * data, unsigned int timeout) const
{
  if (bcount <= 0 || data == nullptr) {
    ERROR("invalid arguments (bcount=%d, data=%p)", bcount, static_cast<void *>(data));
    return -1;
  }

  int count = bcount * 256;
  return _usb.Read(static_cast<uint32_t>(count), kNKFADC500_ADDR_DATA, data, timeout);
}
