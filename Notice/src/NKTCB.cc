#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <thread>
#include <unistd.h>

#include "DAQUtils/ELog.hh"
#include "Notice/NKTCB.hh"

namespace {
constexpr std::uint16_t kTCB_VENDOR_ID = 0x0547;
constexpr std::uint16_t kTCB_PRODUCT_ID = 0x1501;

// Register Addresses (Base)
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
constexpr uint32_t kAddr_PSS = 0x20000010u;
constexpr uint32_t kAddr_RT = 0x20000011u;
constexpr uint32_t kAddr_SR = 0x20000012u;
constexpr uint32_t kAddr_DACGAIN = 0x20000013u;
constexpr uint32_t kAddr_TM = 0x20000014u;
constexpr uint32_t kAddr_TLT = 0x20000015u;
constexpr uint32_t kAddr_ZEROSUP = 0x20000016u;
constexpr uint32_t kAddr_ADCRST = 0x20000017u;
constexpr uint32_t kAddr_ADCCAL = 0x20000018u;
constexpr uint32_t kAddr_ADCDLY = 0x20000019u;
constexpr uint32_t kAddr_ADCALIGN = 0x2000001Au;
constexpr uint32_t kAddr_ADCSTAT = 0x2000001Au; // Read
constexpr uint32_t kAddr_BITSLIP = 0x2000001Bu;
constexpr uint32_t kAddr_FMUX = 0x2000001Cu;
constexpr uint32_t kAddr_ARMFADC = 0x2000001Du;
constexpr uint32_t kAddr_ZSFD = 0x2000001Eu;
constexpr uint32_t kAddr_DSR = 0x2000001Fu;
constexpr uint32_t kAddr_ST = 0x20000020u;
constexpr uint32_t kAddr_PT = 0x20000021u;
constexpr uint32_t kAddr_DRAMDLY = 0x20000022u;
constexpr uint32_t kAddr_DRAMBITS = 0x20000023u;
constexpr uint32_t kAddr_DRAMTEST = 0x20000024u;
constexpr uint32_t kAddr_DAQMODE = 0x20000025u;
constexpr uint32_t kAddr_HV = 0x20000026u;
constexpr uint32_t kAddr_TEMP = 0x20000027u;
constexpr uint32_t kAddr_ADCMUX = 0x20000028u;

// Master specific addresses (when mid=0)
constexpr uint32_t kAddr_Master_DRAMON = 0x2000003Eu;
constexpr uint32_t kAddr_Master_DRAMTEST = 0x2000003Fu;
constexpr uint32_t kAddr_Master_ADCCAL = 0x20000040u;
constexpr uint32_t kAddr_Master_DRAMDLY = 0x20000041u;
constexpr uint32_t kAddr_Master_DRAMBITS = 0x20000042u;

void SleepMs(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
} // namespace

NKTCB::NKTCB()
  : _usb(kTCB_VENDOR_ID, kTCB_PRODUCT_ID, 0)
{
}

NKTCB::~NKTCB() { Close(); }

// Helper: Calculate register address based on MID and Channel
uint32_t NKTCB::GetAddr(uint32_t baseAddr, uint32_t mid, uint32_t ch) const
{
  if (mid == 0) return baseAddr; // Master access often uses base directly
  // Common formula: Base + ((ch-1) << 16)
  return baseAddr + static_cast<uint32_t>(((ch - 1u) & 0xFFu) << 16);
}

int NKTCB::Open()
{
  int status = _usb.Open();
  if (status < 0) {
    ERROR("failed to open TCB device");
    return status;
  }
  status = _usb.ClaimInterface(0);
  if (status < 0) {
    ERROR("failed to claim TCB interface 0");
    _usb.Close();
    return status;
  }
  return 0;
}

void NKTCB::Close()
{
  _usb.ReleaseInterface(0);
  _usb.Close();
}

void NKTCB::WriteLT(uint32_t mid, const char * data, int len)
{
  const int nword = len / 4;
  uint32_t buffer = 0;
  for (int loop = 0; loop < nword; ++loop) {
    std::memcpy(&buffer, data + loop * 4, 4);
    _usb.Write(mid, static_cast<uint32_t>(loop), buffer);
  }
}

void NKTCB::Reset() { _usb.Write(0, kAddr_RUN, 1u << 2); }
void NKTCB::ResetTimer() { _usb.Write(0, kAddr_RUN, 1u); }
void NKTCB::Start() { _usb.Write(0, kAddr_RUN, 1u << 3); }
void NKTCB::Stop() { _usb.Write(0, kAddr_RUN, 0u); }

uint32_t NKTCB::ReadRUN(uint32_t mid) const { return _usb.ReadReg(mid, kAddr_RUN); }

void NKTCB::WriteCW(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_CW, mid, ch), data);
}
uint32_t NKTCB::ReadCW(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_CW, mid, ch));
}

