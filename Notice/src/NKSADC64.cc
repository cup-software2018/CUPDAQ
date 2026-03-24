#include "DAQUtils/ELog.hh"
#include "Notice/NKSADC64.hh"

namespace {
constexpr uint16_t kVENDOR_ID = 0x0547;
constexpr uint16_t kPRODUCT_ID = 0x1503;

// Register Addresses
constexpr uint32_t kAddr_BCOUNT = 0x30000000u;
constexpr uint32_t kAddr_DATA = 0x40000000u;

constexpr uint32_t kBlockSize = 256u;
} // namespace

NKSADC64::NKSADC64()
  : _sid(0),
    _usb(kVENDOR_ID, kPRODUCT_ID, 0)
{
}

NKSADC64::NKSADC64(int sid)
  : _sid(sid),
    _usb(kVENDOR_ID, kPRODUCT_ID, sid)
{
}

NKSADC64::~NKSADC64() { Close(); }

void NKSADC64::SetSID(int sid)
{
  _sid = sid;
  _usb.Set(kVENDOR_ID, kPRODUCT_ID, _sid);
}

int NKSADC64::Open()
{
  int status = _usb.Open();
  if (status < 0) {
    ERROR("NKSADC64: Failed to open device (sid=%d)", _sid);
    return status;
  }

  status = _usb.ClaimInterface(0);
  if (status < 0) {
    ERROR("NKSADC64: Failed to claim interface 0 (sid=%d)", _sid);
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

uint32_t NKSADC64::ReadBCount() const { return _usb.ReadReg(kAddr_BCOUNT); }

int NKSADC64::ReadData(uint32_t bcount, unsigned char * data, unsigned int timeout) const
{
  if (bcount == 0 || data == nullptr) {
    ERROR("NKSADC64: Invalid ReadData arguments (bcount=%u, data=%p)", bcount,
          static_cast<void *>(data));
    return -1;
  }

  // Calculate total bytes: bcount * 256 bytes per block
  uint32_t total_bytes = bcount * kBlockSize;

  return _usb.Read(total_bytes, kAddr_DATA, data, timeout);
}