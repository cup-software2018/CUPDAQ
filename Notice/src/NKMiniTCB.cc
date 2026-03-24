#include <arpa/inet.h>
#include <chrono>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

#include "DAQUtils/ELog.hh"
#include "Notice/NKMiniTCB.hh"

namespace {
constexpr int kPort = 5000;

// MiniTCB Command Codes
constexpr char kCmdWrite = 11;
constexpr char kCmdRead = 12;

// Indirect Module Access Registers
constexpr uint32_t kRegInd_Mid = 0x17;
constexpr uint32_t kRegInd_Addr0 = 0x18;
constexpr uint32_t kRegInd_Addr1 = 0x19;
constexpr uint32_t kRegInd_Addr2 = 0x1A;
constexpr uint32_t kRegInd_Addr3 = 0x1B;
constexpr uint32_t kRegInd_Data0 = 0x1C;
constexpr uint32_t kRegInd_Data1 = 0x1D;
constexpr uint32_t kRegInd_Data2 = 0x1E;
constexpr uint32_t kRegInd_Data3 = 0x1F;
constexpr uint32_t kRegInd_Write = 0x15;
constexpr uint32_t kRegInd_Read = 0x16;

// Helper for address calculation
inline uint32_t GetModuleRegAddr(uint32_t base, uint32_t ch)
{
  return base + (((ch - 1u) & 0xFFu) << 16);
}

inline uint32_t GetModuleRegAddrZB(uint32_t base, uint32_t ch)
{
  return base + ((ch & 0xFFu) << 16);
}

void SleepMs(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
} // namespace

NKMiniTCB::~NKMiniTCB() { Close(); }

int NKMiniTCB::Open()
{
  if (_ipaddr.empty()) {
    ERROR("NKMiniTCB: IP address is empty");
    return -1;
  }

  if (_tcpHandle >= 0) { Close(); }

  sockaddr_in serv_addr{};
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(_ipaddr.c_str());
  serv_addr.sin_port = htons(kPort);

  int handle = ::socket(AF_INET, SOCK_STREAM, 0);
  if (handle < 0) {
    ERROR("NKMiniTCB: can't open socket");
    return -1;
  }

  const int disable = 1;
  ::setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&disable),
               sizeof(disable));

  // Set receive timeout
  struct timeval tv;
  tv.tv_sec = 2; // 2 seconds timeout
  tv.tv_usec = 0;
  ::setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

  if (::connect(handle, reinterpret_cast<sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
    ERROR("NKMiniTCB: can't connect to server (ip=%s, port=%d)", _ipaddr.c_str(), kPort);
    ::close(handle);
    return -2;
  }

  _tcpHandle = handle;
  return 0;
}

void NKMiniTCB::Close()
{
  if (_tcpHandle >= 0) {
    ::close(_tcpHandle);
    _tcpHandle = -1;
  }
}

int NKMiniTCB::Transmit(const char * buf, int len) const
{
  int bytes_more = len;
  int bytes_xferd = 0;

  while (bytes_more > 0) {
    const char * idxPtr = buf + bytes_xferd;
    ssize_t result = ::write(_tcpHandle, idxPtr, static_cast<size_t>(bytes_more));

    if (result < 0) {
      ERROR("NKMiniTCB: transmit failed, remaining=%d", bytes_more);
      return -1;
    }
    bytes_xferd += static_cast<int>(result);
    bytes_more -= static_cast<int>(result);
  }

  return 0;
}

int NKMiniTCB::Receive(char * buf, int len) const
{
  if (buf == nullptr) return -1;

  int buf_count = 0;
  int space_left = len;

  while (space_left > 0) {
    int accum = 0;
    while (true) {
      char * idxPtr = buf + (buf_count + accum);
      int bytes_more = space_left;
      int chunk = (bytes_more > 2048) ? 2048 : bytes_more;

      ssize_t result = ::read(_tcpHandle, idxPtr, static_cast<size_t>(chunk));

      if (result < 0) {
        ERROR("NKMiniTCB: unable to receive data");
        return -1;
      }
      if (result == 0) {
        ERROR("NKMiniTCB: connection closed by server");
        return -1;
      }

      accum += static_cast<int>(result);
      if ((accum + buf_count) >= len) break;

      // Partial read loop condition (optional logic preserved)
      if (result < chunk) {
        // INFO("wanted %d got %d", bytes_more, result);
        return accum + buf_count;
      }
    }

    buf_count += accum;
    space_left -= accum;
  }

  return buf_count;
}