void NKTCB::WriteRL(uint32_t mid, uint32_t data) const { _usb.Write(mid, kAddr_RL, data); }
uint32_t NKTCB::ReadRL(uint32_t mid) const { return _usb.ReadReg(mid, kAddr_RL); }

void NKTCB::WriteGW(uint32_t mid, uint32_t data) const { _usb.Write(mid, kAddr_RL, data); }
uint32_t NKTCB::ReadGW(uint32_t mid) const { return _usb.ReadReg(mid, kAddr_RL); }

void NKTCB::WriteDRAMON(uint32_t mid, uint32_t data) const
{
  uint32_t addr = (mid > 0) ? kAddr_DRAMON : kAddr_Master_DRAMON;

  if (data) {
    uint32_t status = _usb.ReadReg(mid, addr);
    if (status) { _usb.Write(mid, addr, 0u); }
    _usb.Write(mid, addr, 1u);

    // Polling with timeout to prevent infinite loop
    int timeout = 1000;
    while (_usb.ReadReg(mid, addr) == 0 && timeout > 0) {
      timeout--;
    }
    if (timeout <= 0) ERROR("WriteDRAMON timeout on mid %u", mid);
  }
  else {
    _usb.Write(mid, addr, 0u);
  }
}

uint32_t NKTCB::ReadDRAMON(uint32_t mid) const
{
  return _usb.ReadReg(mid, (mid > 0) ? kAddr_DRAMON : kAddr_Master_DRAMON);
}

void NKTCB::WriteDACOFF(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_DACOFF, mid, ch), data);
}
uint32_t NKTCB::ReadDACOFF(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_DACOFF, mid, ch));
}

void NKTCB::MeasurePED(uint32_t mid, uint32_t ch) const
{
  _usb.Write(mid, GetAddr(kAddr_PED_CMD, mid, ch), 0u);
}
uint32_t NKTCB::ReadPED(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_PED_READ, mid, ch));
}

void NKTCB::WriteDLY(uint32_t mid, uint32_t ch, uint32_t data) const
{
  uint32_t value = ((data / 1000u) << 10) | (data % 1000u);
  _usb.Write(mid, GetAddr(kAddr_DLY, mid, ch), value);
}
uint32_t NKTCB::ReadDLY(uint32_t mid, uint32_t ch) const
{
  uint32_t value = _usb.ReadReg(mid, GetAddr(kAddr_DLY, mid, ch));
  return (value >> 10) * 1000u + (value & 0x3FFu);
}

void NKTCB::WriteAMOREDLY(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_DLY, mid, ch), data);
}
uint32_t NKTCB::ReadAMOREDLY(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_DLY, mid, ch));
}

// ... Repeating the pattern for simple Read/Write registers ...

void NKTCB::WriteTHR(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_THR, mid, ch), data);
}
uint32_t NKTCB::ReadTHR(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_THR, mid, ch));
}

void NKTCB::WritePOL(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_POL, mid, ch), data);
}
uint32_t NKTCB::ReadPOL(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_POL, mid, ch));
}

void NKTCB::WritePSW(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_PSW, mid, ch), data);
}
uint32_t NKTCB::ReadPSW(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_PSW, mid, ch));
}

void NKTCB::WriteAMODE(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_AMODE, mid, ch), data);
}
uint32_t NKTCB::ReadAMODE(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_AMODE, mid, ch));
}

void NKTCB::WritePCT(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_PCT, mid, ch), data);
}
uint32_t NKTCB::ReadPCT(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_PCT, mid, ch));
}

void NKTCB::WritePCI(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_PCI, mid, ch), data);
}
uint32_t NKTCB::ReadPCI(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_PCI, mid, ch));
}

