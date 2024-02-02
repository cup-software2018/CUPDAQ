#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "DAQUtils/ELogger.hh"
#include "Notice/nkusb.hh"
#include "Notice/usb3com.hh"

#define NBURST (16)

///////////////////////////////////////////////////////////////////////////////
int USB3Init(libusb_context** ctx)
{
  return nkusb_init(ctx);
}

///////////////////////////////////////////////////////////////////////////////
int USB3Open(uint16_t vendor_id, uint16_t product_id, int sid, libusb_context* ctx)
{
  return nkusb_open_device(vendor_id, product_id, sid, ctx);
}

///////////////////////////////////////////////////////////////////////////////
void USB3PrintOpenDevices(void)
{
  nkusb_print_open_devices();
}

///////////////////////////////////////////////////////////////////////////////
int USB3ClaimInterface(uint16_t vendor_id, uint16_t product_id, int sid, int interface)
{
  return nkusb_claim_interface(vendor_id, product_id, sid, interface);
}

///////////////////////////////////////////////////////////////////////////////
int USB3ReleaseInterface(uint16_t vendor_id, uint16_t product_id, int sid, int interface)
{
  return nkusb_release_interface(vendor_id, product_id, sid, interface);
}

///////////////////////////////////////////////////////////////////////////////
void USB3Close(uint16_t vendor_id, uint16_t product_id, int sid)
{
  nkusb_close_device(vendor_id, product_id, sid);
}

///////////////////////////////////////////////////////////////////////////////
void USB3Exit(libusb_context* ctx)
{
  nkusb_exit(ctx);
}

///////////////////////////////////////////////////////////////////////////////
int USB3Write(uint16_t vendor_id, uint16_t product_id, int sid, uint32_t addr, uint32_t data)
{
  const int length = 8;
  unsigned char buffer[length];

  buffer[0] = data & 0xFF;
  buffer[1] = (data >> 8) & 0xFF;
  buffer[2] = (data >> 16) & 0xFF;
  buffer[3] = (data >> 24) & 0xFF;

  buffer[4] = addr & 0xFF;
  buffer[5] = (addr >> 8) & 0xFF;
  buffer[6] = (addr >> 16) & 0xFF;
  buffer[7] = (addr >> 24) & 0x7F;

  libusb_device_handle* devh = nkusb_get_device_handle(vendor_id, product_id, sid);
  if (!devh) {
    ELogger::Instance(true)->Error(__func__, "could not get device handle for the device");
    return -1;
  }

  int stat = 0;
  int transferred = 0;
  const unsigned int timeout = 1000;
  if ((stat = libusb_bulk_transfer(devh, USB3_SF_WRITE, buffer, length, &transferred, timeout)) < 0) {
    ELogger::Instance(true)->Error(__func__, "write error:%s [sid=%d]", libusb_error_name(stat), sid);
    return stat;
  }

  usleep(1000);

  return stat;
}