void NKMiniTCB::Write(uint32_t address, uint32_t data) const
{
  char tcpBuf[3];
  tcpBuf[0] = kCmdWrite;
  tcpBuf[1] = static_cast<char>(address & 0xFFu);
  tcpBuf[2] = static_cast<char>(data & 0xFFu);

  if (Transmit(tcpBuf, 3) < 0) return;

  char ack[1];
  Receive(ack, 1); // Wait for ACK
}

uint32_t NKMiniTCB::Read(uint32_t address) const
{
  char tcpBuf[2];
  tcpBuf[0] = kCmdRead;
  tcpBuf[1] = static_cast<char>(address & 0xFFu);

  if (Transmit(tcpBuf, 2) < 0) return 0xFFFFFFFF;

  char resp[1];
  if (Receive(resp, 1) < 0) return 0xFFFFFFFF;

  return static_cast<uint32_t>(resp[0] & 0xFFu);
}

void NKMiniTCB::WriteModule(uint32_t mid, uint32_t address, uint32_t data) const
{
  Write(kRegInd_Mid, mid);

  Write(kRegInd_Addr0, address & 0xFFu);
  Write(kRegInd_Addr1, (address >> 8) & 0xFFu);
  Write(kRegInd_Addr2, (address >> 16) & 0xFFu);
  Write(kRegInd_Addr3, (address >> 24) & 0xFFu);

  Write(kRegInd_Data0, data & 0xFFu);
  Write(kRegInd_Data1, (data >> 8) & 0xFFu);
  Write(kRegInd_Data2, (data >> 16) & 0xFFu);
  Write(kRegInd_Data3, (data >> 24) & 0xFFu);

  Write(kRegInd_Write, 0); // Trigger Write
}

uint32_t NKMiniTCB::ReadModule(uint32_t mid, uint32_t address) const
{
  Write(kRegInd_Mid, mid);

  Write(kRegInd_Addr0, address & 0xFFu);
  Write(kRegInd_Addr1, (address >> 8) & 0xFFu);
  Write(kRegInd_Addr2, (address >> 16) & 0xFFu);
  Write(kRegInd_Addr3, (address >> 24) & 0xFFu);

  Write(kRegInd_Read, 0); // Trigger Read

  uint32_t data = 0;
  data |= Read(kRegInd_Data0);
  data |= (Read(kRegInd_Data1) << 8);
  data |= (Read(kRegInd_Data2) << 16);
  data |= (Read(kRegInd_Data3) << 24);

  return data;
}

// -------------------------------------------------------------------------
// Control Wrappers
// -------------------------------------------------------------------------
void NKMiniTCB::Reset() { Write(0x00, 4); }
void NKMiniTCB::ResetTimer() { Write(0x00, 1); }
void NKMiniTCB::Start() { Write(0x00, 8); }
void NKMiniTCB::Stop() { Write(0x00, 0); }

uint32_t NKMiniTCB::ReadRUN(uint32_t mid) const
{
  if (mid) return ReadModule(mid, 0x20000000u);
  return Read(0x00);
}

// -------------------------------------------------------------------------
// Register Accessors
// -------------------------------------------------------------------------

void NKMiniTCB::WriteCW(uint32_t mid, uint32_t ch, uint32_t data) const
{
  if (mid > 0) WriteModule(mid, GetModuleRegAddr(0x20000001u, ch), data);
  else {
    Write(0x02, data & 0xFFu);
    Write(0x03, (data >> 8) & 0xFFu);
  }
}

uint32_t NKMiniTCB::ReadCW(uint32_t mid, uint32_t ch) const
{
  if (mid > 0) return ReadModule(mid, GetModuleRegAddr(0x20000001u, ch));

  uint32_t data = Read(0x02);
  data |= (Read(0x03) << 8);
  return data;
}