void NKTCB::WritePWT(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_PWT, mid, ch), data);
}
uint32_t NKTCB::ReadPWT(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_PWT, mid, ch));
}

void NKTCB::WritePSS(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_PSS, mid, ch), data);
}
uint32_t NKTCB::ReadPSS(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_PSS, mid, ch));
}

void NKTCB::WriteRT(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_RT, mid, ch), data);
}
uint32_t NKTCB::ReadRT(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_RT, mid, ch));
}

void NKTCB::WriteSR(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_SR, mid, ch), data);
}
uint32_t NKTCB::ReadSR(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_SR, mid, ch));
}

void NKTCB::WriteDACGain(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_DACGAIN, mid, ch), data);
}
uint32_t NKTCB::ReadDACGain(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_DACGAIN, mid, ch));
}

void NKTCB::WriteTM(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_TM, mid, ch), data);
}
uint32_t NKTCB::ReadTM(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_TM, mid, ch));
}

void NKTCB::WriteTLT(uint32_t mid, uint32_t data) const { _usb.Write(mid, kAddr_TLT, data); }
uint32_t NKTCB::ReadTLT(uint32_t mid) const { return _usb.ReadReg(mid, kAddr_TLT); }

void NKTCB::WriteSTLT(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_TLT, mid, ch), data);
}
uint32_t NKTCB::ReadSTLT(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_TLT, mid, ch));
}

void NKTCB::WriteZEROSUP(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_ZEROSUP, mid, ch), data);
}
uint32_t NKTCB::ReadZEROSUP(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_ZEROSUP, mid, ch));
}

void NKTCB::SendADCRST(uint32_t mid) const { _usb.Write(mid, kAddr_ADCRST, 0u); }
void NKTCB::SendADCCAL(uint32_t mid) const
{
  _usb.Write(mid, (mid > 0 ? kAddr_ADCCAL : kAddr_Master_ADCCAL), 0u);
}

void NKTCB::WriteADCDLY(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_ADCDLY, mid, ch), data);
}
void NKTCB::WriteADCALIGN(uint32_t mid, uint32_t data) const
{
  _usb.Write(mid, kAddr_ADCALIGN, data);
}
uint32_t NKTCB::ReadADCSTAT(uint32_t mid) const { return _usb.ReadReg(mid, kAddr_ADCSTAT); }

void NKTCB::WriteBITSLIP(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_BITSLIP, mid, ch), data);
}

void NKTCB::WriteFMUX(uint32_t mid, uint32_t ch) const { _usb.Write(mid, kAddr_FMUX, ch - 1u); }
uint32_t NKTCB::ReadFMUX(uint32_t mid) const { return _usb.ReadReg(mid, kAddr_FMUX); }

void NKTCB::ArmFADC(uint32_t mid) const { _usb.Write(mid, kAddr_ARMFADC, 0u); }
uint32_t NKTCB::ReadFREADY(uint32_t mid) const { return _usb.ReadReg(mid, kAddr_ARMFADC); }

void NKTCB::WriteZSFD(uint32_t mid, uint32_t data) const { _usb.Write(mid, kAddr_ZSFD, data); }
uint32_t NKTCB::ReadZSFD(uint32_t mid) const { return _usb.ReadReg(mid, kAddr_ZSFD); }

void NKTCB::WriteDSR(uint32_t mid, uint32_t data) const { _usb.Write(mid, kAddr_DSR, data); }
uint32_t NKTCB::ReadDSR(uint32_t mid) const { return _usb.ReadReg(mid, kAddr_DSR); }

void NKTCB::WriteST(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_ST, mid, ch), data);
}
uint32_t NKTCB::ReadST(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_ST, mid, ch));
}

void NKTCB::WritePT(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_PT, mid, ch), data);
}
uint32_t NKTCB::ReadPT(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_PT, mid, ch));
}

void NKTCB::WriteDRAMDLY(uint32_t mid, uint32_t ch, uint32_t data) const
{
  uint32_t base = (mid > 0) ? kAddr_DRAMDLY : kAddr_Master_DRAMDLY;
  _usb.Write(mid, base + ((ch & 0xFFu) << 16), data);
}

void NKTCB::WriteDRAMBITSLIP(uint32_t mid, uint32_t ch) const
{
  uint32_t base = (mid > 0) ? kAddr_DRAMBITS : kAddr_Master_DRAMBITS;
  _usb.Write(mid, base + ((ch & 0xFFu) << 16), 0u);
}

