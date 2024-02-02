#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Notice/usb3com.hh"
#include "Notice/NoticeM64ADCS.hh"

// open M64ADCS
int M64ADCSopen(int sid, libusb_context * ctx)
{
  int status;

  status = USB3Open(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, ctx);
  USB3ClaimInterface(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0);

  return status;
}

// close M64ADCS
void M64ADCSclose(int sid)
{
  USB3ReleaseInterface(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0);
  USB3Close(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid);
}

// reset timer
void M64ADCSresetTIMER(int sid)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000000, 1);
}

// reset data acquisition
void M64ADCSreset(int sid)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000000, 1 << 2);
}

// start data acquisition
void M64ADCSstart(int sid)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000000, 1 << 3);
}

// stop data acquisition
void M64ADCSstop(int sid)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000000, 0 << 3);
}

// read RUN status
unsigned long M64ADCSread_RUN(int sid)
{
  return USB3ReadReg(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000000);
}

// write coincidence window
void M64ADCSwrite_CW(int sid, unsigned long data)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000001, data);
}

// read coincidence windows
unsigned long M64ADCSread_CW(int sid)
{
  return USB3ReadReg(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000001);
}

// turn on/off DRAM
// 0 = off, 1 = on
void M64ADCSwrite_DRAMON(int sid, unsigned long data)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000003, data);
}

// read DRAM status
unsigned long M64ADCSread_DRAMON(int sid)
{
  return USB3ReadReg(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000003);
}

// read pedestal
unsigned long M64ADCSread_PED(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000006 + (((ch - 1) & 0xFF) << 16);

  return USB3ReadReg(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, addr);
}

// write input delay
void M64ADCSwrite_DLY(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;
  unsigned long value;

  addr = 0x20000007 + (((ch - 1) & 0xFF) << 16);
  value = ((data/1000) << 10) | (data%1000);
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, addr, value);
}

// read input delay
unsigned long M64ADCSread_DLY(int sid, unsigned long ch)
{
  unsigned long addr;
  unsigned long value;
  unsigned long data;

  addr = 0x20000007 + (((ch - 1) & 0xFF) << 16);
  value = USB3ReadReg(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, addr);
  data = (value >> 10)*1000 + (value & 0x3FF);

  return data;
}

// write discriminator threshold
void M64ADCSwrite_THR(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000008 + (((ch - 1) & 0xFF) << 16);

  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, addr, data);
}

// read discriminator threshold
unsigned long M64ADCSread_THR(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000008 + (((ch - 1) & 0xFF) << 16);

  return USB3ReadReg(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, addr);
}

// write pulse sum width
void M64ADCSwrite_PSW(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000A + (((ch - 1) & 0xFF) << 16);
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, addr, data);
}

// read pulse sum width
unsigned long M64ADCSread_PSW(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000A + (((ch - 1) & 0xFF) << 16);
  return USB3ReadReg(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, addr);
}

// Read data buffer count, 1 buffer count = 1 kbyte data
int M64ADCSread_BCOUNT(int sid)
{
  return USB3ReadRegI(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000010);
}

// write pedestal trigger interval in ms;
void M64ADCSwrite_PTRIG(int sid, unsigned long data)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000011, data);
}

// read pedestal trigger interval in ms;
unsigned long M64ADCSread_PTRIG(int sid)
{
  return USB3ReadReg(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000011);
}

// send trigger
void M64ADCSsend_TRIG(int sid)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000012, 0);
}

// write trigger mode
void M64ADCSwrite_TM(int sid, unsigned long data)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000014, data);
}

// read trigger mode
unsigned long M64ADCSread_TM(int sid)
{
  return USB3ReadReg(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000014);
}

// set multiplicity threshold
void M64ADCSwrite_MTHR(int sid, unsigned long data)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000015, data);
}

// read multiplicity threshold
unsigned long M64ADCSread_MTHR(int sid)
{
  return USB3ReadReg(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000015);
}

// send ADC reset signal
void M64ADCSsend_ADCRST(int sid)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000017, 0);
}

// send ADC calibration signal
void M64ADCSsend_ADCCAL(int sid)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x20000018, 0);
}

// write ADC calibration delay
void M64ADCSwrite_ADCDLY(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000019 + (((ch - 1) & 0xFF) << 16);

  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, addr, data);
}

// write ADC align delay
void M64ADCSwrite_ADCALIGN(int sid, unsigned long data)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x2000001A, data);
}

// read ADC status
unsigned long M64ADCSread_ADCSTAT(int sid)
{
  return USB3ReadReg(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x2000001A);
}

