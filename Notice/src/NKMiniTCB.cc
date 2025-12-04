#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "DAQUtils/ELog.hh"
#include "Notice/NKMiniTCB.hh"

NKMiniTCB::~NKMiniTCB() { Close(); }

int NKMiniTCB::Open()
{
  if (_ipaddr.empty()) {
    ERROR("IP address is empty");
    return -1;
  }

  if (_tcpHandle >= 0) { Close(); }

  sockaddr_in serv_addr{};
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(_ipaddr.c_str());
  serv_addr.sin_port = htons(5000);

  int handle = ::socket(AF_INET, SOCK_STREAM, 0);
  if (handle < 0) {
    ERROR("can't open socket");
    return -1;
  }

  const int disable = 1;
  ::setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char *>(&disable), sizeof(disable));

  if (::connect(handle, reinterpret_cast<sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0) {
    ERROR("can't connect to server (ip=%s, port=5000)", _ipaddr.c_str());
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

int NKMiniTCB::Transmit(char * buf, int len) const
{
  int bytes_more = len;
  int bytes_xferd = 0;

  while (bytes_more > 0) {
    char * idxPtr = buf + bytes_xferd;
    int result = static_cast<int>(::write(_tcpHandle, idxPtr, static_cast<size_t>(bytes_more)));
    if (result < 0) {
      ERROR("transmit failed, remaining=%d", bytes_more);
      return -1;
    }
    bytes_xferd += result;
    bytes_more -= result;
  }

  return 0;
}

int NKMiniTCB::Receive(char * buf, int len) const
{
  if (buf == nullptr) { return -1; }

  int buf_count = 0;
  int space_left = len;

  while (space_left > 0) {
    int accum = 0;
    while (true) {
      char * idxPtr = buf + (buf_count + accum);
      int bytes_more = space_left;
      int chunk = (bytes_more > 2048) ? 2048 : bytes_more;

      int result = static_cast<int>(::read(_tcpHandle, idxPtr, static_cast<size_t>(chunk)));
      if (result < 0) {
        ERROR("unable to receive data from the server");
        return -1;
      }

      accum += result;
      if ((accum + buf_count) >= len) { break; }

      if (result < chunk) {
        INFO("wanted %d got %d", bytes_more, result);
        return accum + buf_count;
      }
    }

    buf_count += accum;
    space_left -= accum;
  }

  return buf_count;
}

void NKMiniTCB::Write(unsigned long address, unsigned long data) const
{
  char tcpBuf[3];
  tcpBuf[0] = 11;
  tcpBuf[1] = static_cast<char>(address & 0xFFu);
  tcpBuf[2] = static_cast<char>(data & 0xFFu);

  Transmit(tcpBuf, 3);
  char ack[1];
  Receive(ack, 1);
}

unsigned long NKMiniTCB::Read(unsigned long address) const
{
  char tcpBuf[2];
  tcpBuf[0] = 12;
  tcpBuf[1] = static_cast<char>(address & 0xFFu);

  Transmit(tcpBuf, 2);
  char resp[1];
  Receive(resp, 1);

  unsigned long data = static_cast<unsigned long>(resp[0] & 0xFF);
  return data;
}

void NKMiniTCB::WriteModule(unsigned long mid, unsigned long address, unsigned long data) const
{
  Write(0x17, mid);

  Write(0x18, address & 0xFFu);
  Write(0x19, (address >> 8) & 0xFFu);
  Write(0x1A, (address >> 16) & 0xFFu);
  Write(0x1B, (address >> 24) & 0xFFu);

  Write(0x1C, data & 0xFFu);
  Write(0x1D, (data >> 8) & 0xFFu);
  Write(0x1E, (data >> 16) & 0xFFu);
  Write(0x1F, (data >> 24) & 0xFFu);

  Write(0x15, 0);
}

unsigned long NKMiniTCB::ReadModule(unsigned long mid, unsigned long address) const
{
  Write(0x17, mid);

  Write(0x18, address & 0xFFu);
  Write(0x19, (address >> 8) & 0xFFu);
  Write(0x1A, (address >> 16) & 0xFFu);
  Write(0x1B, (address >> 24) & 0xFFu);

  Write(0x16, 0);

  unsigned long data = Read(0x1C);
  unsigned long value = Read(0x1D);
  data += (value << 8);
  value = Read(0x1E);
  data += (value << 16);
  value = Read(0x1F);
  data += (value << 24);

  return data;
}

void NKMiniTCB::Reset() { Write(0x00, 4); }

void NKMiniTCB::ResetTimer() { Write(0x00, 1); }

void NKMiniTCB::Start() { Write(0x00, 8); }

void NKMiniTCB::Stop() { Write(0x00, 0); }

unsigned long NKMiniTCB::ReadRUN(unsigned long mid) const
{
  if (mid) { return ReadModule(mid, 0x20000000); }
  return Read(0x00);
}

void NKMiniTCB::WriteCW(unsigned long mid, unsigned long ch, unsigned long data) const
{
  if (mid > 0) {
    unsigned long addr = 0x20000001 + (((ch - 1) & 0xFFu) << 16);
    WriteModule(mid, addr, data);
  }
  else {
    Write(0x02, data & 0xFFu);
    Write(0x03, (data >> 8) & 0xFFu);
  }
}

unsigned long NKMiniTCB::ReadCW(unsigned long mid, unsigned long ch) const
{
  if (mid > 0) {
    unsigned long addr = 0x20000001 + (((ch - 1) & 0xFFu) << 16);
    return ReadModule(mid, addr);
  }

  unsigned long data = Read(0x02);
  unsigned long temp = Read(0x03);
  data += (temp << 8);
  return data;
}

void NKMiniTCB::WriteGW(unsigned long mid, unsigned long ch, unsigned long data) const
{
  if (mid > 0) {
    unsigned long addr = 0x20000002 + (((ch - 1) & 0xFFu) << 16);
    WriteModule(mid, addr, data);
  }
  else {
    Write(0x02, data & 0xFFu);
    Write(0x03, (data >> 8) & 0xFFu);
  }
}

unsigned long NKMiniTCB::ReadGW(unsigned long mid, unsigned long ch) const
{
  if (mid > 0) {
    unsigned long addr = 0x20000002 + (((ch - 1) & 0xFFu) << 16);
    return ReadModule(mid, addr);
  }

  unsigned long data = Read(0x02);
  unsigned long temp = Read(0x03);
  data += (temp << 8);
  return data;
}

void NKMiniTCB::WriteRL(unsigned long mid, unsigned long data) const { WriteModule(mid, 0x20000002, data); }

unsigned long NKMiniTCB::ReadRL(unsigned long mid) const { return ReadModule(mid, 0x20000002); }

void NKMiniTCB::WriteDRAMON(unsigned long mid, unsigned long data) const
{
  if (data) {
    unsigned long status = ReadModule(mid, 0x20000003);
    if (status) { WriteModule(mid, 0x20000003, 0); }
    WriteModule(mid, 0x20000003, 1);
    status = 0;
    while (!status) {
      status = ReadModule(mid, 0x20000003);
    }
  }
  else {
    WriteModule(mid, 0x20000003, 0);
  }
}

unsigned long NKMiniTCB::ReadDRAMON(unsigned long mid) const { return ReadModule(mid, 0x20000003); }

void NKMiniTCB::WriteDACOFF(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000004 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadDACOFF(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000004 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::MeasurePED(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000005 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, 0);
}

unsigned long NKMiniTCB::ReadPED(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000006 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WriteDLY(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000007 + (((ch - 1) & 0xFFu) << 16);
  unsigned long value = ((data / 1000) << 10) | (data % 1000);
  WriteModule(mid, addr, value);
}

unsigned long NKMiniTCB::ReadDLY(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000007 + (((ch - 1) & 0xFFu) << 16);
  unsigned long value = ReadModule(mid, addr);
  unsigned long data = (value >> 10) * 1000 + (value & 0x3FFu);
  return data;
}

void NKMiniTCB::WriteAMOREDLY(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000007 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadAMOREDLY(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000007 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WriteTHR(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000008 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadTHR(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000008 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WritePOL(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000009 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadPOL(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000009 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WritePSW(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x2000000A + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadPSW(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x2000000A + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WriteAMODE(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x2000000B + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadAMODE(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x2000000B + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WritePCT(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x2000000C + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadPCT(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x2000000C + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WritePCI(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x2000000D + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadPCI(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x2000000D + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WritePWT(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x2000000E + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadPWT(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x2000000E + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WriteDT(unsigned long mid, unsigned long ch, unsigned long data) const
{
  if (mid > 0) {
    unsigned long addr = 0x2000000F + (((ch - 1) & 0xFFu) << 16);
    WriteModule(mid, addr, data);
  }
  else {
    Write(0x01, data & 0xFFu);
    Write(0x0B, (data >> 8) & 0xFFu);
    Write(0x0D, (data >> 16) & 0xFFu);
  }
}

unsigned long NKMiniTCB::ReadDT(unsigned long mid, unsigned long ch) const
{
  if (mid > 0) {
    unsigned long addr = 0x2000000F + (((ch - 1) & 0xFFu) << 16);
    return ReadModule(mid, addr);
  }

  unsigned long data = Read(0x01);
  unsigned long temp = Read(0x0B);
  data += (temp << 8);
  temp = Read(0x0D);
  data += (temp << 16);
  return data;
}

void NKMiniTCB::WriteTM(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000014 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadTM(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000014 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WriteTLT(unsigned long mid, unsigned long data) const { WriteModule(mid, 0x20000015, data); }

unsigned long NKMiniTCB::ReadTLT(unsigned long mid) const { return ReadModule(mid, 0x20000015); }

void NKMiniTCB::WriteSTLT(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000015 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadSTLT(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000015 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WriteZEROSUP(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000016 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadZEROSUP(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000016 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::SendADCRST(unsigned long mid) const { WriteModule(mid, 0x20000017, 0); }

void NKMiniTCB::SendADCCAL(unsigned long mid) const { WriteModule(mid, 0x20000018, 0); }

void NKMiniTCB::WriteADCDLY(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000019 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

void NKMiniTCB::WriteADCALIGN(unsigned long mid, unsigned long data) const { WriteModule(mid, 0x2000001A, data); }

unsigned long NKMiniTCB::ReadADCSTAT(unsigned long mid) const { return ReadModule(mid, 0x2000001A); }

void NKMiniTCB::WriteBITSLIP(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x2000001B + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

void NKMiniTCB::WriteFMUX(unsigned long mid, unsigned long ch) const { WriteModule(mid, 0x2000001C, ch - 1); }

unsigned long NKMiniTCB::ReadFMUX(unsigned long mid) const { return ReadModule(mid, 0x2000001C); }

void NKMiniTCB::ArmFADC(unsigned long mid) const { WriteModule(mid, 0x2000001D, 0); }

unsigned long NKMiniTCB::ReadFREADY(unsigned long mid) const { return ReadModule(mid, 0x2000001D); }

void NKMiniTCB::WriteZSFD(unsigned long mid, unsigned long data) const { WriteModule(mid, 0x2000001E, data); }

unsigned long NKMiniTCB::ReadZSFD(unsigned long mid) const { return ReadModule(mid, 0x2000001E); }

void NKMiniTCB::WriteDSR(unsigned long mid, unsigned long data) const { WriteModule(mid, 0x2000001F, data); }

unsigned long NKMiniTCB::ReadDSR(unsigned long mid) const { return ReadModule(mid, 0x2000001F); }

void NKMiniTCB::ReadFADCBUF(unsigned long mid, unsigned long * data) const
{
  unsigned long addr = 0x20008000;
  for (unsigned long i = 0; i < 2048; i++) {
    data[i] = ReadModule(mid, addr + i);
  }
}

void NKMiniTCB::AlignFADC500(unsigned long mid) const
{
  SendADCRST(mid);
  usleep(500000);
  SendADCCAL(mid);
  WriteADCALIGN(mid, 1);

  for (unsigned long ch = 1; ch <= 4; ch++) {
    int count = 0;
    int sum = 0;
    int flag = 0;

    for (unsigned long dly = 0; dly < 32; dly++) {
      WriteADCDLY(mid, ch, dly);
      unsigned long value = (ReadADCSTAT(mid) >> (ch - 1)) & 0x1u;

      if (!value) {
        flag = 1;
        count += 1;
        sum += static_cast<int>(dly);
      }
      else {
        if (flag) { dly = 32; }
      }
    }

    int center = (count != 0) ? (sum / count) : 0;
    unsigned long gdly =
        (center < 11) ? static_cast<unsigned long>(center + 11) : static_cast<unsigned long>(center - 11);

    WriteADCDLY(mid, ch, gdly);
    INFO("ch%lu calibration delay = %lu", ch, gdly);
  }

  WriteADCALIGN(mid, 0);
  SendADCCAL(mid);
}

void NKMiniTCB::AlignSADC64(unsigned long mid) const
{
  SendADCRST(mid);
  usleep(500000);
  SendADCCAL(mid);

  for (unsigned long ch = 1; ch <= 4; ch++) {
    int count = 0;
    int sum = 0;
    int flag = 0;
    unsigned long gbitslip = 0;

    WriteADCALIGN(mid, 0x030002);
    usleep(100);
    WriteADCALIGN(mid, 0x010010);
    usleep(100);
    WriteADCALIGN(mid, 0xC78001);
    usleep(100);
    WriteADCALIGN(mid, 0xDE01C0);
    usleep(100);

    WriteADCALIGN(mid, 0x450001);
    WriteBITSLIP(mid, ch, 0);

    for (unsigned long dly = 0; dly < 32; dly++) {
      WriteADCDLY(mid, ch, dly);
      unsigned long value = (ReadADCSTAT(mid) >> (ch - 1)) & 0x1u;

      if (!value) {
        flag = 1;
        count += 1;
        sum += static_cast<int>(dly);
      }
      else {
        if (flag) { dly = 32; }
      }
    }

    int center = (count != 0) ? (sum / count) : 0;
    unsigned long gdly = (center < 9) ? static_cast<unsigned long>(center + 9) : static_cast<unsigned long>(center - 9);

    WriteADCDLY(mid, ch, gdly);

    WriteADCALIGN(mid, 0x450002);
    usleep(100);

    for (unsigned long bitslip = 0; bitslip < 12; bitslip++) {
      WriteBITSLIP(mid, ch, bitslip);
      unsigned long value = (ReadADCSTAT(mid) >> ((ch - 1) + 4)) & 0x1u;
      if (value) {
        gbitslip = bitslip;
        bitslip = 12;
      }
    }

    WriteBITSLIP(mid, ch, gbitslip);
    INFO("ch%lu calibration delay = %lu, bitslip = %lu", ch, gdly, gbitslip);
  }

  WriteADCALIGN(mid, 0x450000);
  usleep(100);
  SendADCCAL(mid);
}

void NKMiniTCB::WritePSS(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000010 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadPSS(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000010 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WriteRT(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000011 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadRT(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000011 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WriteSR(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000012 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadSR(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000012 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WriteDACGAIN(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000013 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadDACGAIN(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000013 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WriteST(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000020 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadST(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000020 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WritePT(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000021 + (((ch - 1) & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

unsigned long NKMiniTCB::ReadPT(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000021 + (((ch - 1) & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::WriteRUNNO(unsigned long data) const
{
  Write(0x04, data & 0xFFu);
  Write(0x05, (data >> 8) & 0xFFu);
}

unsigned long NKMiniTCB::ReadRUNNO() const
{
  unsigned long data = Read(0x04);
  unsigned long temp = Read(0x05);
  data += (temp << 8);
  return data;
}

void NKMiniTCB::SendTRIG() const { Write(0x06, 0); }

unsigned long NKMiniTCB::ReadLNSTAT() const { return Read(0x10); }

unsigned long NKMiniTCB::ReadMIDS(unsigned long ch) const
{
  unsigned long addr = 0x10 + ch;
  return Read(addr);
}

void NKMiniTCB::WritePTRIG(unsigned long data) const
{
  Write(0x08, data & 0xFFu);
  Write(0x09, (data >> 8) & 0xFFu);
}

unsigned long NKMiniTCB::ReadPTRIG() const
{
  unsigned long data = Read(0x08);
  unsigned long temp = Read(0x09);
  data += (temp << 8);
  return data;
}

void NKMiniTCB::WriteTRIGENABLE(unsigned long mid, unsigned long data) const
{
  if (mid) { WriteModule(mid, 0x20000030, data); }
  else {
    Write(0x0A, data & 0xFFu);
  }
}

unsigned long NKMiniTCB::ReadTRIGENABLE(unsigned long mid) const
{
  if (mid) { return ReadModule(mid, 0x20000030); }
  return Read(0x0A);
}

void NKMiniTCB::WriteMTHR(unsigned long data) const { Write(0x0C, data & 0xFFu); }

unsigned long NKMiniTCB::ReadMTHR() const { return Read(0x0C); }

void NKMiniTCB::WritePSCALE(unsigned long data) const
{
  Write(0x0E, data & 0xFFu);
  Write(0x0F, (data >> 8) & 0xFFu);
}

unsigned long NKMiniTCB::ReadPSCALE() const
{
  unsigned long data = Read(0x0E);
  unsigned long temp = Read(0x0F);
  data += (temp << 8);
  return data;
}

void NKMiniTCB::WriteDRAMDLY(unsigned long mid, unsigned long ch, unsigned long data) const
{
  unsigned long addr = 0x20000022 + ((ch & 0xFFu) << 16);
  WriteModule(mid, addr, data);
}

void NKMiniTCB::WriteDRAMBITSLIP(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000023 + ((ch & 0xFFu) << 16);
  WriteModule(mid, addr, 0);
}

void NKMiniTCB::WriteDRAMTEST(unsigned long mid, unsigned long data) const { WriteModule(mid, 0x20000024, data); }

unsigned long NKMiniTCB::ReadDRAMTEST(unsigned long mid, unsigned long ch) const
{
  unsigned long addr = 0x20000024 + ((ch & 0xFFu) << 16);
  return ReadModule(mid, addr);
}

void NKMiniTCB::AlignDRAM(unsigned long mid) const
{
  WriteDRAMON(mid, 1);
  WriteDRAMTEST(mid, 1);
  SendADCCAL(mid);
  WriteDRAMTEST(mid, 2);

  for (unsigned long ch = 0; ch < 8; ch++) {
    int count = 0;
    int sum = 0;
    int flag = 0;
    int aflag = 0;
    unsigned long gdly = 0;

    for (unsigned long dly = 0; dly < 32; dly++) {
      WriteDRAMDLY(mid, ch, dly);
      WriteDRAMTEST(mid, 3);
      unsigned long value = ReadDRAMTEST(mid, ch);

      aflag = 0;
      if (value == 0xFFAA5500u) { aflag = 1; }
      else if (value == 0xAA5500FFu) {
        aflag = 1;
      }
      else if (value == 0x5500FFAAu) {
        aflag = 1;
      }
      else if (value == 0x00FFAA55u) {
        aflag = 1;
      }

      if (aflag) {
        count += 1;
        sum += static_cast<int>(dly);
        if (count > 4) { flag = 1; }
      }
      else {
        if (flag) { dly = 32; }
        else {
          count = 0;
          sum = 0;
        }
      }
    }

    if (count) { gdly = static_cast<unsigned long>(sum / count); }
    else {
      gdly = 9;
    }

    WriteDRAMDLY(mid, ch, gdly);

    int bitslip = 0;
    aflag = 0;
    for (bitslip = 0; bitslip < 4; bitslip++) {
      WriteDRAMTEST(mid, 3);
      unsigned long value = ReadDRAMTEST(mid, ch);

      if (value == 0xFFAA5500u) {
        aflag = 1;
        break;
      }
      else {
        aflag = 0;
        WriteDRAMBITSLIP(mid, ch);
      }
    }

    if (aflag) { INFO("DRAM(%lu) is aligned, delay = %lu", ch, gdly); }
    else {
      ERROR("fail to align DRAM(%lu)", ch);
    }
  }

  WriteDRAMTEST(mid, 0);
}