void NKTCB::WriteDRAMTEST(uint32_t mid, uint32_t data) const
{
  _usb.Write(mid, (mid > 0) ? kAddr_DRAMTEST : kAddr_Master_DRAMTEST, data);
}

uint32_t NKTCB::ReadDRAMTEST(uint32_t mid, uint32_t ch) const
{
  uint32_t base = (mid > 0) ? kAddr_DRAMTEST : kAddr_Master_DRAMTEST;
  return _usb.ReadReg(mid, base + ((ch & 0xFFu) << 16));
}

void NKTCB::WriteDAQMODE(uint32_t mid, uint32_t data) const
{
  _usb.Write(mid, kAddr_DAQMODE, data);
}
uint32_t NKTCB::ReadDAQMODE(uint32_t mid) const { return _usb.ReadReg(mid, kAddr_DAQMODE); }

void NKTCB::WriteHV(uint32_t mid, uint32_t ch, float data) const
{
  float fval = 4.49f * (data - 3.2f);
  int value = static_cast<int>(fval);
  value = std::clamp(value, 0, 254);
  _usb.Write(mid, GetAddr(kAddr_HV, mid, ch), static_cast<uint32_t>(value));
}

float NKTCB::ReadHV(uint32_t mid, uint32_t ch) const
{
  uint32_t data = _usb.ReadReg(mid, GetAddr(kAddr_HV, mid, ch));
  return static_cast<float>(data) / 4.49f + 3.2f;
}

float NKTCB::ReadTEMP(uint32_t mid, uint32_t ch) const
{
  uint32_t data = _usb.ReadReg(mid, GetAddr(kAddr_TEMP, mid, ch));
  float value = static_cast<float>(data) * 250.0f / 4096.0f - 50.0f;
  return value * 5.7f / 4.7f;
}

void NKTCB::WriteADCMUX(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_ADCMUX, mid, ch), data);
}

void NKTCB::ReadFADCBUF(uint32_t mid, uint32_t * data) const
{
  uint32_t addr = 0x20008000u;
  for (uint32_t i = 0; i < 2048u; ++i) {
    data[i] = _usb.ReadReg(mid, addr + i);
  }
}

// --------------------------------------------------------------------------
// Complex Alignment Functions
// --------------------------------------------------------------------------

void NKTCB::AlignFADC500(uint32_t mid) const
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
        ++count;
        sum += dly;
      }
      else if (flag) {
        break; // End of valid window
      }
    }

    int center = (count ? sum / count : 0);
    uint32_t gdly = (center < 11) ? (center + 11) : (center - 11);

    WriteADCDLY(mid, ch, gdly);
    INFO("ch%u calibration delay = %u", ch, gdly);
  }

  WriteADCALIGN(mid, 0);
  SendADCCAL(mid);
}

void NKTCB::AlignSADC64(uint32_t mid) const
{
  SendADCRST(mid);
  SleepMs(500);
  SendADCCAL(mid);

  // Initial sequence
  WriteADCALIGN(mid, 0x030002u);
  SleepMs(1);
  WriteADCALIGN(mid, 0x010010u);
  SleepMs(1);
  WriteADCALIGN(mid, 0xC78001u);
  SleepMs(1);
  WriteADCALIGN(mid, 0xDE01C0u);
  SleepMs(1);

  for (uint32_t ch = 1; ch <= 4; ++ch) {
    WriteADCALIGN(mid, 0x450001u);
    WriteBITSLIP(mid, ch, 0);

    // 1. Scan Delay
    int count = 0;
    int sum = 0;
    bool flag = false;

    for (uint32_t dly = 0; dly < 32; ++dly) {
      WriteADCDLY(mid, ch, dly);
      uint32_t value = (ReadADCSTAT(mid) >> (ch - 1)) & 0x1u;

      if (!value) {
        flag = true;
        ++count;
        sum += dly;
      }
      else if (flag) {
        break;
      }
    }

    int center = (count ? sum / count : 0);
    uint32_t gdly = (center < 9) ? (center + 9) : (center - 9);
    WriteADCDLY(mid, ch, gdly);

    WriteADCALIGN(mid, 0x450002u);
    SleepMs(1);

    // 2. Scan Bitslip
    uint32_t gbitslip = 0;
    for (uint32_t bitslip = 0; bitslip < 12; ++bitslip) {
      WriteBITSLIP(mid, ch, bitslip);
      uint32_t value = (ReadADCSTAT(mid) >> (ch + 3)) & 0x1u; // (ch-1)+4 = ch+3
      if (value) {
        gbitslip = bitslip;
        break;
      }
    }

    WriteBITSLIP(mid, ch, gbitslip);
    INFO("ch%u calibration delay = %u, bitslip = %u", ch, gdly, gbitslip);
  }

  WriteADCALIGN(mid, 0x450000u);
  SleepMs(1);
  SendADCCAL(mid);
}

