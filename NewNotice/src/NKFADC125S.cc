#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <vector>

#include "DAQUtils/ELog.hh"
#include "NewNotice/NKFADC125S.hh"

namespace {

constexpr uint16_t kNKFADC125S_VENDOR_ID = 0x0547;
constexpr uint16_t kNKFADC125S_PRODUCT_ID = 0x1903;

constexpr std::uint32_t kREG_RUN = 0x20000000u;
constexpr std::uint32_t kREG_CW_BASE = 0x20000001u;
constexpr std::uint32_t kREG_RL = 0x20000002u;
constexpr std::uint32_t kREG_DRAMON = 0x20000003u;
constexpr std::uint32_t kREG_DACOFF_BASE = 0x20000004u;
constexpr std::uint32_t kREG_PED_MEASURE_BASE = 0x20000005u;
constexpr std::uint32_t kREG_PED_READ_BASE = 0x20000006u;
constexpr std::uint32_t kREG_DLY = 0x20000007u;
constexpr std::uint32_t kREG_THR_BASE = 0x20000008u;
constexpr std::uint32_t kREG_POL_BASE = 0x20000009u;
constexpr std::uint32_t kREG_PSW_BASE = 0x2000000Au;
constexpr std::uint32_t kREG_PCT_BASE = 0x2000000Cu;
constexpr std::uint32_t kREG_PCI_BASE = 0x2000000Du;
constexpr std::uint32_t kREG_PWT_BASE = 0x2000000Eu;
constexpr std::uint32_t kREG_DT_BASE = 0x2000000Fu;
constexpr std::uint32_t kREG_BCOUNT = 0x20000010u;
constexpr std::uint32_t kREG_PTRIG = 0x20000011u;
constexpr std::uint32_t kREG_TRIG_CHNUM = 0x20000012u;
constexpr std::uint32_t kREG_TRIGENABLE = 0x20000013u;
constexpr std::uint32_t kREG_TM_BASE = 0x20000014u;
constexpr std::uint32_t kREG_TLT = 0x20000015u;
constexpr std::uint32_t kREG_ADCRST = 0x20000017u;
constexpr std::uint32_t kREG_ADCCAL = 0x20000018u;
constexpr std::uint32_t kREG_ADCDLY_BASE = 0x20000019u;
constexpr std::uint32_t kREG_ADCSETUP = 0x2000001Au;
constexpr std::uint32_t kREG_DRAMDLY_BASE = 0x2000001Bu;
constexpr std::uint32_t kREG_DRAMBITSLIP_BASE = 0x2000001Cu;
constexpr std::uint32_t kREG_DRAMTEST_BASE = 0x2000001Du;
constexpr std::uint32_t kREG_ADCBITSLIP_BASE = 0x2000001Eu;
constexpr std::uint32_t kREG_PSCALE_WRITE = 0x20000006u;
constexpr std::uint32_t kREG_PSCALE_READ = 0x2000001Eu;
constexpr std::uint32_t kREG_DSR = 0x2000001Fu;

constexpr std::uint32_t kADDR_DATA = 0x40000000u;

inline std::uint32_t RegChOffset(std::uint32_t base, unsigned long ch) { return base + (((ch - 1UL) & 0xFFUL) << 16); }

inline std::uint32_t RegChOffsetZeroBase(std::uint32_t base, unsigned long ch) { return base + ((ch & 0xFFUL) << 16); }

} // namespace

NKFADC125S::NKFADC125S(int sid)
  : _sid(sid),
    _usb(kNKFADC125S_VENDOR_ID, kNKFADC125S_PRODUCT_ID, sid)
{
}

NKFADC125S::~NKFADC125S() { Close(); }

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

void NKFADC125S::Reset() const { _usb.Write(kREG_RUN, 1u << 2); }

void NKFADC125S::ResetTimer() const { _usb.Write(kREG_RUN, 1u); }

void NKFADC125S::Start() const { _usb.Write(kREG_RUN, 1u << 3); }

void NKFADC125S::Stop() const { _usb.Write(kREG_RUN, 0u << 3); }

unsigned long NKFADC125S::ReadRun() const { return static_cast<unsigned long>(_usb.ReadReg(kREG_RUN)); }

