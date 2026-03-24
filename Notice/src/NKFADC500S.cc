#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <vector>

#include "DAQUtils/ELog.hh"
#include "Notice/NKFADC500S.hh"

namespace {

constexpr std::uint16_t kVENDOR_ID = 0x0547;
constexpr std::uint16_t kPRODUCT_ID = 0x1502;

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
constexpr uint32_t kAddr_AMODE = 0x2000000Bu;
constexpr uint32_t kAddr_PCT = 0x2000000Cu;
constexpr uint32_t kAddr_PCI = 0x2000000Du;
constexpr uint32_t kAddr_PWT = 0x2000000Eu;
constexpr uint32_t kAddr_DT = 0x2000000Fu;

constexpr uint32_t kAddr_BCOUNT = 0x20000010u;
constexpr uint32_t kAddr_PTRIG = 0x20000011u;
constexpr uint32_t kAddr_TRIG_CMD = 0x20000012u;
constexpr uint32_t kAddr_TRIG_ENABLE = 0x20000013u;
constexpr uint32_t kAddr_TM = 0x20000014u;
constexpr uint32_t kAddr_TLT = 0x20000015u;
constexpr uint32_t kAddr_ZEROSUP = 0x20000016u;
constexpr uint32_t kAddr_ADCRST = 0x20000017u;
constexpr uint32_t kAddr_ADCCAL = 0x20000018u;
constexpr uint32_t kAddr_ADCDLY = 0x20000019u;
constexpr uint32_t kAddr_ADCALIGN = 0x2000001Au; // Also ADCSTAT
constexpr uint32_t kAddr_DRAMDLY = 0x2000001Bu;
constexpr uint32_t kAddr_DRAMBITS = 0x2000001Cu;
constexpr uint32_t kAddr_DRAMTEST = 0x2000001Du;
constexpr uint32_t kAddr_PSCALE = 0x2000001Eu;
constexpr uint32_t kAddr_DSR = 0x2000001Fu;

constexpr uint32_t kAddr_DATA = 0x40000000u;

// Helper for channel address calculation: Base + ((ch-1) << 16)
inline uint32_t GetChAddr(uint32_t base, uint32_t ch) { return base + (((ch - 1u) & 0xFFu) << 16); }

// For zero-based channel indices (DRAM operations)
inline uint32_t GetChAddrZB(uint32_t base, uint32_t ch) { return base + ((ch & 0xFFu) << 16); }

void SleepMs(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
} // namespace

NKFADC500S::NKFADC500S(int sid)
  : _sid(sid),
    _usb(kVENDOR_ID, kPRODUCT_ID, sid)
{
}

NKFADC500S::~NKFADC500S() { Close(); }

void NKFADC500S::SetSID(int sid)
{
  _sid = sid;
  _usb.Set(kVENDOR_ID, kPRODUCT_ID, _sid);
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

void NKFADC500S::Reset() const { _usb.Write(kAddr_RUN, 1u << 2); }
void NKFADC500S::ResetTimer() const { _usb.Write(kAddr_RUN, 1u); }
void NKFADC500S::Start() const { _usb.Write(kAddr_RUN, 1u << 3); }
void NKFADC500S::Stop() const { _usb.Write(kAddr_RUN, 0u); }

uint32_t NKFADC500S::ReadRun() const { return _usb.ReadReg(kAddr_RUN); }

void NKFADC500S::WriteCW(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_CW, ch), data);
}
uint32_t NKFADC500S::ReadCW(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_CW, ch)); }

void NKFADC500S::WriteRL(uint32_t data) const { _usb.Write(kAddr_RL, data); }
uint32_t NKFADC500S::ReadRL() const { return _usb.ReadReg(kAddr_RL); }

void NKFADC500S::WriteDRAMON(uint32_t data) const
{
  if (data) {
    uint32_t status = _usb.ReadReg(kAddr_DRAMON);
    if (status) { _usb.Write(kAddr_DRAMON, 0u); }

    _usb.Write(kAddr_DRAMON, 1u);

    // Added timeout to avoid infinite loop
    int timeout = 1000;
    while (_usb.ReadReg(kAddr_DRAMON) == 0 && timeout > 0) {
      timeout--;
    }
    if (timeout == 0) ERROR("NKFADC500S: DRAMON write timeout");
  }
  else {
    _usb.Write(kAddr_DRAMON, 0u);
  }
}