// write ADC bitslip
void M64ADCSwrite_BITSLIP(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000001B + (((ch - 1) & 0xFF) << 16);

  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, addr, data);
}

// write FADC buffer mux
void M64ADCSwrite_FMUX(int sid, unsigned long data)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x2000001C, data);
}

// read FADC buffer mux
unsigned long M64ADCSread_FMUX(int sid)
{
  return USB3ReadReg(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x2000001C);
}

// arm FADC buffer
void M64ADCSarm_FADC(int sid)
{
  USB3Write(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x2000001D, 0);
}

// read FADC buffer status
unsigned long M64ADCSread_FREADY(int sid)
{
  return USB3ReadReg(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 0x2000001D);
}

// align M64ADCS
void M64ADCS_ADCALIGN_64(int sid)
{
  unsigned long ch, dly, value;
  int count, sum, center;
  unsigned long bitslip;
  unsigned long gdly;
  unsigned long gbitslip;
  int flag;

  M64ADCSsend_ADCRST(sid);
  usleep(500000);
  M64ADCSsend_ADCCAL(sid);

  for (ch = 1; ch <= 4; ch++) {
    count = 0;
    sum = 0;
    flag = 0;
    gbitslip = 0;

    // ADC initialization codes
    M64ADCSwrite_ADCALIGN(sid, 0x030002);
    usleep(100);
    M64ADCSwrite_ADCALIGN(sid, 0x010010);
    usleep(100);
    M64ADCSwrite_ADCALIGN(sid, 0xC78001);
    usleep(100);
    M64ADCSwrite_ADCALIGN(sid, 0xDE01C0);
    usleep(100);

    // set deskew pattern
    M64ADCSwrite_ADCALIGN(sid, 0x450001);

    // set bitslip = 0
    M64ADCSwrite_BITSLIP(sid, ch, 0);

    for (dly = 0; dly < 32; dly++) {
      M64ADCSwrite_ADCDLY(sid, ch, dly);
      value = (M64ADCSread_ADCSTAT(sid) >> (ch - 1)) & 0x1;

      // count bad delay
      if (!value) {
        flag = 1;
        count = count + 1;
        sum = sum + (int)dly;
      }
      else {
        if (flag)
          dly = 32;
      }
    }

    // get bad delay center
    center = sum/count;

    // set good delay
    if (center < 9)
      gdly = (unsigned long)(center + 9);
    else
      gdly = (unsigned long)(center - 9);

    // sets delay
    M64ADCSwrite_ADCDLY(sid, ch, gdly);

    // set sync pattern
    M64ADCSwrite_ADCALIGN(sid, 0x450002);
    usleep(100);

    for (bitslip = 0; bitslip < 12; bitslip++) {
      M64ADCSwrite_BITSLIP(sid, ch, bitslip);

      value = (M64ADCSread_ADCSTAT(sid) >> ((ch - 1) + 4)) & 0x1;
      if (value) {
        gbitslip = bitslip;
        bitslip = 12;
      }
    }

    // set good bitslip
    M64ADCSwrite_BITSLIP(sid, ch, gbitslip);

    printf("ch%ld calibration delay = %ld, bitslip = %ld\n", ch, gdly, gbitslip);
  }

  // set normal ADC mode
  M64ADCSwrite_ADCALIGN(sid, 0x450000);
  usleep(100);
  M64ADCSsend_ADCCAL(sid);
}

// read FADC buffer
void M64ADCSread_FADCBUF(int sid, unsigned long * data)
{
  unsigned char rdat[8196];
  unsigned long i;
  unsigned long tmp;

  USB3Read(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, 2049, 0x20008000, rdat);

  for (i = 0; i < 2048; i++) {
    data[i] = (unsigned long)(rdat[4*i] & 0xFF);
    tmp = (unsigned long)(rdat[4*i + 1] & 0xFF);
    data[i] = data[i] + (unsigned long)(tmp << 8);
    tmp = (unsigned long)(rdat[4*i + 2] & 0xFF);
    data[i] = data[i] + (unsigned long)(tmp << 16);
    tmp = (unsigned long)(rdat[4*i + 3] & 0xFF);
    data[i] = data[i] + (unsigned long)(tmp << 24);
  }
}

// read data, reads bcount * 1 kbytes data from M64ADCS DRAM
// returns character raw data, needs sorting after data acquisition
int M64ADCSread_DATA(int sid, int bcount, unsigned char * data)
{
  int count;

  // maximum data size is 64 Mbyte
  count = bcount*256;

  return USB3Read(M64ADCS_VENDOR_ID, M64ADCS_PRODUCT_ID, sid, count, 0x40000000, data);
}
