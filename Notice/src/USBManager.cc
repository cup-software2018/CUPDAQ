#include <cstdlib>

#include "DAQUtils/ELog.hh"
#include "Notice/USBManager.hh"

namespace {
constexpr uint8_t PVMEX_GET_ADDRESS = 0xD2;
constexpr uint8_t NK_SID_ANY = 0xFF;
} // namespace

USBManager & USBManager::Instance()
{
  static USBManager instance;
  return instance;
}

USBManager::USBManager()
  : _ctx(nullptr)
{
  int ret = libusb_init(&_ctx);
  if (ret < 0) {
    ERROR("failed to initialise libusb: %d", ret);
    _ctx = nullptr;
  }
}

USBManager::~USBManager()
{
  std::lock_guard<std::mutex> lock(_mutex);
  for (auto & entry : _devices) {
    if (entry.handle) { libusb_close(entry.handle); }
  }
  _devices.clear();
  if (_ctx) {
    libusb_exit(_ctx);
    _ctx = nullptr;
  }
}

unsigned char USBManager::GetSerialId(libusb_device_handle * handle) const
{
  if (!handle) { return 0; }
  unsigned char data[1] = {0};
  int ret = libusb_control_transfer(handle, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
                                    PVMEX_GET_ADDRESS, 0, 0, data, 1, 1000);
  if (ret < 0) {
    WARNING("could not get serial id");
    return 0;
  }
  return data[0];
}

bool USBManager::IsSameDevice(libusb_device_handle * a, libusb_device_handle * b) const
{
  if (!a || !b) { return false; }
  libusb_device * dev_a = libusb_get_device(a);
  libusb_device * dev_b = libusb_get_device(b);
  if (!dev_a || !dev_b) { return false; }
  int bus_a = libusb_get_bus_number(dev_a);
  int bus_b = libusb_get_bus_number(dev_b);
  int addr_a = libusb_get_device_address(dev_a);
  int addr_b = libusb_get_device_address(dev_b);
  return bus_a == bus_b && addr_a == addr_b;
}

bool USBManager::IsDeviceAlreadyOpen(libusb_device_handle * handle) const
{
  for (const auto & entry : _devices) {
    if (IsSameDevice(entry.handle, handle)) { return true; }
  }
  return false;
}

void USBManager::AddDevice(libusb_device_handle * handle, uint16_t vendor_id, uint16_t product_id,
                           int sid)
{
  DeviceEntry entry{handle, vendor_id, product_id, sid};
  _devices.push_back(entry);
}

void USBManager::RemoveDevice(libusb_device_handle * handle)
{
  auto it = _devices.begin();
  while (it != _devices.end()) {
    if (it->handle == handle) {
      libusb_close(it->handle);
      it = _devices.erase(it);
    }
    else {
      ++it;
    }
  }
}

void USBManager::RemoveDevice(uint16_t vendor_id, uint16_t product_id, int sid)
{
  // std::lock_guard<std::mutex> lock(_mutex);

  auto it = _devices.begin();
  while (it != _devices.end()) {
    if (it->vendor_id == vendor_id && it->product_id == product_id &&
        (sid == NK_SID_ANY || it->serial_id == sid)) {

      if (it->handle) { libusb_close(it->handle); }
      it = _devices.erase(it);
    }
    else {
      ++it;
    }
  }
}

int USBManager::HandleInterface(uint16_t vendor_id, uint16_t product_id, int sid, int interface,
                                bool claim) const
{
  int ret = 0;
  if (!_ctx) { return -1; }

  // Mutex lock is already held by the caller (ClaimInterface/ReleaseInterface)

  for (const auto & entry : _devices) {
    // 하드웨어 쿼리 대신 캐시된 정보(entry) 사용 -> 속도 향상 및 통신 에러 방지
    if (entry.vendor_id == vendor_id && entry.product_id == product_id) {
      if (sid == NK_SID_ANY || entry.serial_id == sid) {
        if (claim) {
          ret = libusb_claim_interface(entry.handle, interface);
          if (ret < 0) {
            WARNING("could not claim interface(%d) on device(SID: %d)", interface, entry.serial_id);
          }
        }
        else {
          ret = libusb_release_interface(entry.handle, interface);
          if (ret < 0) {
            WARNING("could not release interface(%d) on device(SID: %d)", interface,
                    entry.serial_id);
          }
        }
      }
    }
  }
  return ret;
}