void NKMiniTCB::WriteGW(uint32_t mid, uint32_t ch, uint32_t data) const
{
  if (mid > 0) WriteModule(mid, GetModuleRegAddr(0x20000002u, ch), data);
  else {
    Write(0x02, data & 0xFFu);
    Write(0x03, (data >> 8) & 0xFFu);
  }
}

uint32_t NKMiniTCB::ReadGW(uint32_t mid, uint32_t ch) const
{
  if (mid > 0) return ReadModule(mid, GetModuleRegAddr(0x20000002u, ch));

  uint32_t data = Read(0x02);
  data |= (Read(0x03) << 8);
  return data;
}

void NKMiniTCB::WriteRL(uint32_t mid, uint32_t data) const { WriteModule(mid, 0x20000002u, data); }
uint32_t NKMiniTCB::ReadRL(uint32_t mid) const { return ReadModule(mid, 0x20000002u); }

void NKMiniTCB::WriteDRAMON(uint32_t mid, uint32_t data) const
{
  if (data) {
    uint32_t status = ReadModule(mid, 0x20000003u);
    if (status) WriteModule(mid, 0x20000003u, 0);
    WriteModule(mid, 0x20000003u, 1);

    // Timeout applied
    int timeout = 1000;
    while (ReadModule(mid, 0x20000003u) == 0 && timeout > 0) {
      timeout--;
    }
    if (timeout == 0) ERROR("NKMiniTCB: WriteDRAMON timeout mid=%u", mid);
  }
  else {
    WriteModule(mid, 0x20000003u, 0);
  }
}

uint32_t NKMiniTCB::ReadDRAMON(uint32_t mid) const { return ReadModule(mid, 0x20000003u); }

void NKMiniTCB::WriteDACOFF(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000004u, ch), data);
}
uint32_t NKMiniTCB::ReadDACOFF(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000004u, ch));
}

void NKMiniTCB::MeasurePED(uint32_t mid, uint32_t ch) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000005u, ch), 0);
}
uint32_t NKMiniTCB::ReadPED(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000006u, ch));
}

void NKMiniTCB::WriteDLY(uint32_t mid, uint32_t ch, uint32_t data) const
{
  uint32_t value = ((data / 1000u) << 10) | (data % 1000u);
  WriteModule(mid, GetModuleRegAddr(0x20000007u, ch), value);
}
uint32_t NKMiniTCB::ReadDLY(uint32_t mid, uint32_t ch) const
{
  uint32_t value = ReadModule(mid, GetModuleRegAddr(0x20000007u, ch));
  return (value >> 10) * 1000u + (value & 0x3FFu);
}

void NKMiniTCB::WriteAMOREDLY(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000007u, ch), data);
}
uint32_t NKMiniTCB::ReadAMOREDLY(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000007u, ch));
}

// Simple wrappers using helper
void NKMiniTCB::WriteTHR(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000008u, ch), data);
}
uint32_t NKMiniTCB::ReadTHR(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000008u, ch));
}

void NKMiniTCB::WritePOL(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000009u, ch), data);
}
uint32_t NKMiniTCB::ReadPOL(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000009u, ch));
}

void NKMiniTCB::WritePSW(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x2000000Au, ch), data);
}
uint32_t NKMiniTCB::ReadPSW(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x2000000Au, ch));
}

void NKMiniTCB::WriteAMODE(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x2000000Bu, ch), data);
}
uint32_t NKMiniTCB::ReadAMODE(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x2000000Bu, ch));
}

void NKMiniTCB::WritePCT(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x2000000Cu, ch), data);
}
uint32_t NKMiniTCB::ReadPCT(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x2000000Cu, ch));
}

void NKMiniTCB::WritePCI(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x2000000Du, ch), data);
}
uint32_t NKMiniTCB::ReadPCI(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x2000000Du, ch));
}

void NKMiniTCB::WritePWT(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x2000000Eu, ch), data);
}
uint32_t NKMiniTCB::ReadPWT(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x2000000Eu, ch));
}

