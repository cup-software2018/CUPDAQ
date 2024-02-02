#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Notice/usb3com.hh"
#include "Notice/NoticeM64ADC.hh"

// open M64ADC
int M64ADCopen(int sid, libusb_context * ctx)
{
  int status;

  status = USB3Open(M64ADC_VENDOR_ID, M64ADC_PRODUCT_ID, sid, ctx);
  USB3ClaimInterface(M64ADC_VENDOR_ID, M64ADC_PRODUCT_ID, sid, 0);

  return status;
}

// close M64ADC
void M64ADCclose(int sid)
{
  USB3ReleaseInterface(M64ADC_VENDOR_ID, M64ADC_PRODUCT_ID, sid, 0);
  USB3Close(M64ADC_VENDOR_ID, M64ADC_PRODUCT_ID, sid);
}

// Read data buffer count, 1 buffer count = 1 kbyte data
int M64ADCread_BCOUNT(int sid)
{
  return USB3ReadRegI(M64ADC_VENDOR_ID, M64ADC_PRODUCT_ID, sid, 0x30000000);
}

// read data, reads bcount * 1 kbytes data from M64ADC DRAM
// returns character raw data, needs sorting after data acquisition
int M64ADCread_DATA(int sid, int bcount, unsigned char * data)
{
  int count;

  // maximum data size is 64 Mbyte
  count = bcount*256;

  return USB3Read(M64ADC_VENDOR_ID, M64ADC_PRODUCT_ID, sid, count, 0x40000000, data);
}