void NKTCB::AlignFADC125(uint32_t mid) const
{
  SendADCRST(mid);
  SleepMs(1000);
  SendADCCAL(mid);

  WriteADCALIGN(mid, 0x00902u);
  WriteADCALIGN(mid, 0x70A01u);

  for (uint32_t ch = 1; ch <= 16; ++ch) {
    WriteADCALIGN(mid, 0x000602u);
    WriteADCALIGN(mid, 0x000A33u);
    WriteADCALIGN(mid, 0x000B33u);
    WriteBITSLIP(mid, ch, 0);

    // 1. Scan Delay
    int sum = 0;
    int count = 0;
    int flag = 0;

    for (uint32_t dly = 0; dly < 32; ++dly) {
      WriteADCDLY(mid, ch, dly);
      uint32_t value = ReadADCSTAT(mid);
      bool bit_okay = (value >> (ch - 1)) & 0x1u;

      if (bit_okay) {
        ++count;
        sum += dly;
        if (count > 5) ++flag;
      }
      else if (flag) {
        break;
      }
      else {
        sum = 0;
        count = 0; // Reset if not continuous
      }
    }

    uint32_t gdly = (count) ? (sum / count) : 0;
    WriteADCDLY(mid, ch, gdly);

    WriteADCALIGN(mid, 0x000E81u);
    WriteADCALIGN(mid, 0x000F00u);
    WriteADCALIGN(mid, 0x000A55u);
    WriteADCALIGN(mid, 0x000B55u);

    // 2. Scan Bitslip
    uint32_t gbitslip = 0;
    flag = 0;
    for (uint32_t bitslip = 0; bitslip < 7; ++bitslip) {
      WriteBITSLIP(mid, ch, bitslip);
      uint32_t value = ReadADCSTAT(mid);
      bool word_okay = (value >> (ch + 15)) & 0x1u;

      if (word_okay) {
        ++flag;
        gbitslip = bitslip;
        break;
      }
    }

    if (flag > 0) { INFO("ADC(%u) aligned: delay=%u, bitslip=%u", ch, gdly, gbitslip); }
    else {
      INFO("ADC(%u) alignment FAILED", ch);
    }
  }

  // Post-alignment cleanup
  WriteADCALIGN(mid, 0x12202u);
  WriteADCALIGN(mid, 0x22202u);
  WriteADCALIGN(mid, 0x42202u);
  WriteADCALIGN(mid, 0x52202u);

  WriteADCALIGN(mid, 0x000901u);
  WriteADCALIGN(mid, 0x000600u);
  WriteADCALIGN(mid, 0x000A00u);
  WriteADCALIGN(mid, 0x000B00u);
}