void NKMiniTCB::WriteDT(uint32_t mid, uint32_t ch, uint32_t data) const
{
  if (mid > 0) WriteModule(mid, GetModuleRegAddr(0x2000000Fu, ch), data);
  else {
    Write(0x01, data & 0xFFu);
    Write(0x0B, (data >> 8) & 0xFFu);
    Write(0x0D, (data >> 16) & 0xFFu);
  }
}

uint32_t NKMiniTCB::ReadDT(uint32_t mid, uint32_t ch) const
{
  if (mid > 0) return ReadModule(mid, GetModuleRegAddr(0x2000000Fu, ch));

  uint32_t data = Read(0x01);
  data |= (Read(0x0B) << 8);
  data |= (Read(0x0D) << 16);
  return data;
}

void NKMiniTCB::WriteTM(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000014u, ch), data);
}
uint32_t NKMiniTCB::ReadTM(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000014u, ch));
}

void NKMiniTCB::WriteTLT(uint32_t mid, uint32_t data) const { WriteModule(mid, 0x20000015u, data); }
uint32_t NKMiniTCB::ReadTLT(uint32_t mid) const { return ReadModule(mid, 0x20000015u); }

void NKMiniTCB::WriteSTLT(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000015u, ch), data);
}
uint32_t NKMiniTCB::ReadSTLT(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000015u, ch));
}

void NKMiniTCB::WriteZEROSUP(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000016u, ch), data);
}
uint32_t NKMiniTCB::ReadZEROSUP(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000016u, ch));
}

void NKMiniTCB::SendADCRST(uint32_t mid) const { WriteModule(mid, 0x20000017u, 0); }
void NKMiniTCB::SendADCCAL(uint32_t mid) const { WriteModule(mid, 0x20000018u, 0); }

void NKMiniTCB::WriteADCDLY(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000019u, ch), data);
}
void NKMiniTCB::WriteADCALIGN(uint32_t mid, uint32_t data) const
{
  WriteModule(mid, 0x2000001Au, data);
}
uint32_t NKMiniTCB::ReadADCSTAT(uint32_t mid) const { return ReadModule(mid, 0x2000001Au); }

void NKMiniTCB::WriteBITSLIP(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x2000001Bu, ch), data);
}

void NKMiniTCB::WriteFMUX(uint32_t mid, uint32_t ch) const
{
  WriteModule(mid, 0x2000001Cu, ch - 1u);
}
uint32_t NKMiniTCB::ReadFMUX(uint32_t mid) const { return ReadModule(mid, 0x2000001Cu); }

void NKMiniTCB::ArmFADC(uint32_t mid) const { WriteModule(mid, 0x2000001Du, 0); }
uint32_t NKMiniTCB::ReadFREADY(uint32_t mid) const { return ReadModule(mid, 0x2000001Du); }

void NKMiniTCB::WriteZSFD(uint32_t mid, uint32_t data) const
{
  WriteModule(mid, 0x2000001Eu, data);
}
uint32_t NKMiniTCB::ReadZSFD(uint32_t mid) const { return ReadModule(mid, 0x2000001Eu); }

void NKMiniTCB::WriteDSR(uint32_t mid, uint32_t data) const { WriteModule(mid, 0x2000001Fu, data); }
uint32_t NKMiniTCB::ReadDSR(uint32_t mid) const { return ReadModule(mid, 0x2000001Fu); }

void NKMiniTCB::ReadFADCBUF(uint32_t mid, uint32_t * data) const
{
  uint32_t addr = 0x20008000u;
  for (uint32_t i = 0; i < 2048u; ++i) {
    data[i] = ReadModule(mid, addr + i);
  }
}

