#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "DAQUtils/ELogger.hh"
#include "Notice/nkusb.hh"
#include "Notice/usb3tcb.hh"

#define NBURST (16)

///////////////////////////////////////////////////////////////////////////////
int USB3TCBInit(libusb_context ** ctx)
{
  return nkusb_init(ctx);
}

///////////////////////////////////////////////////////////////////////////////
int USB3TCBOpen(uint16_t vendor_id, uint16_t product_id, int sid, libusb_context * ctx)
{
  return nkusb_open_device(vendor_id, product_id, sid, ctx);
}

///////////////////////////////////////////////////////////////////////////////
void USB3TCBPrintOpenDevices(void)
{
  nkusb_print_open_devices();
}

///////////////////////////////////////////////////////////////////////////////
int USB3TCBClaimInterface(uint16_t vendor_id, uint16_t product_id, int sid, int interface)
{
  return nkusb_claim_interface(vendor_id, product_id, sid, interface);
}

///////////////////////////////////////////////////////////////////////////////
int USB3TCBReleaseInterface(uint16_t vendor_id, uint16_t product_id, int sid, int interface)
{
  return nkusb_release_interface(vendor_id, product_id, sid, interface);
}

///////////////////////////////////////////////////////////////////////////////
void USB3TCBClose(uint16_t vendor_id, uint16_t product_id, int sid)
{
  nkusb_close_device(vendor_id, product_id, sid);
}

///////////////////////////////////////////////////////////////////////////////
void USB3TCBExit(libusb_context * ctx)
{
  nkusb_exit(ctx);
}

///////////////////////////////////////////////////////////////////////////////
int USB3TCBWrite(uint16_t vendor_id,
                 uint16_t product_id,
                 int sid,
                 uint32_t mid,
                 uint32_t addr,
                 uint32_t data)
{
  const int length = 12;
  unsigned char buffer[length];

  buffer[0] = data & 0xFF;
  buffer[1] = (data >> 8) & 0xFF;
  buffer[2] = (data >> 16) & 0xFF;
  buffer[3] = (data >> 24) & 0xFF;

  buffer[4] = addr & 0xFF;
  buffer[5] = (addr >> 8) & 0xFF;
  buffer[6] = (addr >> 16) & 0xFF;
  buffer[7] = (addr >> 24) & 0x7F;

  buffer[8] = mid & 0xFF;
  buffer[9] = (mid >> 8) & 0xFF;
  buffer[10] = (mid >> 16) & 0xFF;
  buffer[11] = (mid >> 24) & 0xFF;

  libusb_device_handle * devh = nkusb_get_device_handle(vendor_id, product_id, sid);
  if (!devh) {
    ELogger::Instance(true)->Error(__func__, "could not get device handle for the device");
    return -1;
  }

  int stat = 0;
  int transferred = 0;
  const unsigned int timeout = 1000;
  if ((stat = libusb_bulk_transfer(devh, USB3_SF_WRITE, buffer, length, &transferred, timeout)) <
      0) {
    ELogger::Instance(true)->Error(__func__, "write error:%s [sid=%d]", libusb_error_name(stat), sid);
    return -1;
  }

  usleep(1000);

  return stat;
}

///////////////////////////////////////////////////////////////////////////////
int USB3TCBRead(uint16_t vendor_id,
                uint16_t product_id,
                int sid,
                uint32_t mid,
                uint32_t count,
                uint32_t addr,
                unsigned char * data)
{
  const int length = 12;
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

  buffer[8] = mid & 0xFF;
  buffer[9] = (mid >> 8) & 0xFF;
  buffer[10] = (mid >> 16) & 0xFF;
  buffer[11] = (mid >> 24) & 0xFF;

  libusb_device_handle * devh = nkusb_get_device_handle(vendor_id, product_id, sid);
  if (!devh) {
    ELogger::Instance(true)->Error(__func__, "could not get device handle for the device");
    return -1;
  }

  int stat = 0;
  int transferred = 0;
  const unsigned int timeout = 1000; // Wait forever

  if ((stat = libusb_bulk_transfer(devh,
                                   USB3_SF_WRITE,
                                   buffer,
                                   length,
                                   &transferred,
                                   timeout)) < 0) {
    ELogger::Instance(true)->Error(__func__, "write error:%s [sid=%d]", libusb_error_name(stat), sid);
    return -1;
  }

  int nbulk = count/4096;
  int remains = count%4096;
  const int size = 16384; // 16 kB

  for (int loop = 0; loop < nbulk; loop++) {
    if ((stat = libusb_bulk_transfer(devh,
                                     USB3_SF_READ,
                                     data + loop*size,
                                     size,
                                     &transferred,
                                     timeout)) < 0) {
      ELogger::Instance(true)->Error(__func__, "read error:%s [sid=%d]", libusb_error_name(stat), sid);
      return -1;
    }
  }

  if (remains) {
    if ((stat = libusb_bulk_transfer(devh,
                                     USB3_SF_READ,
                                     data + nbulk*size,
                                     remains*4,
                                     &transferred,
                                     timeout)) < 0) {
      ELogger::Instance(true)->Error(__func__, "read error:%s [sid=%d]", libusb_error_name(stat), sid);
      return -1;
    }
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
int USB3TCBWriteControl(uint16_t vendor_id,
                        uint16_t product_id,
                        int sid,
                        uint8_t bRequest,
                        uint16_t wValue,
                        uint16_t wIndex,
                        unsigned char * data,
                        uint16_t wLength)
{
  const unsigned int timeout = 1000;
  int stat = 0;

  libusb_device_handle * devh = nkusb_get_device_handle(vendor_id, product_id, sid);
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
                                      timeout)) < 0) {
    ELogger::Instance(true)->Error(__func__, "could not make write request:%s [sid=%d]",
                                  libusb_error_name(stat), sid);
    return stat;
  }

  return stat;
}

