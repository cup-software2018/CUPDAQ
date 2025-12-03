// NewNotice/NKTCB.cc
#include <cstdint>
#include <cstring>
#include <unistd.h>

#include "DAQUtils/ELog.hh"
#include "NewNotice/NKTCB.hh"

namespace {
constexpr std::uint16_t kTCB_VENDOR_ID = 0x0547;
constexpr std::uint16_t kTCB_PRODUCT_ID = 0x1501;
} // namespace

NKTCB::NKTCB()
  : _usb(kTCB_VENDOR_ID, kTCB_PRODUCT_ID, 0)
{
}

NKTCB::~NKTCB() { Close(); }

int NKTCB::Open()
{
  int status = _usb.Open();
  if (status < 0) {
    ERROR("failed to open device (sid=%d)", 0);
    return status;
  }

  status = _usb.ClaimInterface(0);
  if (status < 0) {
    ERROR("failed to claim interface 0 (sid=%d)", 0);
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

void NKTCB::WriteLT(unsigned long mid, const char * data, int len)
{
  const int nword = len / 4;
  std::uint32_t buffer = 0;
  for (int loop = 0; loop < nword; ++loop) {
    std::memcpy(&buffer, data + loop * 4, 4);
    _usb.Write(mid, static_cast<std::uint32_t>(loop), buffer);
  }
}

void NKTCB::Reset() { _usb.Write(0, 0x20000000u, 1u << 2); }

void NKTCB::ResetTimer() { _usb.Write(0, 0x20000000u, 1u); }

void NKTCB::Start() { _usb.Write(0, 0x20000000u, 1u << 3); }

void NKTCB::Stop() { _usb.Write(0, 0x20000000u, 0u); }

unsigned long NKTCB::ReadRUN(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x20000000u));
}