void NKMiniTCB::AlignFADC500(uint32_t mid) const
{
  SendADCRST(mid);
  SleepMs(500);
  SendADCCAL(mid);
  WriteADCALIGN(mid, 1);

  for (uint32_t ch = 1; ch <= 4; ++ch) {
    int count = 0;
    int sum = 0;
    bool flag = false;

    for (uint32_t dly = 0; dly < 32; ++dly) {
      WriteADCDLY(mid, ch, dly);
      uint32_t value = (ReadADCSTAT(mid) >> (ch - 1)) & 0x1u;

      if (!value) {
        flag = true;
        count++;
        sum += dly;
      }
      else if (flag) {
        break;
      }
    }

    uint32_t gdly = count ? (sum / count) : 0;
    // Adjustment logic preserved from original
    gdly = (gdly < 11) ? (gdly + 11) : (gdly - 11);

    WriteADCDLY(mid, ch, gdly);
    INFO("NKMiniTCB: ch%u calibration delay = %u", ch, gdly);
  }

  WriteADCALIGN(mid, 0);
  SendADCCAL(mid);
}

void NKMiniTCB::AlignSADC64(uint32_t mid) const
{
  SendADCRST(mid);
  SleepMs(500);
  SendADCCAL(mid);

  for (uint32_t ch = 1; ch <= 4; ++ch) {
    int count = 0;
    int sum = 0;
    bool flag = false;

    // Initialization Sequence
    WriteADCALIGN(mid, 0x030002);
    SleepMs(1);
    WriteADCALIGN(mid, 0x010010);
    SleepMs(1);
    WriteADCALIGN(mid, 0xC78001);
    SleepMs(1);
    WriteADCALIGN(mid, 0xDE01C0);
    SleepMs(1);

    WriteADCALIGN(mid, 0x450001);
    WriteBITSLIP(mid, ch, 0);

    for (uint32_t dly = 0; dly < 32; ++dly) {
      WriteADCDLY(mid, ch, dly);
      uint32_t value = (ReadADCSTAT(mid) >> (ch - 1)) & 0x1u;

      if (!value) {
        flag = true;
        count++;
        sum += dly;
      }
      else if (flag) {
        break;
      }
    }

    uint32_t gdly = count ? (sum / count) : 0;
    gdly = (gdly < 9) ? (gdly + 9) : (gdly - 9);
    WriteADCDLY(mid, ch, gdly);

    WriteADCALIGN(mid, 0x450002);
    SleepMs(1);

    uint32_t gbitslip = 0;
    for (uint32_t bitslip = 0; bitslip < 12; ++bitslip) {
      WriteBITSLIP(mid, ch, bitslip);
      uint32_t value = (ReadADCSTAT(mid) >> (ch + 3)) & 0x1u; // (ch-1)+4
      if (value) {
        gbitslip = bitslip;
        break;
      }
    }

    WriteBITSLIP(mid, ch, gbitslip);
    INFO("NKMiniTCB: ch%u aligned, delay=%u, bitslip=%u", ch, gdly, gbitslip);
  }

  WriteADCALIGN(mid, 0x450000);
  SleepMs(1);
  SendADCCAL(mid);
}

// ... Simple wrappers ...
void NKMiniTCB::WritePSS(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000010u, ch), data);
}
uint32_t NKMiniTCB::ReadPSS(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000010u, ch));
}

void NKMiniTCB::WriteRT(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000011u, ch), data);
}
uint32_t NKMiniTCB::ReadRT(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000011u, ch));
}

void NKMiniTCB::WriteSR(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000012u, ch), data);
}
uint32_t NKMiniTCB::ReadSR(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000012u, ch));
}

void NKMiniTCB::WriteDACGAIN(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000013u, ch), data);
}
uint32_t NKMiniTCB::ReadDACGAIN(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000013u, ch));
}

void NKMiniTCB::WriteST(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000020u, ch), data);
}
uint32_t NKMiniTCB::ReadST(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000020u, ch));
}

void NKMiniTCB::WritePT(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddr(0x20000021u, ch), data);
}
uint32_t NKMiniTCB::ReadPT(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddr(0x20000021u, ch));
}

void NKMiniTCB::WriteRUNNO(uint32_t data) const
{
  Write(0x04, data & 0xFFu);
  Write(0x05, (data >> 8) & 0xFFu);
}

uint32_t NKMiniTCB::ReadRUNNO() const
{
  uint32_t data = Read(0x04);
  data |= (Read(0x05) << 8);
  return data;
}

void NKMiniTCB::SendTRIG() const { Write(0x06, 0); }

