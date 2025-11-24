/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DAQUtils/ELog.hh"
#include "Notice/nkusb.hh"
#include "Notice/nkusbconst.hh"

#define PVMEX_GET_ADDRESS (0xD2)

enum EManipInterface { kInterfaceClaim, kInterfaceRelease };

struct dev_open {
  libusb_device_handle * devh;
  uint16_t vendor_id;
  uint16_t product_id;
  int serial_id;
  struct dev_open * next;
} * ldev_open = 0;

///////////////////////////////////////////////////////////////////////////////
static void add_device(struct dev_open ** list, libusb_device_handle * tobeadded, uint16_t vendor_id,
                       uint16_t product_id, int sid)
{
  struct dev_open * curr;
  curr = (struct dev_open *)malloc(sizeof(struct dev_open));
  curr->devh = tobeadded;
  curr->vendor_id = vendor_id;
  curr->product_id = product_id;
  curr->serial_id = sid;
  curr->next = *list;
  *list = curr;
}

///////////////////////////////////////////////////////////////////////////////
static unsigned char get_serial_id(libusb_device_handle * devh)
{
  if (!devh) { return 0; }
  unsigned char data[1];
  int ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, PVMEX_GET_ADDRESS, 0, 0,
                                    data, 1, 1000);
  if (ret < 0) {
    WARNING("could not get serial id");
    return 0;
  }

  return data[0];
}

///////////////////////////////////////////////////////////////////////////////
static void remove_device_handle(struct dev_open ** list, libusb_device_handle * devh)
{
  if (!*list) { return; }

  struct dev_open * curr = *list;
  struct dev_open * prev = 0;

  while (curr) {
    if (curr->devh == devh) {
      if ((*list) == curr) { *list = curr->next; }
      else {
        prev->next = curr->next;
      }
      libusb_close(curr->devh);
      free(curr);
      return;
    }
    else {
      prev = curr;
      curr = curr->next;
    }
  }
  return;
}