void NKTCB::WriteCW(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000001u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadCW(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000001u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteRL(unsigned long mid, unsigned long data) const
{
  _usb.Write(mid, 0x20000002u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadRL(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x20000002u));
}

void NKTCB::WriteGW(unsigned long mid, unsigned long data) const
{
  _usb.Write(mid, 0x20000002u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadGW(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x20000002u));
}

void NKTCB::WriteDRAMON(unsigned long mid, unsigned long data) const
{
  std::uint32_t addr = (mid ? 0x20000003u : 0x2000003Eu);
  if (data) {
    unsigned int status = _usb.ReadReg(mid, addr);
    if (status) { _usb.Write(mid, addr, 0u); }
    _usb.Write(mid, addr, 1u);
    status = 0;
    while (!status) {
      status = _usb.ReadReg(mid, addr);
    }
  }
  else {
    _usb.Write(mid, addr, 0u);
  }
}

unsigned long NKTCB::ReadDRAMON(unsigned long mid) const
{
  std::uint32_t addr = (mid ? 0x20000003u : 0x2000003Eu);
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteDACOFF(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000004u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadDACOFF(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000004u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::MeasurePED(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000005u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, 0u);
}

unsigned long NKTCB::ReadPED(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000006u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteDLY(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000007u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  unsigned long value = ((data / 1000UL) << 10) | (data % 1000UL);
  _usb.Write(mid, addr, static_cast<std::uint32_t>(value));
}

unsigned long NKTCB::ReadDLY(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000007u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  unsigned long value = static_cast<unsigned long>(_usb.ReadReg(mid, addr));
  unsigned long data = (value >> 10) * 1000UL + (value & 0x3FFUL);
  return data;
}

void NKTCB::WriteAMOREDLY(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000007u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadAMOREDLY(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000007u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteTHR(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000008u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadTHR(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000008u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WritePOL(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000009u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadPOL(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000009u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WritePSW(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x2000000Au;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadPSW(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x2000000Au;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteAMODE(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x2000000Bu;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadAMODE(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x2000000Bu;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WritePCT(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x2000000Cu;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadPCT(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x2000000Cu;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WritePCI(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x2000000Du;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadPCI(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x2000000Du;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WritePWT(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x2000000Eu;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadPWT(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x2000000Eu;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WritePSS(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000010u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadPSS(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000010u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteRT(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000011u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadRT(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000011u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteSR(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000012u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadSR(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000012u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteDACGain(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000013u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadDACGain(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000013u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteTM(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000014u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadTM(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000014u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteTLT(unsigned long mid, unsigned long data) const
{
  _usb.Write(mid, 0x20000015u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadTLT(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x20000015u));
}

void NKTCB::WriteSTLT(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000015u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadSTLT(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000015u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteZEROSUP(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000016u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadZEROSUP(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000016u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::SendADCRST(unsigned long mid) const { _usb.Write(mid, 0x20000017u, 0u); }

void NKTCB::SendADCCAL(unsigned long mid) const
{
  std::uint32_t addr = (mid > 0 ? 0x20000018u : 0x20000040u);
  _usb.Write(mid, addr, 0u);
}

void NKTCB::WriteADCDLY(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000019u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

void NKTCB::WriteADCALIGN(unsigned long mid, unsigned long data) const
{
  _usb.Write(mid, 0x2000001Au, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadADCSTAT(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x2000001Au));
}

void NKTCB::WriteBITSLIP(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x2000001Bu;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

void NKTCB::WriteFMUX(unsigned long mid, unsigned long ch) const
{
  _usb.Write(mid, 0x2000001Cu, static_cast<std::uint32_t>(ch - 1UL));
}

unsigned long NKTCB::ReadFMUX(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x2000001Cu));
}

void NKTCB::ArmFADC(unsigned long mid) const { _usb.Write(mid, 0x2000001Du, 0u); }

unsigned long NKTCB::ReadFREADY(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x2000001Du));
}

void NKTCB::WriteZSFD(unsigned long mid, unsigned long data) const
{
  _usb.Write(mid, 0x2000001Eu, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadZSFD(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x2000001Eu));
}

void NKTCB::WriteDSR(unsigned long mid, unsigned long data) const
{
  _usb.Write(mid, 0x2000001Fu, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadDSR(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x2000001Fu));
}

void NKTCB::WriteST(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000020u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadST(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000020u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WritePT(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000021u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadPT(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000021u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteDRAMDLY(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = (mid > 0 ? 0x20000022u + static_cast<std::uint32_t>((ch & 0xFFUL) << 16) : 0x20000041u);
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

void NKTCB::WriteDRAMBITSLIP(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = (mid > 0 ? 0x20000023u + static_cast<std::uint32_t>((ch & 0xFFUL) << 16) : 0x20000042u);
  _usb.Write(mid, addr, 0u);
}

void NKTCB::WriteDRAMTEST(unsigned long mid, unsigned long data) const
{
  std::uint32_t addr = (mid > 0 ? 0x20000024u : 0x2000003Fu);
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadDRAMTEST(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = (mid > 0 ? 0x20000024u + static_cast<std::uint32_t>((ch & 0xFFUL) << 16) : 0x2000003Fu);
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteDAQMODE(unsigned long mid, unsigned long data) const
{
  _usb.Write(mid, 0x20000025u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadDAQMODE(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x20000025u));
}

void NKTCB::WriteHV(unsigned long mid, unsigned long ch, float data) const
{
  float fval = 4.49f * (data - 3.2f);
  int value = static_cast<int>(fval);
  if (value > 254) value = 254;
  else if (value < 0) value = 0;

  std::uint32_t addr = 0x20000026u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(value));
}

float NKTCB::ReadHV(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000026u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  unsigned long data = static_cast<unsigned long>(_usb.ReadReg(mid, addr));
  float value = static_cast<float>(data);
  value = value / 4.49f + 3.2f;
  return value;
}

float NKTCB::ReadTEMP(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000027u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  unsigned long data = static_cast<unsigned long>(_usb.ReadReg(mid, addr));
  float value = static_cast<float>(data);
  value = value * 250.0f / 4096.0f - 50.0f;
  value = value * 5.7f / 4.7f;
  return value;
}

void NKTCB::WriteADCMUX(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000028u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

void NKTCB::ReadFADCBUF(unsigned long mid, unsigned long * data) const
{
  std::uint32_t addr = 0x20008000u;
  for (unsigned long i = 0; i < 2048UL; ++i) {
    data[i] = static_cast<unsigned long>(_usb.ReadReg(mid, addr + static_cast<std::uint32_t>(i)));
  }
}

void NKTCB::AlignFADC500(unsigned long mid) const
{
  unsigned long ch;
  unsigned long dly;
  unsigned long value;
  int count;
  int sum;
  int center;
  unsigned long gdly;
  int flag;

  SendADCRST(mid);
  usleep(500000);
  SendADCCAL(mid);
  WriteADCALIGN(mid, 1);

  for (ch = 1; ch <= 4; ++ch) {
    count = 0;
    sum = 0;
    flag = 0;

    for (dly = 0; dly < 32; ++dly) {
      WriteADCDLY(mid, ch, dly);
      value = (ReadADCSTAT(mid) >> (ch - 1)) & 0x1UL;
      if (!value) {
        flag = 1;
        ++count;
        sum += static_cast<int>(dly);
      }
      else {
        if (flag) { dly = 32; }
      }
    }

    center = (count ? sum / count : 0);
    if (center < 11) { gdly = static_cast<unsigned long>(center + 11); }
    else {
      gdly = static_cast<unsigned long>(center - 11);
    }

    WriteADCDLY(mid, ch, gdly);
    INFO("ch%lu calibration delay = %lu", ch, gdly);
  }

  WriteADCALIGN(mid, 0);
  SendADCCAL(mid);
}

void NKTCB::AlignSADC64(unsigned long mid) const
{
  unsigned long ch;
  unsigned long dly;
  unsigned long value;
  int count;
  int sum;
  int center;
  unsigned long bitslip;
  unsigned long gdly;
  unsigned long gbitslip;
  int flag;

  SendADCRST(mid);
  usleep(500000);
  SendADCCAL(mid);

  for (ch = 1; ch <= 4; ++ch) {
    count = 0;
    sum = 0;
    flag = 0;
    gbitslip = 0;

    WriteADCALIGN(mid, 0x030002u);
    usleep(100);
    WriteADCALIGN(mid, 0x010010u);
    usleep(100);
    WriteADCALIGN(mid, 0xC78001u);
    usleep(100);
    WriteADCALIGN(mid, 0xDE01C0u);
    usleep(100);

    WriteADCALIGN(mid, 0x450001u);
    WriteBITSLIP(mid, ch, 0);

    for (dly = 0; dly < 32; ++dly) {
      WriteADCDLY(mid, ch, dly);
      value = (ReadADCSTAT(mid) >> (ch - 1)) & 0x1UL;
      if (!value) {
        flag = 1;
        ++count;
        sum += static_cast<int>(dly);
      }
      else {
        if (flag) { dly = 32; }
      }
    }

    center = (count ? sum / count : 0);
    if (center < 9) { gdly = static_cast<unsigned long>(center + 9); }
    else {
      gdly = static_cast<unsigned long>(center - 9);
    }

    WriteADCDLY(mid, ch, gdly);

    WriteADCALIGN(mid, 0x450002u);
    usleep(100);

    for (bitslip = 0; bitslip < 12; ++bitslip) {
      WriteBITSLIP(mid, ch, bitslip);
      value = (ReadADCSTAT(mid) >> ((ch - 1) + 4)) & 0x1UL;
      if (value) {
        gbitslip = bitslip;
        bitslip = 12;
      }
    }

    WriteBITSLIP(mid, ch, gbitslip);
    INFO("ch%lu calibration delay = %lu, bitslip = %lu", ch, gdly, gbitslip);
  }

  WriteADCALIGN(mid, 0x450000u);
  usleep(100);
  SendADCCAL(mid);
}

void NKTCB::AlignFADC125(unsigned long mid) const
{
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

  SendADCRST(mid);
  usleep(1000000);
  SendADCCAL(mid);

  WriteADCALIGN(mid, 0x00902u);
  WriteADCALIGN(mid, 0x70A01u);

  for (ch = 1; ch <= 16; ++ch) {
    WriteADCALIGN(mid, 0x000602u);
    WriteADCALIGN(mid, 0x000A33u);
    WriteADCALIGN(mid, 0x000B33u);

    WriteBITSLIP(mid, ch, 0);

    sum = 0;
    count = 0;
    flag = 0;
    gbitslip = 0;

    for (dly = 0; dly < 32; ++dly) {
      WriteADCDLY(mid, ch, dly);
      value = ReadADCSTAT(mid);
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

    WriteADCDLY(mid, ch, gdly);

    WriteADCALIGN(mid, 0x000E81u);
    WriteADCALIGN(mid, 0x000F00u);
    WriteADCALIGN(mid, 0x000A55u);
    WriteADCALIGN(mid, 0x000B55u);

    for (bitslip = 0; bitslip < 7; ++bitslip) {
      WriteBITSLIP(mid, ch, bitslip);
      value = ReadADCSTAT(mid);
      word_okay = static_cast<int>((value >> (ch + 15)) & 0x1UL);

      if (word_okay) {
        ++flag;
        gbitslip = bitslip;
        bitslip = 7;
      }
    }

    if (flag > 1) { INFO("ADC(%lu) is aligned, delay = %lu, bitslip = %lu", ch, gdly, gbitslip); }
    else {
      INFO("Fail to align ADC(%lu)!", ch);
    }
  }

  WriteADCALIGN(mid, 0x12202u);
  WriteADCALIGN(mid, 0x22202u);
  WriteADCALIGN(mid, 0x42202u);
  WriteADCALIGN(mid, 0x52202u);

  WriteADCALIGN(mid, 0x000901u);
  WriteADCALIGN(mid, 0x000600u);
  WriteADCALIGN(mid, 0x000A00u);
  WriteADCALIGN(mid, 0x000B00u);
}

void NKTCB::AlignIADC64(unsigned long mid) const
{
  unsigned long ch;
  unsigned long dly;
  unsigned long value;
  int flag;
  int count;
  int sum;
  unsigned long gdly;
  unsigned long bitslip;
  unsigned long gbitslip;

  SendADCRST(mid);
  usleep(500000);
  SendADCCAL(mid);

  WriteADCALIGN(mid, 0x030002u);
  WriteADCALIGN(mid, 0x010010u);
  WriteADCALIGN(mid, 0xC78001u);
  WriteADCALIGN(mid, 0xDE01C0u);

  for (ch = 1; ch <= 5; ++ch) {
    WriteADCALIGN(mid, 0x450001u);

    sum = 0;
    count = 0;
    flag = 0;
    gbitslip = 0;

    for (dly = 0; dly < 32; ++dly) {
      WriteADCDLY(mid, ch, dly);
      value = (ReadADCSTAT(mid) >> (ch - 1)) & 0x1UL;

      if (value) {
        ++count;
        sum += static_cast<int>(dly);
        if (count > 8) { ++flag; }
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

    WriteADCDLY(mid, ch, gdly);
    WriteADCALIGN(mid, 0x450002u);

    for (bitslip = 0; bitslip < 12; ++bitslip) {
      if (bitslip > 5) { WriteADCMUX(mid, ch, 1); }
      else {
        WriteADCMUX(mid, ch, 0);
      }

      value = (ReadADCSTAT(mid) >> ((ch - 1) + 8)) & 0x1UL;
      if (value) {
        ++flag;
        gbitslip = bitslip;
        bitslip = 12;
      }
      else {
        WriteBITSLIP(mid, ch, 0);
      }
    }

    if (flag > 1) { INFO("ch%lu calibration delay = %lu, bitslip = %lu", ch, gdly, gbitslip); }
    else {
      INFO("ch%lu fail to align ADC", ch);
    }
  }

  WriteADCALIGN(mid, 0x450000u);
  usleep(100);
  SendADCCAL(mid);
}

void NKTCB::AlignDRAM(unsigned long mid) const
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
  int gbitslip = 0;

  WriteDRAMON(mid, 1);
  WriteDRAMTEST(mid, 1);
  SendADCCAL(mid);
  WriteDRAMTEST(mid, 2);

  for (ch = 0; ch < 8; ++ch) {
    count = 0;
    sum = 0;
    flag = 0;

    for (dly = 0; dly < 32; ++dly) {
      WriteDRAMDLY(mid, ch, dly);
      WriteDRAMTEST(mid, 3);
      value = ReadDRAMTEST(mid, ch);

      aflag = 0;
      if (value == 0xFFAA5500UL || value == 0xAA5500FFUL || value == 0x5500FFAAUL || value == 0x00FFAA55UL) {
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

    WriteDRAMDLY(mid, ch, gdly);

    aflag = 0;
    for (bitslip = 0; bitslip < 4; ++bitslip) {
      WriteDRAMTEST(mid, 3);
      value = ReadDRAMTEST(mid, ch);

      if (value == 0xFFAA5500UL) {
        aflag = 1;
        gbitslip = bitslip;
        bitslip = 4;
      }
      else {
        aflag = 0;
        WriteDRAMBITSLIP(mid, ch);
      }
    }

    if (aflag) { INFO("DRAM(%lu) is aligned, delay = %lu, bitslip = %d", ch, gdly, gbitslip); }
    else {
      INFO("Fail to align DRAM(%lu)!", ch);
    }
  }

  WriteDRAMTEST(mid, 0);
}

void NKTCB::WriteRUNNO(unsigned long data) const { _usb.Write(0, 0x20000002u, static_cast<std::uint32_t>(data)); }

unsigned long NKTCB::ReadRUNNO() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000002u)); }

void NKTCB::WriteGATEDLY(unsigned long data) const { _usb.Write(0, 0x20000003u, static_cast<std::uint32_t>(data)); }

unsigned long NKTCB::ReadGATEDLY() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000003u)); }

void NKTCB::SendTRIG() const { _usb.Write(0, 0x20000004u, 0u); }

void NKTCB::ReadLNSTAT(unsigned long * data) const
{
  std::uint32_t addr = 0x20000005u;
  unsigned char rdat[8]{};
  _usb.Read(0, 2u, addr, rdat);

  for (int i = 0; i < 2; ++i) {
    unsigned long v = 0;
    v |= static_cast<unsigned long>(rdat[i * 4 + 0] & 0xFFu);
    v |= static_cast<unsigned long>((rdat[i * 4 + 1] & 0xFFu) << 8);
    v |= static_cast<unsigned long>((rdat[i * 4 + 2] & 0xFFu) << 16);
    v |= static_cast<unsigned long>((rdat[i * 4 + 3] & 0xFFu) << 24);
    data[i] = v;
  }
}

void NKTCB::ReadMIDS(unsigned long * data) const
{
  std::uint32_t addr = 0x20000007u;
  unsigned char rdat[160]{};
  _usb.Read(0, 40u, addr, rdat);

  for (int i = 0; i < 40; ++i) {
    unsigned long v = 0;
    v |= static_cast<unsigned long>(rdat[i * 4 + 0] & 0xFFu);
    v |= static_cast<unsigned long>((rdat[i * 4 + 1] & 0xFFu) << 8);
    v |= static_cast<unsigned long>((rdat[i * 4 + 2] & 0xFFu) << 16);
    v |= static_cast<unsigned long>((rdat[i * 4 + 3] & 0xFFu) << 24);
    data[i] = v;
  }
}

void NKTCB::WritePTRIG(unsigned long data) const { _usb.Write(0, 0x2000002Fu, static_cast<std::uint32_t>(data)); }

unsigned long NKTCB::ReadPTRIG() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x2000002Fu)); }

void NKTCB::WriteTRIGENABLE(unsigned long mid, unsigned long data) const
{
  _usb.Write(mid, 0x20000030u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadTRIGENABLE(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x20000030u));
}

void NKTCB::WriteEXTOUT(unsigned long data) const { _usb.Write(0, 0x20000039u, static_cast<std::uint32_t>(data)); }

unsigned long NKTCB::ReadEXTOUT() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000039u)); }

void NKTCB::WriteGATEWIDTH(unsigned long data) const { _usb.Write(0, 0x2000003Eu, static_cast<std::uint32_t>(data)); }

unsigned long NKTCB::ReadGATEWIDTH() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x2000003Eu)); }

void NKTCB::WriteEXTOUTWIDTH(unsigned long data) const { _usb.Write(0, 0x2000003Fu, static_cast<std::uint32_t>(data)); }

unsigned long NKTCB::ReadEXTOUTWIDTH() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x2000003Fu)); }

unsigned long NKTCB::ReadBCount(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x30000000u));
}

int NKTCB::ReadData(unsigned long mid, unsigned long bcount, unsigned char * data) const
{
  unsigned long count = bcount * 256UL;
  return _usb.Read(mid, count, 0x40000000u, data);
}

void NKTCB::WriteGAIN(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr = 0x20000013u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadGAIN(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr = 0x20000013u;
  if (mid > 0) { addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16); }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteMTHR(unsigned long mid, unsigned long data) const
{
  _usb.Write(mid, 0x20000015u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadMTHR(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x20000015u));
}

void NKTCB::WritePSDDLY(unsigned long mid, unsigned long data) const
{
  _usb.Write(mid, 0x20000029u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadPSDDLY(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x20000029u));
}

void NKTCB::WritePSDTHR(unsigned long mid, float data) const
{
  float fval = data * 128.0f;
  unsigned long value = static_cast<unsigned long>(fval);
  _usb.Write(mid, 0x2000002Au, static_cast<std::uint32_t>(value));
}

float NKTCB::ReadPSDTHR(unsigned long mid) const
{
  unsigned long data = static_cast<unsigned long>(_usb.ReadReg(mid, 0x2000002Au));
  float fval = static_cast<float>(data);
  fval = fval / 128.0f;
  return fval;
}

unsigned long NKTCB::ReadADCSTATWORD(unsigned long mid) const
{
  return static_cast<unsigned long>(_usb.ReadReg(mid, 0x2000001Bu));
}

void NKTCB::WriteDT(unsigned long mid, unsigned long ch, unsigned long data) const
{
  std::uint32_t addr;
  if (mid > 0) {
    addr = 0x2000000Fu;
    addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16);
  }
  else {
    addr = 0x2000003Au + static_cast<std::uint32_t>(ch);
  }
  _usb.Write(mid, addr, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadDT(unsigned long mid, unsigned long ch) const
{
  std::uint32_t addr;
  if (mid > 0) {
    addr = 0x2000000Fu;
    addr += static_cast<std::uint32_t>(((ch - 1UL) & 0xFFUL) << 16);
  }
  else {
    addr = 0x2000003Au + static_cast<std::uint32_t>(ch);
  }
  return static_cast<unsigned long>(_usb.ReadReg(mid, addr));
}

void NKTCB::WriteMTHRFADC500(unsigned long data) const { _usb.Write(0, 0x20000031u, static_cast<std::uint32_t>(data)); }

unsigned long NKTCB::ReadMTHRFADC500() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000031u)); }

void NKTCB::WritePSCALEFADC500(unsigned long data) const
{
  _usb.Write(0, 0x20000032u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadPSCALEFADC500() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000032u)); }

void NKTCB::WriteMTHRSADC64MU(unsigned long data) const
{
  _usb.Write(0, 0x20000033u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadMTHRSADC64MU() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000033u)); }

void NKTCB::WritePSCALESADC64MU(unsigned long data) const
{
  _usb.Write(0, 0x20000034u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadPSCALESADC64MU() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000034u)); }

void NKTCB::WriteMTHRSADC64LS(unsigned long data) const
{
  _usb.Write(0, 0x20000035u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadMTHRSADC64LS() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000035u)); }

void NKTCB::WritePSCALESADC64LS(unsigned long data) const
{
  _usb.Write(0, 0x20000036u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadPSCALESADC64LS() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000036u)); }

void NKTCB::WriteMTHRIADC64(unsigned long data) const { _usb.Write(0, 0x20000037u, static_cast<std::uint32_t>(data)); }

unsigned long NKTCB::ReadMTHRIADC64() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000037u)); }

void NKTCB::WritePSCALEIADC64(unsigned long data) const
{
  _usb.Write(0, 0x20000038u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadPSCALEIADC64() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000038u)); }

void NKTCB::WriteTrigSwitchFADC500(unsigned long fadc, unsigned long sadc_mu, unsigned long sadc_ls,
                                   unsigned long iadc) const
{
  unsigned long data = 0;
  if (fadc) data += 1;
  if (sadc_mu) data += 2;
  if (sadc_ls) data += 4;
  if (iadc) data += 8;
  _usb.Write(0, 0x20000043u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadSwitchFADC500() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000043u)); }

void NKTCB::WriteTrigSwitchSADC64MU(unsigned long fadc, unsigned long sadc_mu, unsigned long sadc_ls,
                                    unsigned long iadc) const
{
  unsigned long data = 0;
  if (fadc) data += 1;
  if (sadc_mu) data += 2;
  if (sadc_ls) data += 4;
  if (iadc) data += 8;
  _usb.Write(0, 0x20000044u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadSwitchSADC64MU() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000044u)); }

void NKTCB::WriteTrigSwitchSADC64LS(unsigned long fadc, unsigned long sadc_mu, unsigned long sadc_ls,
                                    unsigned long iadc) const
{
  unsigned long data = 0;
  if (fadc) data += 1;
  if (sadc_mu) data += 2;
  if (sadc_ls) data += 4;
  if (iadc) data += 8;
  _usb.Write(0, 0x20000045u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadSwitchSADC64LS() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000045u)); }

void NKTCB::WriteTrigSwitchIADC64(unsigned long fadc, unsigned long sadc_mu, unsigned long sadc_ls,
                                  unsigned long iadc) const
{
  unsigned long data = 0;
  if (fadc) data += 1;
  if (sadc_mu) data += 2;
  if (sadc_ls) data += 4;
  if (iadc) data += 8;
  _usb.Write(0, 0x20000046u, static_cast<std::uint32_t>(data));
}

unsigned long NKTCB::ReadSwitchIADC64() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000046u)); }

unsigned long NKTCB::ReadDBG() const { return static_cast<unsigned long>(_usb.ReadReg(0, 0x20000050u)); }
