#pragma once

#include <cstdint>
#include <libusb.h>
#include <vector>

#include "Notice/USBManager.hh"

/**
 * @class USB3Tcb
 * @brief Handles USB communication for the TCB (Trigger Control Board).
 * * This class extends the standard protocol with a Module ID (mid) field.
 */
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

  // -----------------------------------------------------------------------
  // Core Communication Methods (with Module ID)
  // -----------------------------------------------------------------------

  /**
   * @brief Writes data to a specific address on a specific module.
   * @param mid  Module ID.
   * @param addr Register address.
   * @param data Data to write.
   * @return 0 on success, or negative libusb error code.
   */
  int Write(uint32_t mid, uint32_t addr, uint32_t data) const;

  /**
   * @brief Reads a block of data from a specific module.
   * @param mid   Module ID.
   * @param count Number of 32-bit words to read.
   * @param addr  Register address.
   * @param data  Buffer to store received data.
   * @return 0 on success, or negative libusb error code.
   */
  int Read(uint32_t mid, uint32_t count, uint32_t addr, unsigned char * data,
           unsigned int timeout = 1000u) const;

  // -----------------------------------------------------------------------
  // Control & Register Access
  // -----------------------------------------------------------------------

  int WriteControl(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char * data,
                   uint16_t wLength) const;
  int ReadControl(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char * data,
                  uint16_t wLength) const;

  /**
   * @brief Reads a 32-bit register safely.
   * @param status Optional pointer to store error code.
   * @return Register value or 0xFFFFFFFF on error.
   */
  uint32_t ReadReg(uint32_t mid, uint32_t addr, int * status = nullptr) const;

  /**
   * @brief Reads a 64-bit register safely.
   * @return Register value or 0xFFFFFFFFFFFFFFFF on error.
   */
  uint64_t ReadRegL(uint32_t mid, uint32_t addr, int * status = nullptr) const;

  int ReadRegI(uint32_t mid, uint32_t addr) const;

  // -----------------------------------------------------------------------
  // FPGA Utilities
  // -----------------------------------------------------------------------
  unsigned char CheckFPGADone() const;
  void InitFPGA() const;
  unsigned char CheckFPGADoneNoAVR() const;
  void InitFPGANoAVR() const;

  // Getters
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