void NKTCB::AlignIADC64(uint32_t mid) const
{
  SendADCRST(mid);
  SleepMs(500);
  SendADCCAL(mid);

  WriteADCALIGN(mid, 0x030002u);
  WriteADCALIGN(mid, 0x010010u);
  WriteADCALIGN(mid, 0xC78001u);
  WriteADCALIGN(mid, 0xDE01C0u);

  for (uint32_t ch = 1; ch <= 5; ++ch) {
    WriteADCALIGN(mid, 0x450001u);

    int sum = 0;
    int count = 0;
    int flag = 0;

    for (uint32_t dly = 0; dly < 32; ++dly) {
      WriteADCDLY(mid, ch, dly);
      bool value = (ReadADCSTAT(mid) >> (ch - 1)) & 0x1u;

      if (value) {
        ++count;
        sum += dly;
        if (count > 8) ++flag;
      }
      else if (flag) {
        break;
      }
      else {
        sum = 0;
        count = 0;
      }
    }

    uint32_t gdly = (count) ? (sum / count) : 0;
    WriteADCDLY(mid, ch, gdly);
    WriteADCALIGN(mid, 0x450002u);

    uint32_t gbitslip = 0;
    flag = 0;
    for (uint32_t bitslip = 0; bitslip < 12; ++bitslip) {
      WriteADCMUX(mid, ch, (bitslip > 5) ? 1 : 0);

      bool value = (ReadADCSTAT(mid) >> (ch + 7)) & 0x1u; // (ch-1)+8 = ch+7
      if (value) {
        ++flag;
        gbitslip = bitslip;
        break;
      }
      else {
        WriteBITSLIP(mid, ch, 0);
      }
    }

    if (flag > 0) { INFO("ch%u calibration delay = %u, bitslip = %u", ch, gdly, gbitslip); }
    else {
      INFO("ch%u fail to align ADC", ch);
    }
  }

  WriteADCALIGN(mid, 0x450000u);
  SleepMs(100);
  SendADCCAL(mid);
}

void NKTCB::AlignDRAM(uint32_t mid) const
{
  WriteDRAMON(mid, 1);
  WriteDRAMTEST(mid, 1);
  SendADCCAL(mid);
  WriteDRAMTEST(mid, 2);

  for (uint32_t ch = 0; ch < 8; ++ch) {
    int count = 0;
    int sum = 0;
    bool valid_window = false;

    for (uint32_t dly = 0; dly < 32; ++dly) {
      WriteDRAMDLY(mid, ch, dly);
      WriteDRAMTEST(mid, 3);
      uint32_t value = ReadDRAMTEST(mid, ch);

      bool pattern_ok = (value == 0xFFAA5500u || value == 0xAA5500FFu || value == 0x5500FFAAu ||
                         value == 0x00FFAA55u);

      if (pattern_ok) {
        ++count;
        sum += dly;
        if (count > 4) valid_window = true;
      }
      else if (valid_window) {
        break;
      }
      else {
        count = 0;
        sum = 0;
      }
    }

    uint32_t gdly = (count) ? (sum / count) : 9;
    WriteDRAMDLY(mid, ch, gdly);

    bool bitslip_ok = false;
    uint32_t gbitslip = 0;

    for (uint32_t bitslip = 0; bitslip < 4; ++bitslip) {
      WriteDRAMTEST(mid, 3);
      uint32_t value = ReadDRAMTEST(mid, ch);

      if (value == 0xFFAA5500u) {
        bitslip_ok = true;
        gbitslip = bitslip;
        break;
      }
      else {
        WriteDRAMBITSLIP(mid, ch);
      }
    }

    if (bitslip_ok) { INFO("DRAM(%u) aligned: delay=%u, bitslip=%u", ch, gdly, gbitslip); }
    else {
      INFO("DRAM(%u) align FAILED", ch);
    }
  }

  WriteDRAMTEST(mid, 0);
}

// Global & Misc
void NKTCB::WriteRUNNO(uint32_t data) const { _usb.Write(0, 0x20000002u, data); }
uint32_t NKTCB::ReadRUNNO() const { return _usb.ReadReg(0, 0x20000002u); }

void NKTCB::WriteGATEDLY(uint32_t data) const { _usb.Write(0, 0x20000003u, data); }
uint32_t NKTCB::ReadGATEDLY() const { return _usb.ReadReg(0, 0x20000003u); }

void NKTCB::SendTRIG() const { _usb.Write(0, 0x20000004u, 0u); }

void NKTCB::ReadLNSTAT(uint32_t * data) const
{
  uint32_t addr = 0x20000005u;
  unsigned char rdat[8] = {0};
  _usb.Read(0, 2u, addr, rdat);

  for (int i = 0; i < 2; ++i) {
    uint32_t v = 0;
    v |= rdat[i * 4 + 0];
    v |= (rdat[i * 4 + 1] << 8);
    v |= (rdat[i * 4 + 2] << 16);
    v |= (rdat[i * 4 + 3] << 24);
    data[i] = v;
  }
}

