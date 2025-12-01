#include <cstdint>
#include <string>

#include "DAQUtils/ELog.hh"
#include "NewNotice/USB3Com.hh"

int main(int argc, char ** argv)
{
  if (argc < 3) {
    ERROR("Usage: %s <vendor_id hex> <product_id hex> [sid]", argv[0]);
    return 1;
  }

  uint16_t vendorId = static_cast<uint16_t>(std::stoul(argv[1], nullptr, 16));
  uint16_t productId = static_cast<uint16_t>(std::stoul(argv[2], nullptr, 16));
  int sid = (argc >= 4) ? std::stoi(argv[3]) : 0;

  INFO("=== USB3Com Test Start ===");
  INFO("Vendor ID  = 0x%x", vendorId);
  INFO("Product ID = 0x%x", productId);
  INFO("SID        = %d", sid);

  USB3Com usb(vendorId, productId, sid);

  INFO("Opening USB device...");
  int ret = usb.Open();
  if (ret < 0) {
    ERROR("Failed to open USB device (vid=0x%x pid=0x%x sid=%d)", vendorId, productId, sid);
    return 1;
  }

  INFO("Device opened successfully.");

  INFO("Claiming interface 0...");
  ret = usb.ClaimInterface(0);
  if (ret < 0) {
    ERROR("Failed to claim interface 0");
    usb.Close();
    return 1;
  }

  INFO("Interface 0 claimed successfully.");

  INFO("Releasing interface 0...");
  usb.ReleaseInterface(0);

  INFO("Closing device...");
  usb.Close();

  INFO("=== USB3Com Test Finished Successfully ===");

  return 0;
}