int USBManager::OpenDevice(uint16_t vendor_id, uint16_t product_id, int sid)
{
  if (!_ctx) {
    ERROR("libusb context is not initialized");
    return -1;
  }

  std::lock_guard<std::mutex> lock(_mutex);

  libusb_device ** devs = nullptr;
  ssize_t count = libusb_get_device_list(_ctx, &devs);
  if (count < 0) {
    ERROR("could not get device list");
    return -1;
  }

  int nopen_devices = 0;

  for (ssize_t i = 0; i < count; ++i) {
    libusb_device * dev = devs[i];
    if (!dev) { continue; }

    libusb_device_descriptor desc{};
    int r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
      WARNING("could not get device descriptor");
      continue;
    }

    if (desc.idVendor != vendor_id || desc.idProduct != product_id) { continue; }

    libusb_device_handle * handle = nullptr;
    r = libusb_open(dev, &handle);
    if (r < 0) {
      WARNING("could not open device");
      continue;
    }

    if (IsDeviceAlreadyOpen(handle)) {
      libusb_close(handle);
      continue;
    }

    int interface = 0;
    r = libusb_claim_interface(handle, interface);
    if (r < 0) {
      WARNING("could not claim interface 0 on the device (%d)", r);
      libusb_close(handle);
      continue;
    }

    int sid_tmp = static_cast<int>(GetSerialId(handle));
    if (sid == NK_SID_ANY || sid == sid_tmp) {
      AddDevice(handle, vendor_id, product_id, sid_tmp);
      nopen_devices++;

      int speed = libusb_get_device_speed(dev);
      switch (speed) {
        case LIBUSB_SPEED_SUPER: INFO("super speed device opened"); break;
        case LIBUSB_SPEED_HIGH: INFO("high speed device opened"); break;
        case LIBUSB_SPEED_FULL: INFO("full speed device opened"); break;
        case LIBUSB_SPEED_LOW: INFO("low speed device opened"); break;
        default: WARNING("unknown speed device opened"); break;
      }

      INFO("bus = %d, address = %3d, serial id = %2u", libusb_get_bus_number(dev),
           libusb_get_device_address(dev), static_cast<unsigned>(sid_tmp));

      libusb_release_interface(handle, interface);
      break;
    }
    else {
      libusb_release_interface(handle, interface);
      libusb_close(handle);
    }
  }

  libusb_free_device_list(devs, 1);

  if (nopen_devices) { return 0; }
  return -1;
}

int USBManager::ClaimInterface(uint16_t vendor_id, uint16_t product_id, int sid, int interface)
{
  std::lock_guard<std::mutex> lock(_mutex);
  return HandleInterface(vendor_id, product_id, sid, interface, true);
}

int USBManager::ReleaseInterface(uint16_t vendor_id, uint16_t product_id, int sid, int interface)
{
  std::lock_guard<std::mutex> lock(_mutex);
  return HandleInterface(vendor_id, product_id, sid, interface, false);
}

void USBManager::CloseDevice(uint16_t vendor_id, uint16_t product_id, int sid)
{
  std::lock_guard<std::mutex> lock(_mutex);
  RemoveDevice(vendor_id, product_id, sid);
}

void USBManager::PrintOpenDevices() const
{
  std::lock_guard<std::mutex> lock(_mutex);
  for (const auto & entry : _devices) {
    libusb_device * dev = libusb_get_device(entry.handle);
    if (!dev) { continue; }
    INFO("bus = %d, address = %d", libusb_get_bus_number(dev), libusb_get_device_address(dev));
  }
}

bool USBManager::IsDeviceOpen(uint16_t vendor_id, uint16_t product_id, int sid) const
{
  std::lock_guard<std::mutex> lock(_mutex);
  for (const auto & entry : _devices) {
    if (entry.vendor_id == vendor_id && entry.product_id == product_id) {
      if (sid == NK_SID_ANY || entry.serial_id == sid) {
        return true; // Found
      }
    }
  }
  return false;
}

libusb_device_handle * USBManager::GetDeviceHandle(uint16_t vendor_id, uint16_t product_id,
                                                   int sid) const
{
  std::lock_guard<std::mutex> lock(_mutex);
  for (const auto & entry : _devices) {
    if (entry.vendor_id == vendor_id && entry.product_id == product_id) {
      if (sid == NK_SID_ANY || entry.serial_id == sid) { return entry.handle; }
    }
  }
  return nullptr;
}
