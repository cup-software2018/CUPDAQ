#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "DAQUtils/ELogger.hh"
#include "Notice/usb3com.hh"
#include "Notice/NoticeAMOREADC.hh"

// open AMOREADC
int AMOREADCopen(int sid, libusb_context * ctx)
{
  int status;

  status = USB3Open(AMOREADC_VENDOR_ID, AMOREADC_PRODUCT_ID, sid, ctx);
  USB3ClaimInterface(AMOREADC_VENDOR_ID, AMOREADC_PRODUCT_ID, sid, 0);

  while (1) {
    if (!USB3CheckFPGADoneNoAVR(AMOREADC_VENDOR_ID, AMOREADC_PRODUCT_ID, sid))
      printf(".");
    else
      break;

    usleep(500000);
  }
  printf("\n");

  USB3InitFPGANoAVR(AMOREADC_VENDOR_ID, AMOREADC_PRODUCT_ID, sid);

  AMOREADCread_BCOUNT(sid);
  AMOREADCread_BCOUNT(sid);
  AMOREADCread_BCOUNT(sid);
  AMOREADCread_BCOUNT(sid);

  ELogger::Instance(true)->Info(__func__, "now AMOREADC[%d] is ready.", sid);

  return status;
}

// close AMOREADC
void AMOREADCclose(int sid)
{
  USB3ReleaseInterface(AMOREADC_VENDOR_ID, AMOREADC_PRODUCT_ID, sid, 0);
  USB3Close(AMOREADC_VENDOR_ID, AMOREADC_PRODUCT_ID, sid);
}

// Read data buffer count, 1 buffer count = 1 kbyte data
int AMOREADCread_BCOUNT(int sid)
{
  return USB3ReadRegI(AMOREADC_VENDOR_ID, AMOREADC_PRODUCT_ID, sid, 0x0B);
}

// read data, reads bcount * 1 kbytes data from AMOREADC DRAM
// returns character raw data, needs sorting after data acquisition
int AMOREADCread_DATA(int sid, int bcount, unsigned char * data)
{
  int count;

  // maximum data size is 64 Mbyte
  count = bcount*256;

  return USB3Read(AMOREADC_VENDOR_ID, AMOREADC_PRODUCT_ID, sid, count, 0x40000000, data);
}
