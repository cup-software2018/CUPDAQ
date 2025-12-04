// Notice/USBManager.hh
#pragma once

#include <cstdint>
#include <libusb.h>
#include <mutex>
#include <vector>

class USBManager {
public:
  static USBManager & Instance();

  USBManager(const USBManager &) = delete;
  USBManager & operator=(const USBManager &) = delete;

  int OpenDevice(uint16_t vendor_id, uint16_t product_id, int sid);
  int ClaimInterface(uint16_t vendor_id, uint16_t product_id, int sid, int interface);
  int ReleaseInterface(uint16_t vendor_id, uint16_t product_id, int sid, int interface);
  void CloseDevice(uint16_t vendor_id, uint16_t product_id, int sid);
  void PrintOpenDevices() const;
  int IsDeviceOpen(uint16_t vendor_id, uint16_t product_id, int sid) const;
  libusb_device_handle * GetDeviceHandle(uint16_t vendor_id, uint16_t product_id, int sid) const;

  libusb_context * Context() const;

private:
  struct DeviceEntry {
    libusb_device_handle * handle;
    uint16_t vendor_id;
    uint16_t product_id;
    int serial_id;
  };

  USBManager();
  ~USBManager();

  unsigned char GetSerialId(libusb_device_handle * handle) const;
  bool IsSameDevice(libusb_device_handle * a, libusb_device_handle * b) const;
  bool IsDeviceAlreadyOpen(libusb_device_handle * handle) const;
  void AddDevice(libusb_device_handle * handle, uint16_t vendor_id, uint16_t product_id, int sid);
  void RemoveDevice(libusb_device_handle * handle);
  void RemoveDevice(uint16_t vendor_id, uint16_t product_id, int sid);
  int HandleInterface(uint16_t vendor_id, uint16_t product_id, int sid, int interface, bool claim) const;

  libusb_context * _ctx;
  mutable std::mutex _mutex;
  std::vector<DeviceEntry> _devices;
};

inline libusb_context * USBManager::Context() const { return _ctx; }
