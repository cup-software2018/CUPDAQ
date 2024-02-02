#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Notice/usb3com.hh"
#include "Notice/NoticeNKFADC500.hh"

// open NKFADC500
int NKFADC500open(int sid, libusb_context * ctx)
{
  int status;

  status = USB3Open(NKFADC500_VENDOR_ID, NKFADC500_PRODUCT_ID, sid, ctx);
  USB3ClaimInterface(NKFADC500_VENDOR_ID, NKFADC500_PRODUCT_ID, sid, 0);

  // read USB endpoint garbage
  //unsigned char data[32*1024];
  //NKFADC500read_DATA(sid, 32, data, 1);

  return status;
}

// close NKFADC500
void NKFADC500close(int sid)
{
  USB3ReleaseInterface(NKFADC500_VENDOR_ID, NKFADC500_PRODUCT_ID, sid, 0);
  USB3Close(NKFADC500_VENDOR_ID, NKFADC500_PRODUCT_ID, sid);
}

// Read data buffer count, 1 buffer count = 1 kbyte data
int NKFADC500read_BCOUNT(int sid)
{
  return USB3ReadRegI(NKFADC500_VENDOR_ID, NKFADC500_PRODUCT_ID, sid, 0x30000000);
}

// read data, reads bcount * 1 kbytes data from NKFADC500 DRAM
// returns character raw data, needs sorting after data acquisition
int NKFADC500read_DATA(int sid, int bcount, unsigned char * data, unsigned int timeout)
{
  int retval = 0;

  // maximum data size is 64 Mbyte
  int count = bcount*256;

  if (timeout == 0)
    retval = USB3Read(NKFADC500_VENDOR_ID, NKFADC500_PRODUCT_ID, sid, count, 0x40000000, data);
  else
    retval = USB3ReadTimeout(NKFADC500_VENDOR_ID, NKFADC500_PRODUCT_ID, sid, count, 0x40000000, timeout, data);

  return retval;
}