///////////////////////////////////////////////////////////////////////////////
static void remove_device_id(struct dev_open ** list, uint16_t vendor_id, uint16_t product_id, int sid)
{
  if (!*list) { return; }

  struct dev_open * curr = *list;
  struct dev_open * prev = 0;
  struct libusb_device_descriptor desc;
  libusb_device * dev;

  while (curr) {
    dev = libusb_get_device(curr->devh);
    if (libusb_get_device_descriptor(dev, &desc) < 0) {
      WARNING("could not get device device descriptior");
      break;
    }
    if (desc.idVendor == vendor_id && desc.idProduct == product_id &&
        (sid == NK_SID_ANY || sid == get_serial_id(curr->devh))) {
      if (*list == curr) {
        *list = curr->next;
        libusb_close(curr->devh);
        free(curr);
        curr = *list;
      }
      else {
        prev->next = curr->next;
        libusb_close(curr->devh);
        free(curr);
        curr = prev->next;
      }
    }
    else {
      prev = curr;
      curr = curr->next;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
static int handle_interface_id(struct dev_open ** list, uint16_t vendor_id, uint16_t product_id, int sid, int interface,
                               enum EManipInterface manip_type)
{
  int ret = 0;
  if (!*list) {
    ret = -1;
    return ret;
  }

  struct dev_open * curr = *list;
  struct libusb_device_descriptor desc;
  libusb_device * dev;

  while (curr) {
    dev = libusb_get_device(curr->devh);
    if (libusb_get_device_descriptor(dev, &desc) < 0) {
      WARNING("could not get device descriptor");
      continue;
    }
    if (desc.idVendor == vendor_id && desc.idProduct == product_id &&
        (sid == NK_SID_ANY || sid == get_serial_id(curr->devh))) {
      if (manip_type == kInterfaceClaim) {
        if ((ret = libusb_claim_interface(curr->devh, interface)) < 0) {
          WARNING("could not claim interface(%d) on device(%u,%u,%u)", interface, vendor_id, product_id, sid);
        }
      }
      else if (manip_type == kInterfaceRelease) {
        if ((ret = libusb_release_interface(curr->devh, interface)) < 0) {
          WARNING("could not release interface(%d) on device(%u,%u,%u)", interface, vendor_id, product_id, sid);
        }
      }
      else {
        ERROR("unknown interface handle request: %d", manip_type);
        ret = -1;
        return ret;
      }
    }
    curr = curr->next;
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
static int is_device_open(libusb_device_handle * devh)
{
  struct dev_open * curr = ldev_open;
  libusb_device *dev, *dev_curr;
  int bus, bus_curr, addr, addr_curr;
  while (curr) {
    dev_curr = libusb_get_device(curr->devh);
    bus_curr = libusb_get_bus_number(dev_curr);
    addr_curr = libusb_get_device_address(dev_curr);

    dev = libusb_get_device(devh);
    bus = libusb_get_bus_number(dev);
    addr = libusb_get_device_address(dev);

    if (bus == bus_curr && addr == addr_curr) { return 1; }
    curr = curr->next;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
libusb_device_handle * nkusb_get_device_handle(uint16_t vendor_id, uint16_t product_id, int sid)
{
  struct dev_open * curr = ldev_open;
  while (curr) {
    if (curr->vendor_id == vendor_id && curr->product_id == product_id) {
      if (sid == NK_SID_ANY) { return curr->devh; }
      else if (curr->serial_id == sid) {
        return curr->devh;
      }
    }
    curr = curr->next;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
int nkusb_is_device_open(uint16_t vendor_id, uint16_t product_id, int sid)
{
  struct dev_open * curr = ldev_open;
  struct libusb_device_descriptor desc;
  libusb_device * dev;
  int r;
  while (curr) {
    dev = libusb_get_device(curr->devh);
    r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
      WARNING("could not get device descriptor");
      continue;
    }
    if (desc.idVendor == vendor_id && desc.idProduct == product_id &&
        (sid == NK_SID_ANY || get_serial_id(curr->devh) == sid)) {
      return 1;
    }
    curr = curr->next;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
int nkusb_init(libusb_context ** ctx)
{
  int ret;
  if ((ret = libusb_init(ctx)) < 0) {
    ERROR("failed to initialise libusb");
    exit(1);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
void nkusb_print_open_devices(void)
{
  struct dev_open * curr = ldev_open;
  libusb_device * dev;
  while (curr) {
    dev = libusb_get_device(curr->devh);
    INFO("bus = %d, address = %d", libusb_get_bus_number(dev), libusb_get_device_address(dev));
    curr = curr->next;
  }
}

///////////////////////////////////////////////////////////////////////////////
int nkusb_open_device(uint16_t vendor_id, uint16_t product_id, int sid, libusb_context * ctx)
{
  struct libusb_device ** devs;
  struct libusb_device * dev;
  struct libusb_device_handle * devh;
  size_t i = 0;
  int nopen_devices = 0;

  int r;

  if (libusb_get_device_list(ctx, &devs) < 0) {
    ERROR("could not get device list");
    return -1;
  }

  while ((dev = devs[i++])) {
    struct libusb_device_descriptor desc;
    r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
      WARNING("could not get device device descriptior");
      continue;
    }

    if (desc.idVendor == vendor_id && desc.idProduct == product_id) {
      r = libusb_open(dev, &devh);
      if (r < 0) {
        WARNING("could not open device");
        continue;
      }

      if (is_device_open(devh)) {
        libusb_close(devh);
        continue;
      }

      int interface = 0;
      if ((r = libusb_claim_interface(devh, interface)) < 0) {
        WARNING("could not claim interface 0 on the device (%d)", r);
        libusb_close(devh);
        continue;
      }

      int sid_tmp = get_serial_id(devh);
      if (sid == NK_SID_ANY || sid == sid_tmp) {
        add_device(&ldev_open, devh, vendor_id, product_id, sid_tmp);
        nopen_devices++;

        int speed = libusb_get_device_speed(dev);
        switch (speed) {
          case 4: INFO("super speed device opened"); break;
          case 3: INFO("high speed device opened"); break;
          case 2: INFO("full speed device opened"); break;
          case 1: INFO("low speed device opened"); break;
          case 0: WARNING("unknown speed device opened"); break;
        }
        INFO("bus = %d, address = %3d, serial id = %2u", libusb_get_bus_number(dev), libusb_get_device_address(dev),
             sid_tmp);
        libusb_release_interface(devh, interface);
        break;
      }
      else {
        libusb_release_interface(devh, interface);
        libusb_close(devh);
      }
    }
  }

  libusb_free_device_list(devs, 1);
  if (nopen_devices) { return 0; }
  else {
    return -1;
  }
}

///////////////////////////////////////////////////////////////////////////////
int nkusb_claim_interface(uint16_t vendor_id, uint16_t product_id, int sid, int interface)
{
  return handle_interface_id(&ldev_open, vendor_id, product_id, sid, interface, kInterfaceClaim);
}

///////////////////////////////////////////////////////////////////////////////
int nkusb_release_interface(uint16_t vendor_id, uint16_t product_id, int sid, int interface)
{
  return handle_interface_id(&ldev_open, vendor_id, product_id, sid, interface, kInterfaceRelease);
}

///////////////////////////////////////////////////////////////////////////////
void nkusb_close_device(uint16_t vendor_id, uint16_t product_id, int sid)
{
  remove_device_id(&ldev_open, vendor_id, product_id, sid);
  return;
}

///////////////////////////////////////////////////////////////////////////////
void nkusb_exit(libusb_context * ctx) { libusb_exit(ctx); }
