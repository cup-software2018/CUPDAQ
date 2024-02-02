#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Notice/NoticeMUONDAQ.hh"
#include "Notice/usb3com.hh"


// open MUONDAQ
int MUONDAQopen(int sid, libusb_context* ctx)
{
  int status;

  status = USB3Open(MUONDAQ_VENDOR_ID, MUONDAQ_PRODUCT_ID, sid, ctx);
  USB3ClaimInterface(MUONDAQ_VENDOR_ID, MUONDAQ_PRODUCT_ID, sid, 0);

  return status;
}

// close MUONDAQ
void MUONDAQclose(int sid)
{
  USB3ReleaseInterface(MUONDAQ_VENDOR_ID, MUONDAQ_PRODUCT_ID, sid, 0);
  USB3Close(MUONDAQ_VENDOR_ID, MUONDAQ_PRODUCT_ID, sid);
}

// Read data buffer count, 1 buffer count = 1 kbyte data
int MUONDAQread_BCOUNT(int sid)
{
  // return USB3ReadRegI(MUONDAQ_VENDOR_ID, MUONDAQ_PRODUCT_ID, sid, 0x20000010);
  return USB3ReadRegI(MUONDAQ_VENDOR_ID, MUONDAQ_PRODUCT_ID, sid, 0x30000000);
}

// read data, reads bcount * 1 kbytes data from MUONDAQ DRAM
// returns character raw data, needs sorting after data acquisition
int MUONDAQread_DATA(int sid, int bcount, unsigned char* data, unsigned int timeout)
{
  int retval;

  // maximum data size is 64 Mbyte
  int count = bcount * 256;

  if (timeout == 0)
    retval = USB3Read(MUONDAQ_VENDOR_ID, MUONDAQ_PRODUCT_ID, sid, count, 0x40000000, data);
  else
    retval = USB3ReadTimeout(MUONDAQ_VENDOR_ID, MUONDAQ_PRODUCT_ID, sid, count, 0x40000000, timeout, data);

  return retval;
}

// flush remaining data
void MUONDAQflush_DATA(int sid)
{
  // max buffer size is 10 MB
  unsigned char data[10485760];
  unsigned long bcount;
  unsigned long chunk;
  unsigned long slice;
  unsigned long i;

  // check data size to be flushed
  bcount = MUONDAQread_BCOUNT(sid);
  chunk = bcount / 10240;
  slice = bcount % 10240;

  for (i = 0; i < chunk; i++)
    MUONDAQread_DATA(sid, 10240, data);

  if (slice)
    MUONDAQread_DATA(sid, slice, data);
}
