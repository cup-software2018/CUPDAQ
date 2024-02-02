//
// Created by cupsoft on 6/23/20.
//
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>
#include <iostream>
#include <libusb.h>

#include "TString.h"
#include "DAQUtils/ELogger.hh"

using namespace std;

const int kNMAX = 100;
int gBUS[kNMAX];
int gDEV[kNMAX];

ELogger * gLog = nullptr;

int isnotice(libusb_device * dev, int & busid, int & devid)
{
  libusb_device_descriptor desc;

  int r = libusb_get_device_descriptor(dev, &desc);
  if (r < 0) {
    gLog->Error("%s: libusb_get_device_descriptor error", __func__ );
    return -1;
  }

  int vendor = desc.idVendor;
  if (vendor != 0x0547) return 1;

  busid = libusb_get_bus_number(dev);
  devid = libusb_get_device_address(dev);

  return 0;
}

int searchnotice()
{
  libusb_device ** devs;
  libusb_context * ctx = NULL;

  int r = libusb_init(&ctx); //initialize a library session
  if (r < 0) {
    gLog->Error("%s: libusb_init error", __func__ );
    return -1;
  }

  libusb_set_option(ctx, LIBUSB_OPTION_MAX);

  ssize_t cnt = libusb_get_device_list(ctx, &devs);
  if (cnt < 0) {
    gLog->Error("%s: libusb_get_device error", __func__ );
    return -1;
  }

  int n = 0;
  for (ssize_t i = 0; i < cnt; i++) {
    if (isnotice(devs[i], gBUS[n], gDEV[n]) == 0) {
      n += 1;
    }
  }

  gLog->Info("%s: %d NOTICE devices found", __func__, n);

  libusb_free_device_list(devs, 1); //free the list, unref the devices in it
  libusb_exit(ctx); //close the session

  return n;
}

int main(int argc, char ** argv)
{
  gLog = ELogger::Instance();

  int ndevs = searchnotice();

  if (ndevs == 0) {
    gLog->Info("%s: no NOTICE device found", __func__);
  }

  int fd;
  int rc;

  for (int i = 0; i < ndevs; i++) {
    const char * filename = Form("/dev/bus/usb/%03d/%03d", gBUS[i], gDEV[i]);

    fd = open(filename, O_WRONLY);
    if (fd < 0) {
      gLog->Error("%s: error opening output file", __func__);
      return 1;
    }

    rc = ioctl(fd, USBDEVFS_RESET, 0);
    if (rc < 0) {
      gLog->Error("%s: ioctl error", __func__);
      return 1;
    }

    gLog->Info("%s: reset USB device %s", __func__ , filename);
    close(fd);
  }

  return 0;
}
