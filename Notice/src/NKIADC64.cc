#include <algorithm> // for std::min
#include <vector>

#include "DAQUtils/ELog.hh"
#include "Notice/NKIADC64.hh"

namespace {
constexpr uint16_t kVENDOR_ID = 0x0547;
constexpr uint16_t kPRODUCT_ID = 0x2010;

// Register Addresses
constexpr uint32_t kAddr_BCOUNT = 0x30000000u;
constexpr uint32_t kAddr_DATA = 0x40000000u;

// Constants
constexpr uint32_t kBlockSize = 256u;
} // namespace

NKIADC64::NKIADC64()
  : _sid(0),
    _usb(kVENDOR_ID, kPRODUCT_ID, 0)
{
}

NKIADC64::NKIADC64(int sid)
  : _sid(sid),
    _usb(kVENDOR_ID, kPRODUCT_ID, sid)
{
}

NKIADC64::~NKIADC64() { Close(); }

void NKIADC64::SetSID(int sid)
{
  _sid = sid;
  _usb.Set(kVENDOR_ID, kPRODUCT_ID, _sid);
}

int NKIADC64::Open()
{
  int status = _usb.Open();
  if (status < 0) {
    ERROR("NKIADC64: Failed to open device (sid=%d)", _sid);
    return status;
  }

  status = _usb.ClaimInterface(0);
  if (status < 0) {
    ERROR("NKIADC64: Failed to claim interface 0 (sid=%d)", _sid);
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

uint32_t NKIADC64::ReadBCount() const { return _usb.ReadReg(kAddr_BCOUNT); }

int NKIADC64::ReadData(uint32_t bcount, unsigned char * data, unsigned int timeout) const
{
  if (bcount == 0 || data == nullptr) {
    ERROR("NKIADC64: Invalid ReadData arguments (bcount=%u, data=%p)", bcount,
          static_cast<void *>(data));
    return -1;
  }

  // Calculate total bytes: bcount * 256 bytes per block
  uint32_t total_bytes = bcount * kBlockSize;

  return _usb.Read(total_bytes, kAddr_DATA, data, timeout);
}

void NKIADC64::FlushData() const
{
  uint32_t bcount = ReadBCount();
  if (bcount == 0) return;

  // Define a chunk size for flushing (e.g., 2.6 MB buffer)
  // 10240 blocks * 256 bytes = 2,621,440 bytes
  constexpr uint32_t kChunkBlocks = 10240u;
  constexpr size_t kBufferSize = kChunkBlocks * kBlockSize;

  std::vector<unsigned char> buffer(kBufferSize);

  uint32_t remaining_blocks = bcount;

  while (remaining_blocks > 0) {
    uint32_t read_blocks = std::min(remaining_blocks, kChunkBlocks);

    int ret = ReadData(read_blocks, buffer.data());
    if (ret < 0) {
      ERROR("NKIADC64: FlushData failed while draining %u blocks (sid=%d)", read_blocks, _sid);
      break;
    }

    remaining_blocks -= read_blocks;
  }
}