uint32_t NKFADC500S::ReadDRAMON() const { return _usb.ReadReg(kAddr_DRAMON); }

void NKFADC500S::WriteDACOFF(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_DACOFF, ch), data);
}
uint32_t NKFADC500S::ReadDACOFF(uint32_t ch) const
{
  return _usb.ReadReg(GetChAddr(kAddr_DACOFF, ch));
}

void NKFADC500S::MeasurePED(uint32_t ch) const { _usb.Write(GetChAddr(kAddr_PED_CMD, ch), 0u); }
uint32_t NKFADC500S::ReadPED(uint32_t ch) const
{
  return _usb.ReadReg(GetChAddr(kAddr_PED_READ, ch));
}

void NKFADC500S::WriteDLY(uint32_t ch, uint32_t data) const
{
  uint32_t value = ((data / 1000u) << 10) | (data % 1000u);
  _usb.Write(GetChAddr(kAddr_DLY, ch), value);
}

uint32_t NKFADC500S::ReadDLY(uint32_t ch) const
{
  uint32_t value = _usb.ReadReg(GetChAddr(kAddr_DLY, ch));
  return (value >> 10) * 1000u + (value & 0x3FFu);
}

void NKFADC500S::WriteTHR(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_THR, ch), data);
}
uint32_t NKFADC500S::ReadTHR(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_THR, ch)); }

void NKFADC500S::WritePOL(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_POL, ch), data);
}
uint32_t NKFADC500S::ReadPOL(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_POL, ch)); }

void NKFADC500S::WritePSW(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_PSW, ch), data);
}
uint32_t NKFADC500S::ReadPSW(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_PSW, ch)); }

void NKFADC500S::WriteAMODE(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_AMODE, ch), data);
}
uint32_t NKFADC500S::ReadAMODE(uint32_t ch) const
{
  return _usb.ReadReg(GetChAddr(kAddr_AMODE, ch));
}

void NKFADC500S::WritePCT(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_PCT, ch), data);
}
uint32_t NKFADC500S::ReadPCT(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_PCT, ch)); }

void NKFADC500S::WritePCI(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_PCI, ch), data);
}
uint32_t NKFADC500S::ReadPCI(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_PCI, ch)); }

void NKFADC500S::WritePWT(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_PWT, ch), data);
}
uint32_t NKFADC500S::ReadPWT(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_PWT, ch)); }

void NKFADC500S::WriteDT(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_DT, ch), data);
}
uint32_t NKFADC500S::ReadDT(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_DT, ch)); }

void NKFADC500S::WritePTRIG(uint32_t data) const { _usb.Write(kAddr_PTRIG, data); }
uint32_t NKFADC500S::ReadPTRIG() const { return _usb.ReadReg(kAddr_PTRIG); }

void NKFADC500S::SendTRIG() const { _usb.Write(kAddr_TRIG_CMD, 0u); }

void NKFADC500S::WriteTRIGENABLE(uint32_t data) const { _usb.Write(kAddr_TRIG_ENABLE, data); }
uint32_t NKFADC500S::ReadTRIGENABLE() const { return _usb.ReadReg(kAddr_TRIG_ENABLE); }

void NKFADC500S::WriteTM(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_TM, ch), data);
}
uint32_t NKFADC500S::ReadTM(uint32_t ch) const { return _usb.ReadReg(GetChAddr(kAddr_TM, ch)); }

void NKFADC500S::WriteTLT(uint32_t data) const { _usb.Write(kAddr_TLT, data); }
uint32_t NKFADC500S::ReadTLT() const { return _usb.ReadReg(kAddr_TLT); }

void NKFADC500S::WriteZEROSUP(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_ZEROSUP, ch), data);
}
uint32_t NKFADC500S::ReadZEROSUP(uint32_t ch) const
{
  return _usb.ReadReg(GetChAddr(kAddr_ZEROSUP, ch));
}

void NKFADC500S::SendADCRST() const { _usb.Write(kAddr_ADCRST, 0u); }
void NKFADC500S::SendADCCAL() const { _usb.Write(kAddr_ADCCAL, 0u); }

void NKFADC500S::WriteADCDLY(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddr(kAddr_ADCDLY, ch), data);
}
void NKFADC500S::WriteADCALIGN(uint32_t data) const { _usb.Write(kAddr_ADCALIGN, data); }
uint32_t NKFADC500S::ReadADCSTAT() const { return _usb.ReadReg(kAddr_ADCALIGN); }