///////////////////////////////////////////////////////////////////////////////
int USB3Read(uint16_t vendor_id, uint16_t product_id, int sid,
    uint32_t count, uint32_t addr, unsigned char* data)
{
  const int length = 8;
  unsigned char buffer[length];

  buffer[0] = count & 0xFF;
  buffer[1] = (count >> 8) & 0xFF;
  buffer[2] = (count >> 16) & 0xFF;
  buffer[3] = (count >> 24) & 0xFF;

  buffer[4] = addr & 0xFF;
  buffer[5] = (addr >> 8) & 0xFF;
  buffer[6] = (addr >> 16) & 0xFF;
  buffer[7] = (addr >> 24) & 0x7F;
  buffer[7] = buffer[7] | 0x80;

  libusb_device_handle* devh = nkusb_get_device_handle(vendor_id, product_id, sid);
  if (!devh) {
    ELogger::Instance(true)->Error(__func__, "could not get device handle for the device");
    return -1;
  }

  int stat = 0;
  int transferred = 0;
  const unsigned int timeout = 1000; // Wait forever

  if ((stat = libusb_bulk_transfer(devh, USB3_SF_WRITE, buffer, length, &transferred, timeout)) < 0) {
    ELogger::Instance(true)->Error(__func__, "%s: write error:%s [sid=%d]", libusb_error_name(stat), sid);
    return stat;
  }

  int nbulk = count / 4096;
  int remains = count % 4096;
  const int size = 16384; // 16 kB

  for (int loop = 0; loop < nbulk; loop++) {
    if ((stat = libusb_bulk_transfer(devh, USB3_SF_READ, data + loop * size, size, &transferred, timeout)) < 0) {
      ELogger::Instance(true)->Error(__func__, "read error:%s [sid=%d]", libusb_error_name(stat), sid);
      return stat;
    }
  }

  if (remains > 0) {
    if ((stat = libusb_bulk_transfer(devh, USB3_SF_READ, data + nbulk * size, remains * 4, &transferred, timeout)) < 0) {
      ELogger::Instance(true)->Error(__func__, "read error:%s [sid=%d]", libusb_error_name(stat), sid);
      return stat;
    }
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
int USB3ReadTimeout(uint16_t vendor_id, uint16_t product_id, int sid,
    uint32_t count, uint32_t addr, unsigned int timeout, unsigned char* data)
{
  const int length = 8;
  unsigned char buffer[length];

  buffer[0] = count & 0xFF;
  buffer[1] = (count >> 8) & 0xFF;
  buffer[2] = (count >> 16) & 0xFF;
  buffer[3] = (count >> 24) & 0xFF;

  buffer[4] = addr & 0xFF;
  buffer[5] = (addr >> 8) & 0xFF;
  buffer[6] = (addr >> 16) & 0xFF;
  buffer[7] = (addr >> 24) & 0x7F;
  buffer[7] = buffer[7] | 0x80;

  libusb_device_handle* devh = nkusb_get_device_handle(vendor_id, product_id, sid);
  if (!devh) {
    ELogger::Instance(true)->Error(__func__, "could not get device handle for the device");
    return -1;
  }

  int stat = 0;
  int transferred = 0;

  if ((stat = libusb_bulk_transfer(devh,
           USB3_SF_WRITE,
           buffer,
           length,
           &transferred,
           timeout))
      < 0) {
    ELogger::Instance(true)->Error(__func__, "write error:%s [sid=%d]", libusb_error_name(stat), sid);
    return stat;
  }

  int nbulk = count / 4096;
  int remains = count % 4096;
  const int size = 16384; // 16 kB

  for (int loop = 0; loop < nbulk; loop++) {
    if ((stat = libusb_bulk_transfer(devh,
             USB3_SF_READ,
             data + loop * size,
             size,
             &transferred,
             timeout))
        < 0) {
      ELogger::Instance(true)->Error(__func__, "read error:%s [sid=%d]", libusb_error_name(stat), sid);
      return stat;
    }
  }

  if (remains > 0) {
    if ((stat = libusb_bulk_transfer(devh,
             USB3_SF_READ,
             data + nbulk * size,
             remains * 4,
             &transferred,
             timeout))
        < 0) {
      ELogger::Instance(true)->Error(__func__, "read error:%s [sid=%d]", libusb_error_name(stat), sid);
      return stat;
    }
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
int USB3WriteControl(uint16_t vendor_id,
    uint16_t product_id,
    int sid,
    uint8_t bRequest,
    uint16_t wValue,
    uint16_t wIndex,
    unsigned char* data,
    uint16_t wLength)
{
  const unsigned int timeout = 1000;
  int stat = 0;

  libusb_device_handle* devh = nkusb_get_device_handle(vendor_id, product_id, sid);
  if (!devh) {
    ELogger::Instance(true)->Error(__func__, "could not get device handle for the device");
    return -1;
  }

  if ((stat = libusb_control_transfer(devh,
           LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT,
           bRequest,
           wValue,
           wIndex,
           data,
           wLength,
           timeout))
      < 0) {
    ELogger::Instance(true)->Error(__func__, "%s: could not make write request:%s [sid=%d]",
        libusb_error_name(stat), sid);
    return stat;
  }

  return stat;
}

///////////////////////////////////////////////////////////////////////////////
int USB3ReadControl(uint16_t vendor_id,
    uint16_t product_id,
    int sid,
    uint8_t bRequest,
    uint16_t wValue,
    uint16_t wIndex,
    unsigned char* data,
    uint16_t wLength)
{
  const unsigned int timeout = 1000;
  int stat = 0;

  libusb_device_handle* devh = nkusb_get_device_handle(vendor_id, product_id, sid);
  if (!devh) {
    ELogger::Instance(true)->Error(__func__, "could not get device handle for the device");
    return -1;
  }

  if ((stat = libusb_control_transfer(devh,
           LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN,
           bRequest,
           wValue,
           wIndex,
           data,
           wLength,
           timeout))
      < 0) {
    ELogger::Instance(true)->Error(__func__, "could not make write request:%s [sid=%d]",
        libusb_error_name(stat), sid);
    return stat;
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
unsigned int USB3ReadReg(uint16_t vendor_id, uint16_t product_id, int sid, uint32_t addr)
{
  unsigned char data[4];
  unsigned int value;
  unsigned int tmp;

  int status = USB3Read(vendor_id, product_id, sid, 1, addr, data);
  if (status < 0) {
    ELogger::Instance(true)->Error(__func__, "read error:%s [sid=%d]", libusb_error_name(status), sid);
    return status;
  }

  value = data[0] & 0xFF;
  tmp = data[1] & 0xFF;
  value = value + (unsigned int)(tmp << 8);
  tmp = data[2] & 0xFF;
  value = value + (unsigned int)(tmp << 16);
  tmp = data[3] & 0xFF;
  value = value + (unsigned int)(tmp << 24);

  return value;
}

///////////////////////////////////////////////////////////////////////////////
int USB3ReadRegI(uint16_t vendor_id, uint16_t product_id, int sid, uint32_t addr)
{
/*
  unsigned char data[4];
  unsigned int value;
  unsigned int tmp;

  int status = USB3Read(vendor_id, product_id, sid, 1, addr, data);
  if (status < 0) {
    return status;
  }

  value = data[0] & 0xFF;
  tmp = data[1] & 0xFF;
  value = value + (unsigned int)(tmp << 8);
  tmp = data[2] & 0xFF;
  value = value + (unsigned int)(tmp << 16);
  tmp = data[3] & 0xFF;
  value = value + (unsigned int)(tmp << 24);

  return (int)value;
*/
  return (int)USB3ReadReg(vendor_id, product_id, sid, addr);
}

///////////////////////////////////////////////////////////////////////////////
unsigned long USB3ReadRegL(uint16_t vendor_id, uint16_t product_id, int sid, uint32_t addr)
{
  unsigned char data[8];
  unsigned long value;
  unsigned long tmp;

  int status = USB3Read(vendor_id, product_id, sid, 2, addr, data);
  if (status < 0) {
    ELogger::Instance(true)->Error(__func__, "read error:%s [sid=%d]", libusb_error_name(status), sid);
    return status;
  }

  value = data[0] & 0xFF;
  tmp = data[1] & 0xFF;
  value = value + (unsigned long)(tmp << 8);
  tmp = data[2] & 0xFF;
  value = value + (unsigned long)(tmp << 16);
  tmp = data[3] & 0xFF;
  value = value + (unsigned long)(tmp << 24);
  tmp = data[4] & 0xFF;
  value = value + (unsigned long)(tmp << 32);
  tmp = data[5] & 0xFF;
  value = value + (unsigned long)(tmp << 40);
  tmp = data[6] & 0xFF;
  value = value + (unsigned long)(tmp << 48);
  tmp = data[7] & 0xFF;
  value = value + (unsigned long)(tmp << 56);

  return value;
}

///////////////////////////////////////////////////////////////////////////////
unsigned char USB3CheckFPGADone(uint16_t vendor_id, uint16_t product_id, int sid)
{
  const uint8_t bRequest = NKPROGRAMMER_TARGET_USB3;
  const uint16_t wValue = NKPROGRAMMER_CMD_CHECK_DONE;
  const uint16_t wIndex = 0;
  const uint16_t wLength = 1;

  unsigned char data;

  USB3ReadControl(vendor_id, product_id, sid, bRequest, wValue, wIndex, &data, wLength);

  return data;
}

///////////////////////////////////////////////////////////////////////////////
void USB3InitFPGA(uint16_t vendor_id, uint16_t product_id, int sid)
{
  const uint8_t bRequest = NKPROGRAMMER_TARGET_USB3;
  const uint16_t wValue = NKPROGRAMMER_CMD_INIT;
  const uint16_t wIndex = 0;
  const uint16_t wLength = 1;
  unsigned char data = 0;

  USB3WriteControl(vendor_id, product_id, sid, bRequest, wValue, wIndex, &data, wLength);
}

///////////////////////////////////////////////////////////////////////////////
unsigned char USB3CheckFPGADoneNoAVR(uint16_t vendor_id, uint16_t product_id, int sid)
{
  const uint8_t bRequest = NKPROGRAMMER_CHECK_DONE;
  const uint16_t wValue = 0;
  const uint16_t wIndex = 0;
  const uint16_t wLength = 1;

  unsigned char data;

  USB3ReadControl(vendor_id, product_id, sid, bRequest, wValue, wIndex, &data, wLength);

  return data;
}

///////////////////////////////////////////////////////////////////////////////
void USB3InitFPGANoAVR(uint16_t vendor_id, uint16_t product_id, int sid)
{
  const uint8_t bRequest = NKPROGRAMMER_RESET_FIFO;
  const uint16_t wValue = 0;
  const uint16_t wIndex = 0;
  const uint16_t wLength = 1;
  unsigned char data = 0;

  USB3WriteControl(vendor_id, product_id, sid, bRequest, wValue, wIndex, &data, wLength);
}

///////////////////////////////////////////////////////////////////////////////
void USB3ResetEP2(uint16_t vendor_id, uint16_t product_id, int sid)
{
  const uint8_t bRequest = NKPROGRAMMER_RESET_EP2;
  const uint16_t wValue = 0;
  const uint16_t wIndex = 0;
  const uint16_t wLength = 0;
  unsigned char data = 0;

  USB3WriteControl(vendor_id, product_id, sid, bRequest, wValue, wIndex, &data, wLength);
}

///////////////////////////////////////////////////////////////////////////////
void USB3ResetEP6(uint16_t vendor_id, uint16_t product_id, int sid)
{
  const uint8_t bRequest = NKPROGRAMMER_RESET_EP6;
  const uint16_t wValue = 0;
  const uint16_t wIndex = 0;
  const uint16_t wLength = 0;
  unsigned char data = 0;

  USB3WriteControl(vendor_id, product_id, sid, bRequest, wValue, wIndex, &data, wLength);
}
