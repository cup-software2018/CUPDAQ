#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <vector>

#include "DAQUtils/ELog.hh"
#include "Notice/NKFADC125S.hh"

namespace {

constexpr std::uint16_t kNKFADC125S_VENDOR_ID = 0x0547;
constexpr std::uint16_t kNKFADC125S_PRODUCT_ID = 0x1903;

inline std::uint32_t RegChOffset(std::uint32_t base, unsigned long ch) { return base + (((ch - 1UL) & 0xFFUL) << 16); }
inline std::uint32_t RegChOffsetZeroBase(std::uint32_t base, unsigned long ch) { return base + ((ch & 0xFFUL) << 16); }

} // namespace

NKFADC125S::NKFADC125S(int sid)
  : _sid(sid),
    _usb(kNKFADC125S_VENDOR_ID, kNKFADC125S_PRODUCT_ID, sid)
{
}

NKFADC125S::~NKFADC125S() { Close(); }

void NKFADC125S::SetSID(int sid)
{
  _sid = sid;
  _usb.Set(kNKFADC125S_VENDOR_ID, kNKFADC125S_PRODUCT_ID, _sid);
}

int NKFADC125S::Open()
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

void NKFADC125S::Close()
{
  _usb.ReleaseInterface(0);
  _usb.Close();
}

void NKFADC125S::Reset() const { _usb.Write(0x20000000u, 1u << 2); }

void NKFADC125S::ResetTimer() const { _usb.Write(0x20000000u, 1u); }

void NKFADC125S::Start() const { _usb.Write(0x20000000u, 1u << 3); }

void NKFADC125S::Stop() const { _usb.Write(0x20000000u, 0u << 3); }

unsigned long NKFADC125S::ReadRun() const { return static_cast<unsigned long>(_usb.ReadReg(0x20000000u)); }