void NKTCB::ReadMIDS(uint32_t * data) const
{
  uint32_t addr = 0x20000007u;
  unsigned char rdat[160] = {0};
  _usb.Read(0, 40u, addr, rdat);

  for (int i = 0; i < 40; ++i) {
    uint32_t v = 0;
    v |= rdat[i * 4 + 0];
    v |= (rdat[i * 4 + 1] << 8);
    v |= (rdat[i * 4 + 2] << 16);
    v |= (rdat[i * 4 + 3] << 24);
    data[i] = v;
  }
}

// ... Simple wrappers for Trigger/Ext/Gate ...
void NKTCB::WritePTRIG(uint32_t data) const { _usb.Write(0, 0x2000002Fu, data); }
uint32_t NKTCB::ReadPTRIG() const { return _usb.ReadReg(0, 0x2000002Fu); }

void NKTCB::WriteTRIGENABLE(uint32_t mid, uint32_t data) const
{
  _usb.Write(mid, 0x20000030u, data);
}
uint32_t NKTCB::ReadTRIGENABLE(uint32_t mid) const { return _usb.ReadReg(mid, 0x20000030u); }

void NKTCB::WriteEXTOUT(uint32_t data) const { _usb.Write(0, 0x20000039u, data); }
uint32_t NKTCB::ReadEXTOUT() const { return _usb.ReadReg(0, 0x20000039u); }

void NKTCB::WriteGATEWIDTH(uint32_t data) const { _usb.Write(0, 0x2000003Eu, data); }
uint32_t NKTCB::ReadGATEWIDTH() const { return _usb.ReadReg(0, 0x2000003Eu); }

void NKTCB::WriteEXTOUTWIDTH(uint32_t data) const { _usb.Write(0, 0x2000003Fu, data); }
uint32_t NKTCB::ReadEXTOUTWIDTH() const { return _usb.ReadReg(0, 0x2000003Fu); }

uint32_t NKTCB::ReadBCount(uint32_t mid) const { return _usb.ReadReg(mid, 0x30000000u); }

int NKTCB::ReadData(uint32_t mid, uint32_t bcount, unsigned char * data) const
{
  uint32_t count = bcount * 256u;
  return _usb.Read(mid, count, 0x40000000u, data);
}

void NKTCB::WriteGAIN(uint32_t mid, uint32_t ch, uint32_t data) const
{
  _usb.Write(mid, GetAddr(kAddr_DACGAIN, mid, ch), data);
}
uint32_t NKTCB::ReadGAIN(uint32_t mid, uint32_t ch) const
{
  return _usb.ReadReg(mid, GetAddr(kAddr_DACGAIN, mid, ch));
}

void NKTCB::WriteMTHR(uint32_t mid, uint32_t data) const { _usb.Write(mid, 0x20000015u, data); }
uint32_t NKTCB::ReadMTHR(uint32_t mid) const { return _usb.ReadReg(mid, 0x20000015u); }

void NKTCB::WritePSDDLY(uint32_t mid, uint32_t data) const { _usb.Write(mid, 0x20000029u, data); }
uint32_t NKTCB::ReadPSDDLY(uint32_t mid) const { return _usb.ReadReg(mid, 0x20000029u); }

void NKTCB::WritePSDTHR(uint32_t mid, float data) const
{
  _usb.Write(mid, 0x2000002Au, static_cast<uint32_t>(data * 128.0f));
}
float NKTCB::ReadPSDTHR(uint32_t mid) const
{
  return static_cast<float>(_usb.ReadReg(mid, 0x2000002Au)) / 128.0f;
}

uint32_t NKTCB::ReadADCSTATWORD(uint32_t mid) const { return _usb.ReadReg(mid, kAddr_BITSLIP); }

// Simplified DT access (uses different address for Master vs Slave)
void NKTCB::WriteDT(uint32_t mid, uint32_t ch, uint32_t data) const
{
  uint32_t addr;
  if (mid > 0) { addr = 0x2000000Fu + ((ch - 1u) << 16); }
  else {
    addr = 0x2000003Au + ch;
  }
  _usb.Write(mid, addr, data);
}

uint32_t NKTCB::ReadDT(uint32_t mid, uint32_t ch) const
{
  uint32_t addr;
  if (mid > 0) { addr = 0x2000000Fu + ((ch - 1u) << 16); }
  else {
    addr = 0x2000003Au + ch;
  }
  return _usb.ReadReg(mid, addr);
}

