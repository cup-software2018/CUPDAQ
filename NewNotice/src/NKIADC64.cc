#include <vector>

#include "DAQUtils/ELog.hh"
#include "NewNotice/NKIADC64.hh"

namespace {
constexpr uint16_t kNKIADC64_VENDOR_ID = 0x0547;
constexpr uint16_t kNKIADC64_PRODUCT_ID = 0x2010;

constexpr uint32_t kNKIADC64_REG_BCOUNT = 0x30000000u;
constexpr uint32_t kNKIADC64_ADDR_DATA = 0x40000000u;

constexpr unsigned long kNKIADC64_FLUSH_MAX_BYTES = 10485760ul;
constexpr unsigned long kNKIADC64_FLUSH_CHUNK_BCOUNT = 10240ul;
} // namespace

NKIADC64::NKIADC64() {}

NKIADC64::NKIADC64(int sid)
  : _sid(sid),
    _usb(kNKIADC64_VENDOR_ID, kNKIADC64_PRODUCT_ID, sid)
{
}

NKIADC64::~NKIADC64() { Close(); }

void NKIADC64::SetSID(int sid)
{
  _sid = sid;
  _usb.Set(kNKIADC64_VENDOR_ID, kNKIADC64_PRODUCT_ID, _sid);
}

int NKIADC64::Open()
{
  int status = _usb.Open();
  if (status < 0) {
    ERROR("NKIADC64: failed to open device (sid=%d)", _sid);
    return status;
  }

  status = _usb.ClaimInterface(0);
  if (status < 0) {
    ERROR("NKIADC64: failed to claim interface 0 (sid=%d)", _sid);
    _usb.Close();
    return status;
  }

  return 0;
}

void NKIADC64::Close()
{
  _usb.ReleaseInterface(0);
  _usb.Close();
}

int NKIADC64::ReadBCount() const { return _usb.ReadRegI(kNKIADC64_REG_BCOUNT); }

int NKIADC64::ReadData(int bcount, unsigned char * data, unsigned int timeout) const
{
  if (bcount <= 0 || data == nullptr) {
    ERROR("NKIADC64: invalid ReadData arguments (bcount=%d, data=%p)", bcount, static_cast<void *>(data));
    return -1;
  }

  int count = bcount * 256;
  return _usb.Read(static_cast<uint32_t>(count), kNKIADC64_ADDR_DATA, data, timeout);
}

void NKIADC64::FlushData() const
{
  int bcountInt = ReadBCount();
  if (bcountInt <= 0) { return; }

  unsigned long bcount = static_cast<unsigned long>(bcountInt);

  std::vector<unsigned char> buffer(kNKIADC64_FLUSH_MAX_BYTES);

  unsigned long chunk = bcount / kNKIADC64_FLUSH_CHUNK_BCOUNT;
  unsigned long slice = bcount % kNKIADC64_FLUSH_CHUNK_BCOUNT;

  for (unsigned long i = 0; i < chunk; ++i) {
    int ret = ReadData(static_cast<int>(kNKIADC64_FLUSH_CHUNK_BCOUNT), buffer.data());
    if (ret < 0) {
      ERROR("NKIADC64: FlushData chunk read failed at i=%lu (sid=%d)", i, _sid);
      return;
    }
  }

  if (slice > 0ul) {
    int ret = ReadData(static_cast<int>(slice), buffer.data());
    if (ret < 0) { ERROR("NKIADC64: FlushData slice read failed (sid=%d)", _sid); }
  }
}
