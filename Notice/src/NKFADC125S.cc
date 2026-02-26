#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <vector>

#include "DAQUtils/ELog.hh"
#include "Notice/NKFADC125S.hh"

namespace {

constexpr std::uint16_t kVENDOR_ID = 0x0547;
constexpr std::uint16_t kPRODUCT_ID = 0x1903;

// Register Addresses
constexpr uint32_t kAddr_RUN = 0x20000000u;
constexpr uint32_t kAddr_CW = 0x20000001u;
constexpr uint32_t kAddr_RL = 0x20000002u;
constexpr uint32_t kAddr_DRAMON = 0x20000003u;
constexpr uint32_t kAddr_DACOFF = 0x20000004u;
constexpr uint32_t kAddr_PED_CMD = 0x20000005u;
constexpr uint32_t kAddr_PED_READ = 0x20000006u;
constexpr uint32_t kAddr_DLY = 0x20000007u;
constexpr uint32_t kAddr_THR = 0x20000008u;
constexpr uint32_t kAddr_POL = 0x20000009u;
constexpr uint32_t kAddr_PSW = 0x2000000Au;
// 0xB is skipped?
constexpr uint32_t kAddr_PCT = 0x2000000Cu;
constexpr uint32_t kAddr_PCI = 0x2000000Du;
constexpr uint32_t kAddr_PWT = 0x2000000Eu;
constexpr uint32_t kAddr_DT = 0x2000000Fu;

constexpr uint32_t kAddr_BCOUNT = 0x20000010u;
constexpr uint32_t kAddr_PTRIG = 0x20000011u;
constexpr uint32_t kAddr_TRIG_CMD = 0x20000012u; // Also ReadCHNUM
constexpr uint32_t kAddr_TRIG_ENABLE = 0x20000013u;
constexpr uint32_t kAddr_TM = 0x20000014u;
constexpr uint32_t kAddr_TLT = 0x20000015u;
// 0x16?
constexpr uint32_t kAddr_ADCRST = 0x20000017u;
constexpr uint32_t kAddr_ADCCAL = 0x20000018u;
constexpr uint32_t kAddr_ADCDLY = 0x20000019u;
constexpr uint32_t kAddr_ADCSETUP = 0x2000001Au; // Also ADCSTAT
constexpr uint32_t kAddr_DRAMDLY = 0x2000001Bu;
constexpr uint32_t kAddr_DRAMBITS = 0x2000001Cu;
constexpr uint32_t kAddr_DRAMTEST = 0x2000001Du;
constexpr uint32_t kAddr_ADCBITS = 0x2000001Eu; // Also PSCALE read?
constexpr uint32_t kAddr_PSCALE =
    0x20000006u; // Write addr distinct from read? (Original code used 0x06 for write)
constexpr uint32_t kAddr_DSR = 0x2000001Fu;

constexpr uint32_t kAddr_DATA = 0x40000000u;

// Helper to calculate register address for a channel
// Base + ((ch-1) << 16)
inline uint32_t GetChAddr(uint32_t base, uint32_t ch) { return base + (((ch - 1u) & 0xFFu) << 16); }

// Base + (ch << 16) - For zero-based channel index registers like DRAMDLY
inline uint32_t GetChAddrZB(uint32_t base, uint32_t ch) { return base + ((ch & 0xFFu) << 16); }

void SleepMs(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

} // namespace

NKFADC125S::NKFADC125S(int sid)
  : _sid(sid),
    _usb(kVENDOR_ID, kPRODUCT_ID, sid)
{
}

NKFADC125S::~NKFADC125S() { Close(); }

void NKFADC125S::SetSID(int sid)
{
  _sid = sid;
  _usb.Set(kVENDOR_ID, kPRODUCT_ID, _sid);
}

int NKFADC125S::Open()
{
  int status = _usb.Open();
  if (status < 0) {
    ERROR("NKFADC125S: failed to open device (sid=%d)", _sid);
    return status;
  }

  status = _usb.ClaimInterface(0);
  if (status < 0) {
    ERROR("NKFADC125S: failed to claim interface 0 (sid=%d)", _sid);
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

void NKFADC125S::Reset() const { _usb.Write(kAddr_RUN, 1u << 2); }
void NKFADC125S::ResetTimer() const { _usb.Write(kAddr_RUN, 1u); }
void NKFADC125S::Start() const { _usb.Write(kAddr_RUN, 1u << 3); }
void NKFADC125S::Stop() const { _usb.Write(kAddr_RUN, 0u); }

uint32_t NKFADC125S::ReadRun() const { return _usb.ReadReg(kAddr_RUN); }

void NKFADC125S::WriteCW(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_CW, ch), data);
}
uint32_t NKFADC125S::ReadCW(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_CW, ch)); }