///////////////////////////////////////////////////////////////////////////////
int USB3TCBReadControl(uint16_t vendor_id,
                       uint16_t product_id,
                       int sid,
                       uint8_t bRequest,
                       uint16_t wValue,
                       uint16_t wIndex,
                       unsigned char * data,
                       uint16_t wLength)
{
  const unsigned int timeout = 1000;
  int stat = 0;

  libusb_device_handle * devh = nkusb_get_device_handle(vendor_id, product_id, sid);
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
                                      timeout)) < 0) {
    ELogger::Instance(true)->Error(__func__, "could not make write request:%s [sid=%d]",
                                  libusb_error_name(stat), sid);
    return stat;
  }
  //fprintf(stdout, "ADCRead: bytes transferred = %d\n", transferred);
  return 0;
}

///////////////////////////////////////////////////////////////////////////////
unsigned int USB3TCBReadReg(uint16_t vendor_id,
                            uint16_t product_id,
                            int sid,
                            uint32_t mid,
                            uint32_t addr)
{
  unsigned char data[4];
  unsigned int value;
  unsigned int tmp;

  USB3TCBRead(vendor_id, product_id, sid, mid, 1, addr, data);

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
unsigned long USB3TCBReadRegL(uint16_t vendor_id,
                              uint16_t product_id,
                              int sid,
                              uint32_t mid,
                              uint32_t addr)
{
  unsigned char data[8];
  unsigned long value;
  unsigned long tmp;

  USB3TCBRead(vendor_id, product_id, sid, mid, 2, addr, data);

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
unsigned char USB3TCBCheckFPGADone(uint16_t vendor_id, uint16_t product_id, int sid)
{
  const uint8_t bRequest = NKPROGRAMMER_TARGET_USB3;
  const uint16_t wValue = NKPROGRAMMER_CMD_CHECK_DONE;
  const uint16_t wIndex = 0;
  const uint16_t wLength = 1;

  unsigned char data;

  USB3TCBReadControl(vendor_id, product_id, sid, bRequest, wValue, wIndex, &data, wLength);

  return data;
}

///////////////////////////////////////////////////////////////////////////////
void USB3TCBInitFPGA(uint16_t vendor_id, uint16_t product_id, int sid)
{
  const uint8_t bRequest = NKPROGRAMMER_TARGET_USB3;
  const uint16_t wValue = NKPROGRAMMER_CMD_INIT;
  const uint16_t wIndex = 0;
  const uint16_t wLength = 1;
  unsigned char data = 0;

  USB3TCBWriteControl(vendor_id, product_id, sid, bRequest, wValue, wIndex, &data, wLength);
}

///////////////////////////////////////////////////////////////////////////////
unsigned char USB3TCBCheckFPGADoneNoAVR(uint16_t vendor_id, uint16_t product_id, int sid)
{
  const uint8_t bRequest = NKPROGRAMMER_CHECK_DONE;
  const uint16_t wValue = 0;
  const uint16_t wIndex = 0;
  const uint16_t wLength = 1;

  unsigned char data;

  USB3TCBReadControl(vendor_id, product_id, sid, bRequest, wValue, wIndex, &data, wLength);

  return data;
}

///////////////////////////////////////////////////////////////////////////////
void USB3TCBInitFPGANoAVR(uint16_t vendor_id, uint16_t product_id, int sid)
{
  const uint8_t bRequest = NKPROGRAMMER_RESET_FIFO;
  const uint16_t wValue = 0;
  const uint16_t wIndex = 0;
  const uint16_t wLength = 1;
  unsigned char data = 0;

  USB3TCBWriteControl(vendor_id, product_id, sid, bRequest, wValue, wIndex, &data, wLength);
}