// MTHR & PSCALE Wrappers
void NKTCB::WriteMTHRFADC500(uint32_t data) const { _usb.Write(0, 0x20000031u, data); }
uint32_t NKTCB::ReadMTHRFADC500() const { return _usb.ReadReg(0, 0x20000031u); }

void NKTCB::WritePSCALEFADC500(uint32_t data) const { _usb.Write(0, 0x20000032u, data); }
uint32_t NKTCB::ReadPSCALEFADC500() const { return _usb.ReadReg(0, 0x20000032u); }

void NKTCB::WriteMTHRSADC64MU(uint32_t data) const { _usb.Write(0, 0x20000033u, data); }
uint32_t NKTCB::ReadMTHRSADC64MU() const { return _usb.ReadReg(0, 0x20000033u); }

void NKTCB::WritePSCALESADC64MU(uint32_t data) const { _usb.Write(0, 0x20000034u, data); }
uint32_t NKTCB::ReadPSCALESADC64MU() const { return _usb.ReadReg(0, 0x20000034u); }

void NKTCB::WriteMTHRSADC64LS(uint32_t data) const { _usb.Write(0, 0x20000035u, data); }
uint32_t NKTCB::ReadMTHRSADC64LS() const { return _usb.ReadReg(0, 0x20000035u); }

void NKTCB::WritePSCALESADC64LS(uint32_t data) const { _usb.Write(0, 0x20000036u, data); }
uint32_t NKTCB::ReadPSCALESADC64LS() const { return _usb.ReadReg(0, 0x20000036u); }

void NKTCB::WriteMTHRIADC64(uint32_t data) const { _usb.Write(0, 0x20000037u, data); }
uint32_t NKTCB::ReadMTHRIADC64() const { return _usb.ReadReg(0, 0x20000037u); }

void NKTCB::WritePSCALEIADC64(uint32_t data) const { _usb.Write(0, 0x20000038u, data); }
uint32_t NKTCB::ReadPSCALEIADC64() const { return _usb.ReadReg(0, 0x20000038u); }

// Trigger Switches
void NKTCB::WriteTrigSwitchFADC500(uint32_t fadc, uint32_t sadc_mu, uint32_t sadc_ls,
                                   uint32_t iadc) const
{
  uint32_t data = (fadc ? 1 : 0) | (sadc_mu ? 2 : 0) | (sadc_ls ? 4 : 0) | (iadc ? 8 : 0);
  _usb.Write(0, 0x20000043u, data);
}
uint32_t NKTCB::ReadSwitchFADC500() const { return _usb.ReadReg(0, 0x20000043u); }

void NKTCB::WriteTrigSwitchSADC64MU(uint32_t fadc, uint32_t sadc_mu, uint32_t sadc_ls,
                                    uint32_t iadc) const
{
  uint32_t data = (fadc ? 1 : 0) | (sadc_mu ? 2 : 0) | (sadc_ls ? 4 : 0) | (iadc ? 8 : 0);
  _usb.Write(0, 0x20000044u, data);
}
uint32_t NKTCB::ReadSwitchSADC64MU() const { return _usb.ReadReg(0, 0x20000044u); }

void NKTCB::WriteTrigSwitchSADC64LS(uint32_t fadc, uint32_t sadc_mu, uint32_t sadc_ls,
                                    uint32_t iadc) const
{
  uint32_t data = (fadc ? 1 : 0) | (sadc_mu ? 2 : 0) | (sadc_ls ? 4 : 0) | (iadc ? 8 : 0);
  _usb.Write(0, 0x20000045u, data);
}
uint32_t NKTCB::ReadSwitchSADC64LS() const { return _usb.ReadReg(0, 0x20000045u); }

void NKTCB::WriteTrigSwitchIADC64(uint32_t fadc, uint32_t sadc_mu, uint32_t sadc_ls,
                                  uint32_t iadc) const
{
  uint32_t data = (fadc ? 1 : 0) | (sadc_mu ? 2 : 0) | (sadc_ls ? 4 : 0) | (iadc ? 8 : 0);
  _usb.Write(0, 0x20000046u, data);
}
uint32_t NKTCB::ReadSwitchIADC64() const { return _usb.ReadReg(0, 0x20000046u); }

uint32_t NKTCB::ReadDBG() const { return _usb.ReadReg(0, 0x20000050u); }