void NKFADC125S::WriteRL(uint32_t data) const { _usb.Write(kAddr_RL, data); }
uint32_t NKFADC125S::ReadRL() const { return _usb.ReadReg(kAddr_RL); }

void NKFADC125S::WriteDRAMON(uint32_t data) const
{
  if (data) {
    uint32_t status = _usb.ReadReg(kAddr_DRAMON);
    if (status) { _usb.Write(kAddr_DRAMON, 0u); }

    _usb.Write(kAddr_DRAMON, 1u);

    // Added Timeout to prevent infinite loop
    int timeout = 1000;
    while (_usb.ReadReg(kAddr_DRAMON) == 0 && timeout > 0) {
      timeout--;
      // Optional: short sleep could be added here
    }
    if (timeout == 0) ERROR("WriteDRAMON timeout");
  }
  else {
    _usb.Write(kAddr_DRAMON, 0u);
  }
}

uint32_t NKFADC125S::ReadDRAMON() const { return _usb.ReadReg(kAddr_DRAMON); }

void NKFADC125S::WriteDACOFF(uint32_t ch, uint32_t data) const
{
  if (ch > 0) { _usb.Write(GetChAddr(kAddr_DACOFF, ch), data); }
  else {
    // Broadcast to all channels
    uint32_t nch = ReadCHNUM();
    for (uint32_t i = 1; i <= nch; ++i) {
      _usb.Write(GetChAddr(kAddr_DACOFF, i), data);
    }
  }
}

uint32_t NKFADC125S::ReadDACOFF(uint32_t ch) const
{
  return _usb.ReadReg(GetChAddr(kAddr_DACOFF, ch));
}

void NKFADC125S::MeasurePED(uint32_t ch) const { _usb.Write(GetChAddr(kAddr_PED_CMD, ch), 0u); }
uint32_t NKFADC125S::ReadPED(uint32_t ch) const
{
  return _usb.ReadReg(GetChAddr(kAddr_PED_READ, ch));
}

void NKFADC125S::WriteDLY(uint32_t, uint32_t data) const
{
  uint32_t value = ((data / 1000u) << 10) | (data % 1000u);
  _usb.Write(kAddr_DLY, value);
}

uint32_t NKFADC125S::ReadDLY(uint32_t) const
{
  uint32_t value = _usb.ReadReg(kAddr_DLY);
  return (value >> 10) * 1000u + (value & 0x3FFu);
}

// Simple Wrappers
void NKFADC125S::WriteTHR(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_THR, ch), data);
}
uint32_t NKFADC125S::ReadTHR(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_THR, ch)); }

void NKFADC125S::WritePOL(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_POL, ch), data);
}
uint32_t NKFADC125S::ReadPOL(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_POL, ch)); }

void NKFADC125S::WritePSW(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_PSW, ch), data);
}
uint32_t NKFADC125S::ReadPSW(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_PSW, ch)); }

void NKFADC125S::WritePCT(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_PCT, ch), data);
}
uint32_t NKFADC125S::ReadPCT(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_PCT, ch)); }

void NKFADC125S::WritePCI(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_PCI, ch), data);
}
uint32_t NKFADC125S::ReadPCI(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_PCI, ch)); }

void NKFADC125S::WritePWT(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_PWT, ch), data);
}
uint32_t NKFADC125S::ReadPWT(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_PWT, ch)); }

void NKFADC125S::WriteDT(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_DT, ch), data);
}
uint32_t NKFADC125S::ReadDT(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_DT, ch)); }

void NKFADC125S::WritePTRIG(uint32_t data) const { _usb.Write(kAddr_PTRIG, data); }
uint32_t NKFADC125S::ReadPTRIG() const { return _usb.ReadReg(kAddr_PTRIG); }