void NKFADC125S::WriteCW(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x20000001u, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadCW(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000001u, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WriteRL(unsigned long data) const { _usb.Write(0x20000002u, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC125S::ReadRL() const { return static_cast<unsigned long>(_usb.ReadReg(0x20000002u)); }

void NKFADC125S::WriteDRAMON(unsigned long data) const
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

unsigned long NKFADC125S::ReadDRAMON() const { return static_cast<unsigned long>(_usb.ReadReg(0x20000003u)); }

void NKFADC125S::WriteDACOFF(unsigned long ch, unsigned long data) const
{
  if (ch) {
    const std::uint32_t addr = RegChOffset(0x20000004u, ch);
    _usb.Write(addr, static_cast<std::uint32_t>(data));
  }
  else {
    const unsigned long nch = ReadCHNUM();
    for (unsigned long chan = 1; chan <= nch; ++chan) {
      const std::uint32_t addr = RegChOffset(0x20000004u, chan);
      _usb.Write(addr, static_cast<std::uint32_t>(data));
    }
  }
}

unsigned long NKFADC125S::ReadDACOFF(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000004u, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::MeasurePED(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000005u, ch);
  _usb.Write(addr, 0u);
}

unsigned long NKFADC125S::ReadPED(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000006u, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WriteDLY(unsigned long, unsigned long data) const
{
  const unsigned long value = ((data / 1000UL) << 10) | (data % 1000UL);
  _usb.Write(0x20000007u, static_cast<std::uint32_t>(value));
}

unsigned long NKFADC125S::ReadDLY(unsigned long) const
{
  const unsigned long value = static_cast<unsigned long>(_usb.ReadReg(0x20000007u));
  const unsigned long data = (value >> 10) * 1000UL + (value & 0x3FFUL);
  return data;
}

void NKFADC125S::WriteTHR(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x20000008u, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadTHR(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000008u, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WritePOL(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x20000009u, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadPOL(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000009u, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WritePSW(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Au, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadPSW(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Au, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WritePCT(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Cu, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadPCT(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Cu, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WritePCI(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Du, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadPCI(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Du, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WritePWT(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Eu, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadPWT(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Eu, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WriteDT(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Fu, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadDT(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x2000000Fu, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WritePTRIG(unsigned long data) const { _usb.Write(0x20000011u, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC125S::ReadPTRIG() const { return static_cast<unsigned long>(_usb.ReadReg(0x20000011u)); }

void NKFADC125S::SendTRIG() const { _usb.Write(0x20000012u, 0u); }

unsigned long NKFADC125S::ReadCHNUM() const { return static_cast<unsigned long>(_usb.ReadReg(0x20000012u)); }

void NKFADC125S::WriteTRIGENABLE(unsigned long data) const
{
  _usb.Write(0x20000013u, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadTRIGENABLE() const { return static_cast<unsigned long>(_usb.ReadReg(0x20000013u)); }

void NKFADC125S::WriteTM(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x20000014u, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadTM(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(0x20000014u, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WriteTLT(unsigned long data) const { _usb.Write(0x20000015u, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC125S::ReadTLT() const { return static_cast<unsigned long>(_usb.ReadReg(0x20000015u)); }

void NKFADC125S::SendADCRST() const { _usb.Write(0x20000017u, 0u); }

void NKFADC125S::SendADCCAL() const { _usb.Write(0x20000018u, 0u); }

void NKFADC125S::WriteADCDLY(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x20000019u, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

void NKFADC125S::WriteADCSETUP(unsigned long addr, unsigned long data) const
{
  const unsigned long value = (addr << 8) | (data & 0xFFUL);
  _usb.Write(0x2000001Au, static_cast<std::uint32_t>(value));
}

unsigned long NKFADC125S::ReadADCSTAT() const { return static_cast<unsigned long>(_usb.ReadReg(0x2000001Au)); }

void NKFADC125S::WriteDRAMDLY(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffsetZeroBase(0x2000001Bu, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

void NKFADC125S::WriteDRAMBITSLIP(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffsetZeroBase(0x2000001Cu, ch);
  _usb.Write(addr, 0u);
}

void NKFADC125S::WriteDRAMTEST(unsigned long data) const { _usb.Write(0x2000001Du, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC125S::ReadDRAMTEST(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffsetZeroBase(0x2000001Du, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WriteADCBITSLIP(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(0x2000001Eu, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

void NKFADC125S::WritePSCALE(unsigned long data) const { _usb.Write(0x20000006u, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC125S::ReadPSCALE() const { return static_cast<unsigned long>(_usb.ReadReg(0x2000001Eu)); }

void NKFADC125S::WriteDSR(unsigned long data) const { _usb.Write(0x2000001Fu, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC125S::ReadDSR() const { return static_cast<unsigned long>(_usb.ReadReg(0x2000001Fu)); }

int NKFADC125S::ReadBCount() const { return static_cast<int>(_usb.ReadReg(0x20000010u)); }

int NKFADC125S::ReadData(int bcount, unsigned char * data, unsigned int timeout) const
{
  if (bcount <= 0 || data == nullptr) {
    ERROR("invalid arguments (bcount=%d, data=%p, sid=%d)", bcount, static_cast<void *>(data), _sid);
    return -1;
  }

  const int count = bcount * 256;
  return _usb.Read(static_cast<std::uint32_t>(count), 0x40000000u, data, timeout);
}

void NKFADC125S::FlushData() const
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

void NKFADC125S::AlignADC() const
{
  int mini;
  unsigned long nch;
  unsigned long ch;
  unsigned long dly;
  unsigned long value;
  int bit_okay;
  int word_okay;
  int flag;
  int count;
  int sum;
  unsigned long gdly;
  unsigned long bitslip;
  unsigned long gbitslip;

  nch = ReadCHNUM();
  if (nch > 4) { mini = 0; }
  else {
    mini = 1;
  }

  SendADCRST();
  usleep(1000000);
  SendADCCAL();

  WriteADCSETUP(0x009, 0x02);

  for (ch = 1; ch <= nch; ++ch) {
    WriteADCSETUP(0x0006, 0x02);
    WriteADCSETUP(0x000A, 0x33);
    WriteADCSETUP(0x000B, 0x33);

    WriteADCBITSLIP(ch, 0);

    sum = 0;
    count = 0;
    flag = 0;
    gbitslip = 0;

    for (dly = 0; dly < 32; ++dly) {
      WriteADCDLY(ch, dly);

      value = ReadADCSTAT();
      bit_okay = static_cast<int>((value >> (ch - 1)) & 0x1UL);

      if (bit_okay) {
        ++count;
        sum += static_cast<int>(dly);
        if (count > 5) { ++flag; }
      }
      else {
        if (flag) { dly = 32; }
        else {
          sum = 0;
          count = 0;
        }
      }
    }

    if (count) { gdly = static_cast<unsigned long>(sum / count); }
    else {
      gdly = 0;
    }

    WriteADCDLY(ch, gdly);

    WriteADCSETUP(0x000E, 0x81);
    WriteADCSETUP(0x000F, 0x00);
    WriteADCSETUP(0x000A, 0x55);
    WriteADCSETUP(0x000B, 0x55);

    for (bitslip = 0; bitslip < 7; ++bitslip) {
      if (!mini) { WriteADCBITSLIP(ch, bitslip); }

      value = ReadADCSTAT();
      word_okay = static_cast<int>((value >> (ch + 15)) & 0x1UL);

      if (word_okay) {
        ++flag;
        gbitslip = bitslip;
        bitslip = 7;
      }
      else {
        if (mini) { WriteADCBITSLIP(ch, bitslip); }
      }
    }

    if (flag > 1) { INFO("ADC(%lu) is aligned, delay = %lu, bitslip = %lu", ch, gdly, gbitslip); }
    else {
      INFO("Fail to align ADC(%lu)!", ch);
    }
  }

  WriteADCSETUP(0x122, 0x02);
  WriteADCSETUP(0x222, 0x02);
  WriteADCSETUP(0x422, 0x02);
  WriteADCSETUP(0x522, 0x02);

  WriteADCSETUP(0x0009, 0x01);
  WriteADCSETUP(0x0006, 0x00);
  WriteADCSETUP(0x000A, 0x00);
  WriteADCSETUP(0x000B, 0x00);
}

void NKFADC125S::AlignDRAM() const
{
  int mini;
  unsigned long ch;
  unsigned long dly;
  unsigned long value;
  int flag;
  int count;
  int sum;
  int aflag;
  unsigned long gdly;
  int bitslip;
  int gbitslip = 0;

  value = ReadCHNUM();
  if (value > 4) { mini = 0; }
  else {
    mini = 1;
  }

  WriteDRAMON(1);

  if (mini) { INFO("DRAM is aligned"); }
  else {
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

      if (count) { gdly = static_cast<unsigned long>(sum / count); }
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
          gbitslip = bitslip;
          bitslip = 4;
        }
        else {
          aflag = 0;
          WriteDRAMBITSLIP(ch);
        }
      }

      if (aflag) { INFO("DRAM(%lu) is aligned, delay = %lu, bitslip = %d", ch, gdly, gbitslip); }
      else {
        INFO("Fail to align DRAM(%lu)!", ch);
      }
    }

    WriteDRAMTEST(0);
  }
}