void NKFADC500S::WriteDRAMDLY(uint32_t ch, uint32_t data) const
{
  _usb.Write(GetChAddrZB(kAddr_DRAMDLY, ch), data);
}
void NKFADC500S::WriteDRAMBITSLIP(uint32_t ch) const
{
  _usb.Write(GetChAddrZB(kAddr_DRAMBITS, ch), 0u);
}

void NKFADC500S::WriteDRAMTEST(uint32_t data) const { _usb.Write(kAddr_DRAMTEST, data); }
uint32_t NKFADC500S::ReadDRAMTEST(uint32_t ch) const
{
  return _usb.ReadReg(GetChAddrZB(kAddr_DRAMTEST, ch));
}

void NKFADC500S::WritePSCALE(uint32_t data) const { _usb.Write(kAddr_PSCALE, data); }
uint32_t NKFADC500S::ReadPSCALE() const { return _usb.ReadReg(kAddr_PSCALE); }

void NKFADC500S::WriteDSR(uint32_t data) const { _usb.Write(kAddr_DSR, data); }
uint32_t NKFADC500S::ReadDSR() const { return _usb.ReadReg(kAddr_DSR); }

int NKFADC500S::ReadBCount() const { return static_cast<int>(_usb.ReadReg(kAddr_BCOUNT)); }

int NKFADC500S::ReadData(int bcount, unsigned char * data, unsigned int timeout) const
{
  if (bcount <= 0 || data == nullptr) {
    ERROR("NKFADC500S [sid=%d]: invalid ReadData arguments", _sid);
    return -1;
  }
  uint32_t count = static_cast<uint32_t>(bcount) * 256;
  return _usb.Read(count, kAddr_DATA, data, timeout);
}

void NKFADC500S::FlushData() const
{
  int bcount = ReadBCount();
  if (bcount <= 0) return;

  constexpr int kChunkSize = 10240 * 256; // 2.6 MB
  std::vector<unsigned char> buffer(kChunkSize);

  uint32_t total_bytes = static_cast<uint32_t>(bcount) * 256;
  uint32_t bytes_read = 0;

  while (bytes_read < total_bytes) {
    uint32_t to_read = std::min(static_cast<uint32_t>(buffer.size()), total_bytes - bytes_read);
    _usb.Read(to_read, kAddr_DATA, buffer.data());
    bytes_read += to_read;
  }
}

void NKFADC500S::AlignADC() const
{
  SendADCRST();
  SleepMs(500);
  SendADCCAL();
  WriteADCALIGN(1);

  for (uint32_t ch = 1; ch <= 4; ++ch) {
    int count = 0;
    int sum = 0;
    bool flag = false;

    // Scan Delay
    for (uint32_t dly = 0; dly < 32; ++dly) {
      WriteADCDLY(ch, dly);
      uint32_t value = (ReadADCSTAT() >> (ch - 1)) & 0x1u;

      if (!value) {
        flag = true;
        ++count;
        sum += dly;
      }
      else if (flag) {
        break; // End of valid window
      }
    }

    uint32_t center = count ? (sum / count) : 0;
    uint32_t gdly = (center < 11) ? (center + 11) : (center - 11);

    WriteADCDLY(ch, gdly);
    INFO("NKFADC500S ADC(%u) calibration delay = %u", ch, gdly);
  }

  WriteADCALIGN(0);
  SendADCCAL();
}

void NKFADC500S::AlignDRAM() const
{
  WriteDRAMON(1);
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

      // Check alignment pattern
      bool pattern_ok = (value == 0xFFAA5500u || value == 0xAA5500FFu || value == 0x5500FFAAu ||
                         value == 0x00FFAA55u);

      if (pattern_ok) {
        ++count;
        sum += dly;
        if (count > 4) flag = true;
      }
      else if (flag) {
        break;
      }
      else {
        count = 0;
        sum = 0;
      }
    }

    uint32_t gdly = count ? (sum / count) : 9;
    WriteDRAMDLY(ch, gdly);

    // 2. Scan Bitslip
    bool aligned = false;
    uint32_t gbitslip = 0;

    for (uint32_t bitslip = 0; bitslip < 4; ++bitslip) {
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

    if (aligned) { INFO("NKFADC500S DRAM(%u) aligned: delay=%u, bitslip=%u", ch, gdly, gbitslip); }
    else {
      INFO("NKFADC500S: DRAM(%u) alignment FAILED", ch);
    }
  }

  WriteDRAMTEST(0);
}