void NKFADC125S::SendTRIG() const { _usb.Write(kAddr_TRIG_CMD, 0u); }
uint32_t NKFADC125S::ReadCHNUM() const { return _usb.ReadReg(kAddr_TRIG_CMD); }

void NKFADC125S::WriteTRIGENABLE(uint32_t data) const { _usb.Write(kAddr_TRIG_ENABLE, data); }
uint32_t NKFADC125S::ReadTRIGENABLE() const { return _usb.ReadReg(kAddr_TRIG_ENABLE); }

void NKFADC125S::WriteTM(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_TM, ch), data);
}
uint32_t NKFADC125S::ReadTM(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_TM, ch)); }

void NKFADC125S::WriteTLT(uint32_t data) const { _usb.Write(kAddr_TLT, data); }
uint32_t NKFADC125S::ReadTLT() const { return _usb.ReadReg(kAddr_TLT); }

void NKFADC125S::SendADCRST() const { _usb.Write(kAddr_ADCRST, 0u); }
void NKFADC125S::SendADCCAL() const { _usb.Write(kAddr_ADCCAL, 0u); }

void NKFADC125S::WriteADCDLY(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_ADCDLY, ch), data);
}

void NKFADC125S::WriteADCSETUP(uint32_t addr, uint32_t data) const
{
  uint32_t value = (addr << 8) | (data & 0xFFu);
  _usb.Write(kAddr_ADCSETUP, value);
}
uint32_t NKFADC125S::ReadADCSTAT() const { return _usb.ReadReg(kAddr_ADCSETUP); }

void NKFADC125S::WriteDRAMDLY(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddrZB(kAddr_DRAMDLY, ch), data);
}
void NKFADC125S::WriteDRAMBITSLIP(uint32_t ch) const
{
  _usb.Write(GetChAddrZB(kAddr_DRAMBITS, ch), 0u);
}

void NKFADC125S::WriteDRAMTEST(uint32_t data) const { _usb.Write(kAddr_DRAMTEST, data); }
uint32_t NKFADC125S::ReadDRAMTEST(uint32_t ch) const
{
  return _usb.ReadReg(GetChAddrZB(kAddr_DRAMTEST, ch));
}

void NKFADC125S::WriteADCBITSLIP(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_ADCBITS, ch), data);
}

void NKFADC125S::WritePSCALE(uint32_t data) const { _usb.Write(kAddr_PSCALE, data); }
// Note: Read addr 0x2000001E (ADCBITS) for PSCALE read? Preserving original logic.
uint32_t NKFADC125S::ReadPSCALE() const { return _usb.ReadReg(kAddr_ADCBITS); }

void NKFADC125S::WriteDSR(uint32_t data) const { _usb.Write(kAddr_DSR, data); }
uint32_t NKFADC125S::ReadDSR() const { return _usb.ReadReg(kAddr_DSR); }

int NKFADC125S::ReadBCount() const { return static_cast<int>(_usb.ReadReg(kAddr_BCOUNT)); }

int NKFADC125S::ReadData(int bcount, unsigned char * data, unsigned int timeout) const
{
  if (bcount <= 0 || data == nullptr) {
    ERROR("NKFADC125S: invalid ReadData arguments");
    return -1;
  }
  uint32_t count = static_cast<uint32_t>(bcount) * 256;
  return _usb.Read(count, kAddr_DATA, data, timeout);
}

void NKFADC125S::FlushData() const
{
  int bcount = ReadBCount();
  if (bcount <= 0) return;

  // Use a smaller fixed-size buffer on heap or stack to drain data
  // instead of allocating 10MB every time.
  constexpr int kFlushChunkSize = 10240 * 256; // ~2.6MB
  std::vector<unsigned char> buffer(kFlushChunkSize);

  uint32_t total_bytes = static_cast<uint32_t>(bcount) * 256;

  // Just drain it using existing USB API
  // Note: if USB3Com::Read handles large buffers, we can just call it in chunks
  // But here we emulate original logic logic for safety, just safer memory usage.

  uint32_t bytes_read = 0;
  while (bytes_read < total_bytes) {
    uint32_t to_read = std::min(static_cast<uint32_t>(buffer.size()), total_bytes - bytes_read);
    _usb.Read(to_read, kAddr_DATA, buffer.data());
    bytes_read += to_read;
  }
}

