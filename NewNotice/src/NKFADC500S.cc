#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <vector>

#include "DAQUtils/ELog.hh"
#include "NewNotice/NKFADC500S.hh"

namespace {

constexpr std::uint16_t kNKFADC500S_VENDOR_ID = 0x0547;
constexpr std::uint16_t kNKFADC500S_PRODUCT_ID = 0x1502;

inline std::uint32_t RegChOffset(std::uint32_t base, unsigned long ch) { return base + (((ch - 1UL) & 0xFFUL) << 16); }

inline std::uint32_t RegChOffsetZeroBase(std::uint32_t base, unsigned long ch) { return base + ((ch & 0xFFUL) << 16); }

} // namespace

NKFADC500S::NKFADC500S(int sid)
  : _sid(sid),
    _usb(kNKFADC500S_VENDOR_ID, kNKFADC500S_PRODUCT_ID, sid)
{
}

NKFADC500S::~NKFADC500S() { Close(); }

void NKFADC500S::SetSID(int sid)
{
  _sid = sid;
  _usb.Set(kNKFADC500S_VENDOR_ID, kNKFADC500S_PRODUCT_ID, _sid);
}

int NKFADC500S::Open()
{
  int status = _usb.Open();
  if (status < 0) {
    ERROR("NKFADC500S [sid=%d]: failed to open device", _sid);
    return status;
  }

  status = _usb.ClaimInterface(0);
  if (status < 0) {
    ERROR("NKFADC500S [sid=%d]: failed to claim interface 0", _sid);
    _usb.Close();
    return status;
  }

  return 0;
}

void NKFADC500S::Close()
{
  _usb.ReleaseInterface(0);
  _usb.Close();
}

void NKFADC500S::Reset() const { _usb.Write(0x20000000u, 1u << 2); }

void NKFADC500S::ResetTimer() const { _usb.Write(0x20000000u, 1u); }

void NKFADC500S::Start() const { _usb.Write(0x20000000u, 1u << 3); }

void NKFADC500S::Stop() const { _usb.Write(0x20000000u, 0u << 3); }

unsigned long NKFADC500S::ReadRun() const { return static_cast<unsigned long>(_usb.ReadReg(0x20000000u)); }

