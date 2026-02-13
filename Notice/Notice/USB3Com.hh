#pragma once

#include <cstdint>
#include <libusb.h>
#include <vector>

#include "Notice/USBManager.hh"

/**
 * @class USB3Com
 * @brief Handles low-level USB 3.0 communication with the DAQ hardware.
 * * This class manages bulk transfers and control requests via libusb.
 * It provides methods to read/write registers and handle FPGA initialization.
 */
class USB3Com {
public:
  USB3Com();
  USB3Com(uint16_t vendorId, uint16_t productId, int sid);
  ~USB3Com();

  // Set device identifiers manually
  void Set(uint16_t vendorId, uint16_t productId, int sid);

  // Open/Close device connection
  int Open();
  void Close();

  // Interface management
  int ClaimInterface(int interface);
  int ReleaseInterface(int interface);

  // -----------------------------------------------------------------------
  // Core Communication Methods
  // -----------------------------------------------------------------------

  /**
   * @brief Writes data to a specific address.
   * @return 0 on success, or negative libusb error code.
   */
  int Write(uint32_t addr, uint32_t data) const;

  /**
   * @brief Reads a block of data.
   * @param count Number of 32-bit words to read.
   * @param addr  Starting address.
   * @param data  Buffer to store received data (must be large enough).
   * @return 0 on success, or negative libusb error code.
   */
  int Read(uint32_t count, uint32_t addr, unsigned char * data, unsigned int timeout = 1000u) const;

  // -----------------------------------------------------------------------
  // Register Access Methods
  // -----------------------------------------------------------------------

  // Control transfers
  int WriteControl(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char * data,
                   uint16_t wLength) const;
  int ReadControl(uint8_t bRequest, uint16_t wValue, uint16_t wIndex, unsigned char * data,
                  uint16_t wLength) const;

  /**
   * @brief Reads a 32-bit register.
   * @param addr Register address.
   * @param status Optional pointer to store the error code (0 if success).
   * @return Register value, or 0xFFFFFFFF on error.
   */
  uint32_t ReadReg(uint32_t addr, int * status = nullptr) const;

  /**
   * @brief Reads a 64-bit register.
   * @param addr Register address.
   * @param status Optional pointer to store the error code (0 if success).
   * @return Register value, or 0xFFFFFFFFFFFFFFFF on error.
   */
  uint64_t ReadRegL(uint32_t addr, int * status = nullptr) const;

  // Legacy integer read wrapper
  int ReadRegI(uint32_t addr) const;

  // -----------------------------------------------------------------------
  // FPGA & Hardware Utilities
  // -----------------------------------------------------------------------
  unsigned char CheckFPGADone() const;
  void InitFPGA() const;
  unsigned char CheckFPGADoneNoAVR() const;
  void InitFPGANoAVR() const;
  void ResetEP2() const;
  void ResetEP6() const;

  // Getters
  uint16_t VendorId() const;
  uint16_t ProductId() const;
  int Sid() const;

private:
  // Helper to retrieve the libusb handle from USBManager
  libusb_device_handle * Handle() const;

  uint16_t _vendorId;
  uint16_t _productId;
  int _sid;
};

// Inline implementations for getters
inline void USB3Com::Set(uint16_t vendorId, uint16_t productId, int sid)
{
  _vendorId = vendorId;
  _productId = productId;
  _sid = sid;
}

inline uint16_t USB3Com::VendorId() const { return _vendorId; }
inline uint16_t USB3Com::ProductId() const { return _productId; }
inline int USB3Com::Sid() const { return _sid; }

inline libusb_device_handle * USB3Com::Handle() const
{
  return USBManager::Instance().GetDeviceHandle(_vendorId, _productId, _sid);
}