uint32_t NKMiniTCB::ReadLNSTAT() const { return Read(0x10); }

uint32_t NKMiniTCB::ReadMIDS(uint32_t ch) const { return Read(0x10 + ch); }

void NKMiniTCB::WritePTRIG(uint32_t data) const
{
  Write(0x08, data & 0xFFu);
  Write(0x09, (data >> 8) & 0xFFu);
}

uint32_t NKMiniTCB::ReadPTRIG() const
{
  uint32_t data = Read(0x08);
  data |= (Read(0x09) << 8);
  return data;
}

void NKMiniTCB::WriteTRIGENABLE(uint32_t mid, uint32_t data) const
{
  if (mid) WriteModule(mid, 0x20000030u, data);
  else Write(0x0A, data & 0xFFu);
}

uint32_t NKMiniTCB::ReadTRIGENABLE(uint32_t mid) const
{
  if (mid) return ReadModule(mid, 0x20000030u);
  return Read(0x0A);
}

void NKMiniTCB::WriteMTHR(uint32_t data) const { Write(0x0C, data & 0xFFu); }
uint32_t NKMiniTCB::ReadMTHR() const { return Read(0x0C); }

void NKMiniTCB::WritePSCALE(uint32_t data) const
{
  Write(0x0E, data & 0xFFu);
  Write(0x0F, (data >> 8) & 0xFFu);
}

uint32_t NKMiniTCB::ReadPSCALE() const
{
  uint32_t data = Read(0x0E);
  data |= (Read(0x0F) << 8);
  return data;
}

void NKMiniTCB::WriteDRAMDLY(uint32_t mid, uint32_t ch, uint32_t data) const
{
  WriteModule(mid, GetModuleRegAddrZB(0x20000022u, ch), data);
}
void NKMiniTCB::WriteDRAMBITSLIP(uint32_t mid, uint32_t ch) const
{
  WriteModule(mid, GetModuleRegAddrZB(0x20000023u, ch), 0);
}
void NKMiniTCB::WriteDRAMTEST(uint32_t mid, uint32_t data) const
{
  WriteModule(mid, 0x20000024u, data);
}
uint32_t NKMiniTCB::ReadDRAMTEST(uint32_t mid, uint32_t ch) const
{
  return ReadModule(mid, GetModuleRegAddrZB(0x20000024u, ch));
}

void NKMiniTCB::AlignDRAM(uint32_t mid) const
{
  WriteDRAMON(mid, 1);
  WriteDRAMTEST(mid, 1);
  SendADCCAL(mid);
  WriteDRAMTEST(mid, 2);

  for (uint32_t ch = 0; ch < 8; ++ch) {
    int count = 0;
    int sum = 0;
    bool flag = false;

    // Scan Delay
    for (uint32_t dly = 0; dly < 32; ++dly) {
      WriteDRAMDLY(mid, ch, dly);
      WriteDRAMTEST(mid, 3);
      uint32_t value = ReadDRAMTEST(mid, ch);

      bool pattern_ok = (value == 0xFFAA5500u || value == 0xAA5500FFu || value == 0x5500FFAAu ||
                         value == 0x00FFAA55u);

      if (pattern_ok) {
        flag = true;
        count++;
        sum += dly;
      }
      else if (flag) {
        break;
      }
    }

    uint32_t gdly = count ? (sum / count) : 9;
    WriteDRAMDLY(mid, ch, gdly);

    // Scan Bitslip
    bool aligned = false;
    for (uint32_t bitslip = 0; bitslip < 4; ++bitslip) {
      WriteDRAMTEST(mid, 3);
      uint32_t value = ReadDRAMTEST(mid, ch);

      if (value == 0xFFAA5500u) {
        aligned = true;
        break;
      }
      else {
        WriteDRAMBITSLIP(mid, ch);
      }
    }

    if (aligned) { INFO("NKMiniTCB: DRAM(%u) aligned, delay=%u", ch, gdly); }
    else {
      ERROR("NKMiniTCB: DRAM(%u) alignment FAILED", ch);
    }
  }

  WriteDRAMTEST(mid, 0);
}