void NKFADC500S::WriteCW(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x20000001u, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC500S::ReadCW(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000001u, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC500S::WriteRL(unsigned long data) const { _usb.Write(0x20000002u, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC500S::ReadRL() const { return static_cast<unsigned long>(_usb.ReadReg(0x20000002u)); }

void NKFADC500S::WriteDRAMON(unsigned long data) const
{
  if (data) {
    unsigned int status = _usb.ReadReg(0x20000003u);
    if (status) { _usb.Write(0x20000003u, 0u); }

    _usb.Write(0x20000003u, 1u);

    status = 0;
    while (!status) {
      status = _usb.ReadReg(0x20000003u);
    }
  }
  else {
    _usb.Write(0x20000003u, 0u);
  }
}

unsigned long NKFADC500S::ReadDRAMON() const { return static_cast<unsigned long>(_usb.ReadReg(0x20000003u)); }

void NKFADC500S::WriteDACOFF(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x20000004u, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC500S::ReadDACOFF(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000004u, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC500S::MeasurePED(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000005u, ch);
  _usb.Write(addr, 0u);
}

unsigned long NKFADC500S::ReadPED(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000006u, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC500S::WriteDLY(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x20000007u, ch);
  const unsigned long value = ((data / 1000UL) << 10) | (data % 1000UL);
  _usb.Write(addr, static_cast<std::uint32_t>(value));
}

unsigned long NKFADC500S::ReadDLY(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000007u, ch);
  const unsigned long value = static_cast<unsigned long>(_usb.ReadReg(addr));
  const unsigned long data = (value >> 10) * 1000UL + (value & 0x3FFUL);
  return data;
}

void NKFADC500S::WriteTHR(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x20000008u, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC500S::ReadTHR(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000008u, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC500S::WritePOL(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x20000009u, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC500S::ReadPOL(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000009u, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC500S::WritePSW(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Au, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC500S::ReadPSW(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Au, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC500S::WriteAMODE(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Bu, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC500S::ReadAMODE(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Bu, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC500S::WritePCT(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Cu, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC500S::ReadPCT(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Cu, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC500S::WritePCI(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Du, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC500S::ReadPCI(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Du, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC500S::WritePWT(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Eu, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC500S::ReadPWT(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Eu, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC500S::WriteDT(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Fu, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC500S::ReadDT(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Fu, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

int NKFADC500S::ReadBCount() const { return static_cast<int>(_usb.ReadReg(0x20000010u)); }

void NKFADC500S::WritePTRIG(unsigned long data) const { _usb.Write(0x20000011u, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC500S::ReadPTRIG() const { return static_cast<unsigned long>(_usb.ReadReg(0x20000011u)); }

void NKFADC500S::SendTRIG() const { _usb.Write(0x20000012u, 0u); }

void NKFADC500S::WriteTRIGENABLE(unsigned long data) const
{
  _usb.Write(0x20000013u, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC500S::ReadTRIGENABLE() const { return static_cast<unsigned long>(_usb.ReadReg(0x20000013u)); }

void NKFADC500S::WriteTM(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x20000014u, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC500S::ReadTM(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000014u, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC500S::WriteTLT(unsigned long data) const { _usb.Write(0x20000015u, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC500S::ReadTLT() const { return static_cast<unsigned long>(_usb.ReadReg(0x20000015u)); }

void NKFADC500S::WriteZEROSUP(unsigned long, unsigned long data) const
{
  _usb.Write(0x20000016u, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC500S::ReadZEROSUP(unsigned long) const
{
  return static_cast<unsigned long>(_usb.ReadReg(0x20000016u));
}

void NKFADC500S::SendADCRST() const { _usb.Write(0x20000017u, 0u); }

void NKFADC500S::SendADCCAL() const { _usb.Write(0x20000018u, 0u); }

void NKFADC500S::WriteADCDLY(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x20000019u, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

void NKFADC500S::WriteADCALIGN(unsigned long data) const { _usb.Write(0x2000001Au, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC500S::ReadADCSTAT() const { return static_cast<unsigned long>(_usb.ReadReg(0x2000001Au)); }

void NKFADC500S::WriteDRAMDLY(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffsetZeroBase(0x2000001Bu, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

void NKFADC500S::WriteDRAMBITSLIP(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffsetZeroBase(0x2000001Cu, ch);
  _usb.Write(addr, 0u);
}

void NKFADC500S::WriteDRAMTEST(unsigned long data) const { _usb.Write(0x2000001Du, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC500S::ReadDRAMTEST(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffsetZeroBase(0x2000001Du, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC500S::WritePSCALE(unsigned long data) const { _usb.Write(0x2000001Eu, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC500S::ReadPSCALE() const { return static_cast<unsigned long>(_usb.ReadReg(0x2000001Eu)); }

void NKFADC500S::WriteDSR(unsigned long data) const { _usb.Write(0x2000001Fu, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC500S::ReadDSR() const { return static_cast<unsigned long>(_usb.ReadReg(0x2000001Fu)); }

int NKFADC500S::ReadData(int bcount, unsigned char * data, unsigned int timeout) const
{
  if (bcount <= 0 || data == nullptr) {
    ERROR("NKFADC500S [sid=%d]: invalid arguments (bcount=%d, data=%p)", _sid, bcount, static_cast<void *>(data));
    return -1;
  }

  const int count = bcount * 256;
  return _usb.Read(static_cast<std::uint32_t>(count), 0x40000000u, data, timeout);
}

void NKFADC500S::FlushData() const
{
  int bcount = ReadBCount();
  if (bcount <= 0) { return; }

  std::vector<unsigned char> buffer(10485760);

  const unsigned long total = static_cast<unsigned long>(bcount);
  const unsigned long chunk = total / 10240UL;
  const unsigned long slice = total % 10240UL;

  for (unsigned long i = 0; i < chunk; ++i) {
    ReadData(10240, buffer.data());
  }

  if (slice) { ReadData(static_cast<int>(slice), buffer.data()); }
}

void NKFADC500S::AlignADC() const
{
  unsigned long ch;
  unsigned long dly;
  unsigned long value;
  int count;
  int sum;
  int center;
  unsigned long gdly;
  int flag;

  SendADCRST();
  usleep(500000);
  SendADCCAL();
  WriteADCALIGN(1);

  for (ch = 1; ch <= 4; ++ch) {
    count = 0;
    sum = 0;
    flag = 0;

    for (dly = 0; dly < 32; ++dly) {
      WriteADCDLY(ch, dly);
      value = (ReadADCSTAT() >> (ch - 1)) & 0x1UL;

      if (!value) {
        flag = 1;
        ++count;
        sum += static_cast<int>(dly);
      }
      else {
        if (flag) { dly = 32; }
      }
    }

    if (count > 0) { center = sum / count; }
    else {
      center = 0;
    }

    if (center < 11) { gdly = static_cast<unsigned long>(center + 11); }
    else {
      gdly = static_cast<unsigned long>(center - 11);
    }

    WriteADCDLY(ch, gdly);
    INFO("NKFADC500S ADC(%lu) calibration delay = %lu", ch, gdly);
  }

  WriteADCALIGN(0);
  SendADCCAL();
}

void NKFADC500S::AlignDRAM() const
{
  unsigned long ch;
  unsigned long dly;
  unsigned long value;
  int flag;
  int count;
  int sum;
  int aflag;
  unsigned long gdly;
  int bitslip;

  WriteDRAMON(1);
  WriteDRAMTEST(1);
  SendADCCAL();
  WriteDRAMTEST(2);

  for (ch = 0; ch < 8; ++ch) {
    count = 0;
    sum = 0;
    flag = 0;

    for (dly = 0; dly < 32; ++dly) {
      WriteDRAMDLY(ch, dly);

      WriteDRAMTEST(3);
      value = ReadDRAMTEST(ch);

      aflag = 0;
      if (value == 0xFFAA5500UL) { aflag = 1; }
      else if (value == 0xAA5500FFUL) {
        aflag = 1;
      }
      else if (value == 0x5500FFAAUL) {
        aflag = 1;
      }
      else if (value == 0x00FFAA55UL) {
        aflag = 1;
      }

      if (aflag) {
        ++count;
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

    if (count > 0) { gdly = static_cast<unsigned long>(sum / count); }
    else {
      gdly = 9;
    }

    WriteDRAMDLY(ch, gdly);

    aflag = 0;
    for (bitslip = 0; bitslip < 4; ++bitslip) {
      WriteDRAMTEST(3);
      value = ReadDRAMTEST(ch);

      if (value == 0xFFAA5500UL) {
        aflag = 1;
        bitslip = 4;
      }
      else {
        aflag = 0;
        WriteDRAMBITSLIP(ch);
      }
    }

    if (aflag) { INFO("NKFADC500S DRAM(%lu) is aligned, delay = %lu", ch, gdly); }
    else {
      INFO("NKFADC500S: fail to align DRAM(%lu)!", ch);
    }
  }

  WriteDRAMTEST(0);
}
