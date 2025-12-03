#pragma once

#include <cstdint>
#include <libusb.h>

#include "NewNotice/USBManager.hh"

class USB3Tcb {
public:
  USB3Tcb();
  USB3Tcb(uint16_t vendorId, uint16_t productId, int sid);
  ~USB3Tcb();

  void Set(uint16_t vendorId, uint16_t productId, int sid);

  int Open();
  void Close();

  int ClaimInterface(int interface);
  int ReleaseInterface(int interface);

  int Write(uint32_t mid, uint32_t addr, uint32_t data) const;
  int Read(uint32_t mid, uint32_t count, uint32_t addr, unsigned char * data, unsigned int timeout = 1000u) const;

  int WriteControl(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char * data, uint16_t wLength) const;
  int ReadControl(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char * data, uint16_t wLength) const;

  unsigned int ReadReg(uint32_t mid, uint32_t addr) const;
  unsigned long ReadRegL(uint32_t mid, uint32_t addr) const;
  int ReadRegI(uint32_t mid, uint32_t addr) const;

  unsigned char CheckFPGADone() const;
  void InitFPGA() const;
  unsigned char CheckFPGADoneNoAVR() const;
  void InitFPGANoAVR() const;

  uint16_t VendorId() const;
  uint16_t ProductId() const;
  int Sid() const;

private:
  libusb_device_handle * Handle() const;

  uint16_t _vendorId;
  uint16_t _productId;
  int _sid;
};

inline void USB3Tcb::Set(uint16_t vendorId, uint16_t productId, int sid) 
{
  _vendorId = vendorId;
  _productId = productId;
  _sid = sid;
}


inline uint16_t USB3Tcb::VendorId() const { return _vendorId; }
inline uint16_t USB3Tcb::ProductId() const { return _productId; }
inline int USB3Tcb::Sid() const { return _sid; }

inline libusb_device_handle * USB3Tcb::Handle() const
{
  return USBManager::Instance().GetDeviceHandle(_vendorId, _productId, _sid);
}