void NKFADC125S::AlignADC() const
{
  uint32_t nch = ReadCHNUM();
  bool is_small_module = (nch > 4) ? false : true;

  SendADCRST();
  SleepMs(1000);
  SendADCCAL();

  WriteADCSETUP(0x009, 0x02);

  for (uint32_t ch = 1; ch <= nch; ++ch) {
    WriteADCSETUP(0x0006, 0x02);
    WriteADCSETUP(0x000A, 0x33);
    WriteADCSETUP(0x000B, 0x33);

    WriteADCBITSLIP(ch, 0);

    // 1. Scan Delay
    int sum = 0;
    int count = 0;
    bool flag = false;

    for (uint32_t dly = 0; dly < 32; ++dly) {
      WriteADCDLY(ch, dly);

      uint32_t value = ReadADCSTAT();
      bool bit_okay = (value >> (ch - 1)) & 0x1u;

      if (bit_okay) {
        ++count;
        sum += dly;
        if (count > 5) flag = true;
      }
      else {
        if (flag) break; // End of valid window
        else {
          sum = 0;
          count = 0;
        }
      }
    }

    uint32_t gdly = count ? (sum / count) : 0;
    WriteADCDLY(ch, gdly);

    WriteADCSETUP(0x000E, 0x81);
    WriteADCSETUP(0x000F, 0x00);
    WriteADCSETUP(0x000A, 0x55);
    WriteADCSETUP(0x000B, 0x55);

    // 2. Scan Bitslip
    uint32_t gbitslip = 0;
    flag = false;

    for (uint32_t bitslip = 0; bitslip < 7; ++bitslip) {
      if (!is_small_module) { WriteADCBITSLIP(ch, bitslip); }

      uint32_t value = ReadADCSTAT();
      bool word_okay = (value >> (ch + 15)) & 0x1u;

      if (word_okay) {
        flag = true;
        gbitslip = bitslip;
        break; // Found it
      }
      else {
        // Original logic: For small modules, write bitslip AFTER check fail?
        // This implies checking 'default' first, then applying 0, 1, 2...
        if (is_small_module) { WriteADCBITSLIP(ch, bitslip); }
      }
    }

    if (flag) { INFO("ADC(%u) aligned: delay=%u, bitslip=%u", ch, gdly, gbitslip); }
    else {
      INFO("ADC(%u) alignment FAILED", ch);
    }
  }

  // Finalize
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
  uint32_t nch = ReadCHNUM();
  bool is_small_module = (nch > 4) ? false : true;

  WriteDRAMON(1);

  if (is_small_module) {
    INFO("DRAM is aligned (Skipped for small module)");
    return;
  }

  WriteDRAMTEST(1);
  SendADCCAL();
  WriteDRAMTEST(2);

  for (uint32_t ch = 0; ch < 8; ++ch) {
    int count = 0;
    int sum = 0;
    bool flag = false;

    // 1. Scan Delay
    for (uint32_t dly = 0; dly < 32; ++dly) {
      WriteDRAMDLY(ch, dly);
      WriteDRAMTEST(3);
      uint32_t value = ReadDRAMTEST(ch);

      bool aflag = (value == 0xFFAA5500u || value == 0xAA5500FFu || value == 0x5500FFAAu ||
                    value == 0x00FFAA55u);

      if (aflag) {
        ++count;
        sum += dly;
        if (count > 4) flag = true;
      }
      else {
        if (flag) break;
        else {
          count = 0;
          sum = 0;
        }
      }
    }

    uint32_t gdly = count ? (sum / count) : 9;
    WriteDRAMDLY(ch, gdly);

    // 2. Scan Bitslip
    bool aligned = false;
    int gbitslip = 0;

    for (int bitslip = 0; bitslip < 4; ++bitslip) {
      WriteDRAMTEST(3);
      uint32_t value = ReadDRAMTEST(ch);

      if (value == 0xFFAA5500u) {
        aligned = true;
        gbitslip = bitslip;
        break;
      }
      else {
        WriteDRAMBITSLIP(ch);
      }
    }

    if (aligned) { INFO("DRAM(%u) aligned: delay=%u, bitslip=%d", ch, gdly, gbitslip); }
    else {
      INFO("DRAM(%u) align FAILED", ch);
    }
  }

  WriteDRAMTEST(0);
}