void NKFADC125S::WriteCW(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(kREG_CW_BASE, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadCW(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(kREG_CW_BASE, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WriteRL(unsigned long data) const { _usb.Write(kREG_RL, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC125S::ReadRL() const { return static_cast<unsigned long>(_usb.ReadReg(kREG_RL)); }

void NKFADC125S::WriteDRAMOn(unsigned long data) const
{
  if (data) {
    unsigned int status = _usb.ReadReg(kREG_DRAMON);
    if (status) { _usb.Write(kREG_DRAMON, 0u); }

    _usb.Write(kREG_DRAMON, 1u);

    status = 0;
    while (!status) {
      status = _usb.ReadReg(kREG_DRAMON);
    }
  }
  else {
    _usb.Write(kREG_DRAMON, 0u);
  }
}

unsigned long NKFADC125S::ReadDRAMOn() const { return static_cast<unsigned long>(_usb.ReadReg(kREG_DRAMON)); }

void NKFADC125S::WriteDACOFF(unsigned long ch, unsigned long data) const
{
  if (ch) {
    const std::uint32_t addr = RegChOffset(kREG_DACOFF_BASE, ch);
    _usb.Write(addr, static_cast<std::uint32_t>(data));
  }
  else {
    const unsigned long nch = ReadCHNUM();
    for (unsigned long chan = 1; chan <= nch; ++chan) {
      const std::uint32_t addr = RegChOffset(kREG_DACOFF_BASE, chan);
      _usb.Write(addr, static_cast<std::uint32_t>(data));
    }
  }
}

unsigned long NKFADC125S::ReadDACOFF(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(kREG_DACOFF_BASE, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::MeasurePED(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(kREG_PED_MEASURE_BASE, ch);
  _usb.Write(addr, 0u);
}

unsigned long NKFADC125S::ReadPED(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(kREG_PED_READ_BASE, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WriteDLY(unsigned long, unsigned long data) const
{
  const unsigned long value = ((data / 1000UL) << 10) | (data % 1000UL);
  _usb.Write(kREG_DLY, static_cast<std::uint32_t>(value));
}

unsigned long NKFADC125S::ReadDLY(unsigned long) const
{
  const unsigned long value = static_cast<unsigned long>(_usb.ReadReg(kREG_DLY));
  const unsigned long data = (value >> 10) * 1000UL + (value & 0x3FFUL);
  return data;
}

void NKFADC125S::WriteTHR(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(kREG_THR_BASE, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadTHR(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(kREG_THR_BASE, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WritePOL(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(kREG_POL_BASE, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadPOL(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(kREG_POL_BASE, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WritePSW(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(kREG_PSW_BASE, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadPSW(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(kREG_PSW_BASE, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WritePCT(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(kREG_PCT_BASE, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadPCT(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(kREG_PCT_BASE, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WritePCI(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(kREG_PCI_BASE, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadPCI(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(kREG_PCI_BASE, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WritePWT(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(kREG_PWT_BASE, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadPWT(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(kREG_PWT_BASE, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WriteDT(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(kREG_DT_BASE, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadDT(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(kREG_DT_BASE, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WritePTRIG(unsigned long data) const { _usb.Write(kREG_PTRIG, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC125S::ReadPTRIG() const { return static_cast<unsigned long>(_usb.ReadReg(kREG_PTRIG)); }

void NKFADC125S::SendTRIG() const { _usb.Write(kREG_TRIG_CHNUM, 0u); }

unsigned long NKFADC125S::ReadCHNUM() const { return static_cast<unsigned long>(_usb.ReadReg(kREG_TRIG_CHNUM)); }

void NKFADC125S::WriteTRIGENABLE(unsigned long data) const
{
  _usb.Write(kREG_TRIGENABLE, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadTRIGENABLE() const { return static_cast<unsigned long>(_usb.ReadReg(kREG_TRIGENABLE)); }

void NKFADC125S::WriteTM(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(kREG_TM_BASE, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadTM(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffset(kREG_TM_BASE, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WriteTLT(unsigned long data) const { _usb.Write(kREG_TLT, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC125S::ReadTLT() const { return static_cast<unsigned long>(_usb.ReadReg(kREG_TLT)); }

void NKFADC125S::SendADCRST() const { _usb.Write(kREG_ADCRST, 0u); }

void NKFADC125S::SendADCCAL() const { _usb.Write(kREG_ADCCAL, 0u); }

void NKFADC125S::WriteADCDLY(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(kREG_ADCDLY_BASE, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

void NKFADC125S::WriteADCSETUP(unsigned long addr, unsigned long data) const
{
  const unsigned long value = (addr << 8) | (data & 0xFFUL);
  _usb.Write(kREG_ADCSETUP, static_cast<std::uint32_t>(value));
}

unsigned long NKFADC125S::ReadADCSTAT() const { return static_cast<unsigned long>(_usb.ReadReg(kREG_ADCSETUP)); }

void NKFADC125S::WriteDRAMDLY(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffsetZeroBase(kREG_DRAMDLY_BASE, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

void NKFADC125S::WriteDRAMBITSLIP(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffsetZeroBase(kREG_DRAMBITSLIP_BASE, ch);
  _usb.Write(addr, 0u);
}

void NKFADC125S::WriteDRAMTEST(unsigned long data) const
{
  _usb.Write(kREG_DRAMTEST_BASE, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadDRAMTEST(unsigned long ch) const
{
  const std::uint32_t addr = RegChOffsetZeroBase(kREG_DRAMTEST_BASE, ch);
  return static_cast<unsigned long>(_usb.ReadReg(addr));
}

void NKFADC125S::WriteADCBITSLIP(unsigned long ch, unsigned long data) const
{
  const std::uint32_t addr = RegChOffset(kREG_ADCBITSLIP_BASE, ch);
  _usb.Write(addr, static_cast<std::uint32_t>(data));
}

void NKFADC125S::WritePSCALE(unsigned long data) const
{
  _usb.Write(kREG_PSCALE_WRITE, static_cast<std::uint32_t>(data));
}

unsigned long NKFADC125S::ReadPSCALE() const { return static_cast<unsigned long>(_usb.ReadReg(kREG_PSCALE_READ)); }

void NKFADC125S::WriteDSR(unsigned long data) const { _usb.Write(kREG_DSR, static_cast<std::uint32_t>(data)); }

unsigned long NKFADC125S::ReadDSR() const { return static_cast<unsigned long>(_usb.ReadReg(kREG_DSR)); }

int NKFADC125S::ReadBCount() const { return static_cast<int>(_usb.ReadReg(kREG_BCOUNT)); }

int NKFADC125S::ReadData(int bcount, unsigned char * data, unsigned int timeout) const
{
  if (bcount <= 0 || data == nullptr) {
    ERROR("invalid arguments (bcount=%d, data=%p, sid=%d)", bcount, static_cast<void *>(data),
          _sid);
    return -1;
  }

  const int count = bcount * 256;
  return _usb.Read(static_cast<std::uint32_t>(count), kADDR_DATA, data, timeout);
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

  WriteDRAMOn(1);

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
