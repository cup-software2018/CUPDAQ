#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Notice/usb3com.hh"
#include "Notice/NoticeNKFADC500S.hh"

// open NKFADC500S
int NKFADC500Sopen(int sid, libusb_context *ctx)
{
  int status;

  status = USB3Open(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, ctx);
  USB3ClaimInterface(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0);

  // read USB endpoint garbage
  //unsigned char data[32*1024];
  //NKFADC500Sread_DATA(sid, 32, data, 1);

  return status;
}

// close NKFADC500S
void NKFADC500Sclose(int sid)
{
  USB3ReleaseInterface(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0);
  USB3Close(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid);
}

// reset data acquisition
void NKFADC500Sreset(int sid)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000000, 1 << 2);
}

// reset timer
void NKFADC500SresetTIMER(int sid)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000000, 1);
}

// start data acquisition
void NKFADC500Sstart(int sid)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000000, 1 << 3);
}

// stop data acquisition
void NKFADC500Sstop(int sid)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000000, 0 << 3);
}

// read RUN status
unsigned long NKFADC500Sread_RUN(int sid)
{
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000000);
}

// write coincidence window
void NKFADC500Swrite_CW(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000001 + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

// read coincidence windows
unsigned long NKFADC500Sread_CW(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000001 + (((ch - 1) & 0xFF) << 16);
  
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
}

// write recording length
void NKFADC500Swrite_RL(int sid, unsigned long data)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000002, data);
}

// read recording length
unsigned long NKFADC500Sread_RL(int sid)
{
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000002);
}

// turn on/off DRAM
// 0 = off, 1 = on
void NKFADC500Swrite_DRAMON(int sid, unsigned long data)
{
  unsigned long status;

  // turn on DRAM
  if (data) {
    // check DRAM is on
    status = USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000003);

    // when DRAM is on now, turn it off
    if (status)
      USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000003, 0);

    // turn on DRAM
    USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000003, 1);

    // wait for DRAM ready
    status = 0;
    while (!status)
      status = USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000003);
  }
  // trun off DRAM
  else
    USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000003, 0);
}

// read DRAM status
unsigned long NKFADC500Sread_DRAMON(int sid)
{
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000003);
}

// write offset adjustment
void NKFADC500Swrite_DACOFF(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000004 + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

// read offset adjustment
unsigned long NKFADC500Sread_DACOFF(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000004 + (((ch - 1) & 0xFF) << 16);
  
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
}

// measure pedestal
void NKFADC500Smeasure_PED(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000005 + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, 0);
}

// read pedestal
unsigned long NKFADC500Sread_PED(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000006 + (((ch - 1) & 0xFF) << 16);
  
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
}

// write input delay
void NKFADC500Swrite_DLY(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;
  unsigned long value;

  addr = 0x20000007 + (((ch - 1) & 0xFF) << 16);
  value = ((data / 1000) << 10) | (data % 1000);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, value);
}

// read input delay
unsigned long NKFADC500Sread_DLY(int sid, unsigned long ch)
{
  unsigned long addr;
  unsigned long value;
  unsigned long data;

  addr = 0x20000007 + (((ch - 1) & 0xFF) << 16);
  value = USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
  data = (value >> 10) * 1000 + (value & 0x3FF);
  
  return data;
}

// write discriminator threshold
void NKFADC500Swrite_THR(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000008 + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

// read discriminator threshold
unsigned long NKFADC500Sread_THR(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000008 + (((ch - 1) & 0xFF) << 16);
  
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
}

// write input pulse polarity
void NKFADC500Swrite_POL(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000009 + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

// read input pulse polarity
unsigned long NKFADC500Sread_POL(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000009 + (((ch - 1) & 0xFF) << 16);
  
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
}

// write pulse sum trigger width
void NKFADC500Swrite_PSW(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000A + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

// read pulse sum trigger width
unsigned long NKFADC500Sread_PSW(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000A + (((ch - 1) & 0xFF) << 16);
  
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
}

// write ADC mode
void NKFADC500Swrite_AMODE(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000B + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

// read ADC mode
unsigned long NKFADC500Sread_AMODE(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000B + (((ch - 1) & 0xFF) << 16);
  
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
}

// write pulse count threshold
void NKFADC500Swrite_PCT(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000C + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

// read pulse count threshold
unsigned long NKFADC500Sread_PCT(int sid, unsigned long ch)
{
  unsigned long addr;

 addr = 0x2000000C + (((ch - 1) & 0xFF) << 16);
  
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
}

// write pulse count interval
void NKFADC500Swrite_PCI(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000D + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

// write pulse count interval
unsigned long NKFADC500Sread_PCI(int sid, unsigned long ch)
{
  unsigned long addr;

 addr = 0x2000000D + (((ch - 1) & 0xFF) << 16);
  
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
}

// write pulse width threshold
void NKFADC500Swrite_PWT(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000E + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

// read pulse width threshold
unsigned long NKFADC500Sread_PWT(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000E + (((ch - 1) & 0xFF) << 16);
  
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
}

// write deadtime
void NKFADC500Swrite_DT(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000F + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

// read deadtime
unsigned long NKFADC500Sread_DT(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000F + (((ch - 1) & 0xFF) << 16);
  
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
}

// Read data buffer count, 1 buffer count = 1 kbyte data
unsigned long NKFADC500Sread_BCOUNT(int sid)
{
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000010);
}

// write pedestal trigger interval in ms;
void NKFADC500Swrite_PTRIG(int sid, unsigned long data)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000011, data);
}

// read pedestal trigger interval in ms;
unsigned long NKFADC500Sread_PTRIG(int sid)
{
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000011);
}

// send trigger
void NKFADC500Ssend_TRIG(int sid)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000012, 0);
}

// write trigger enable
void NKFADC500Swrite_TRIGENABLE(int sid, unsigned long data)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000013, data);
}

// read trigger enable
extern unsigned long NKFADC500Sread_TRIGENABLE(int sid)
{
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000013);
}

// write trigger mode
void NKFADC500Swrite_TM(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000014 + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

// read trigger mode
unsigned long NKFADC500Sread_TM(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000014 + (((ch - 1) & 0xFF) << 16);
  
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
}

// write trigger lookup table
void NKFADC500Swrite_TLT(int sid, unsigned long data)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000015, data);
}

// write trigger lookup table
unsigned long NKFADC500Sread_TLT(int sid)
{
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000015);
}

// write zero suppression
void NKFADC500Swrite_ZEROSUP(int sid, unsigned long ch, unsigned long data)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000016, data);
}

// read zero suppression
unsigned long NKFADC500Sread_ZEROSUP(int sid, unsigned long ch)
{
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000016);
}

// send ADC reset signal
void NKFADC500Ssend_ADCRST(int sid)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000017, 0);
}

// send ADC calibration signal
void NKFADC500Ssend_ADCCAL(int sid)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x20000018, 0);
}

// write ADC calibration delay
void NKFADC500Swrite_ADCDLY(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000019 + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

// write ADC align delay
void NKFADC500Swrite_ADCALIGN(int sid, unsigned long data)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x2000001A, data);
}

// read ADC status
unsigned long NKFADC500Sread_ADCSTAT(int sid)
{
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x2000001A);
}

// write DRAM calibration delay
void NKFADC500Swrite_DRAMDLY(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000001B + ((ch & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

// write DRAM bitslip
void NKFADC500Swrite_DRAMBITSLIP(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000001C + ((ch & 0xFF) << 16);

  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, 0);
}

// write DRAM test
void NKFADC500Swrite_DRAMTEST(int sid, unsigned long data)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x2000001D, data);
}

// read DRAM alignment
unsigned long NKFADC500Sread_DRAMTEST(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000001D + ((ch & 0xFF) << 16);

  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
}

// write self trigger prescale
void NKFADC500Swrite_PSCALE(int sid, unsigned long data)
{
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x2000001E, data);
}

// read self trigger prescale
extern unsigned long NKFADC500Sread_PSCALE(int sid)
{
  return USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, 0x2000001E);
}

// write ADC down-sampling rate
void NKFADC500Swrite_DSR(int sid, unsigned long data)
{
  unsigned long addr = 0x2000001F;
  USB3Write(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr, data);
}

unsigned long NKFADC500Sread_DSR(int sid)
{
  unsigned long data;
  unsigned long addr = 0x2000001F;
  data = USB3ReadReg(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, addr);
  return data;
}

// read data, reads bcount * 1 kbytes data from NKFADC500S DRAM
// returns character raw data, needs sorting after data acquisition
int NKFADC500Sread_DATA(int sid, int bcount, unsigned char *data, unsigned int timeout)
{
  int retval = 0;

  // maximum data size is 64 Mbyte
  int count = bcount*256;

  if (timeout == 0)
    retval = USB3Read(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, count, 0x40000000, data);
  else
    retval = USB3ReadTimeout(NKFADC500S_VENDOR_ID, NKFADC500S_PRODUCT_ID, sid, count, 0x40000000, timeout, data);

  return retval;
}

// align ADC for NKFADC500S
void NKFADC500S_ADCALIGN_500(int sid)
{
  unsigned long ch, dly, value;
  int count, sum, center;
  unsigned long gdly;
  int flag;

  NKFADC500Ssend_ADCRST(sid); 
  usleep(500000);
  NKFADC500Ssend_ADCCAL(sid); 
  NKFADC500Swrite_ADCALIGN(sid, 1);

  for (ch = 1; ch <= 4; ch++) {
    count = 0;
    sum = 0;
    flag = 0;

    for (dly = 0; dly < 32; dly++) {
      NKFADC500Swrite_ADCDLY(sid, ch, dly); 
      value = (NKFADC500Sread_ADCSTAT(sid) >> (ch -1)) & 0x1; 
      
      // count bad delay
      if (!value) {
        flag = 1;
        count = count + 1;
        sum = sum + dly;
      }
      else {
        if (flag) 
          dly = 32;
      }
    }

    // get bad delay center
    center = sum / count;

    // set good delay
    if (center < 11)
      gdly = center + 11;
    else
      gdly = center - 11;

    // set delay
    NKFADC500Swrite_ADCDLY(sid, ch, gdly); 
    printf("ch%ld calibration delay = %ld\n", ch, gdly);
  }

  NKFADC500Swrite_ADCALIGN(sid, 0); 
  NKFADC500Ssend_ADCCAL(sid); 
}

// align DRAM
void NKFADC500S_ADCALIGN_DRAM(int sid)
{
  unsigned long ch;
  unsigned long dly;
  unsigned long value;
  int flag;
  int count;
  int sum;
  int aflag;
  unsigned long gdly;
  int bitslip;

  // turn on DRAM
  NKFADC500Swrite_DRAMON(sid, 1);

  // enter DRAM test mode
  NKFADC500Swrite_DRAMTEST(sid, 1);

  // send reset to iodelay
  NKFADC500Ssend_ADCCAL(sid);

  // fill DRAM test pattern
  NKFADC500Swrite_DRAMTEST(sid, 2);

  for (ch = 0; ch < 8; ch++) {
    count = 0;
    sum = 0;
    flag = 0;

    // search delay
    for (dly = 0; dly < 32; dly++) {
      // set delay
      NKFADC500Swrite_DRAMDLY(sid, ch, dly);

      // read DRAM test pattern
      NKFADC500Swrite_DRAMTEST(sid, 3);
      value = NKFADC500Sread_DRAMTEST(sid, ch);

      aflag = 0;
      if (value == 0xFFAA5500)
        aflag = 1;
      else if (value == 0xAA5500FF)
        aflag = 1;
      else if (value == 0x5500FFAA)
        aflag = 1;
      else if (value == 0x00FFAA55)
        aflag = 1;

      if (aflag) {
        count = count + 1;
        sum = sum + dly;
        if (count > 4)
          flag = 1;
     }
      else {
        if (flag)
          dly = 32;
        else {
          count = 0;
          sum = 0;
        }
      }
    }

    // get bad delay center
    if (count)
      gdly = sum / count;
    else
      gdly = 9;

    // set delay
    NKFADC500Swrite_DRAMDLY(sid, ch, gdly);

    // get bitslip
    for (bitslip = 0; bitslip < 4; bitslip++) {
      // read DRAM test pattern
      NKFADC500Swrite_DRAMTEST(sid, 3);
      value = NKFADC500Sread_DRAMTEST(sid, ch);

      if (value == 0xFFAA5500) {
        aflag = 1;
        bitslip = 4;
      }
      else {
        aflag = 0;
        NKFADC500Swrite_DRAMBITSLIP(sid, ch);
      }
    }

    if (aflag)
      printf("DRAM(%ld) is aligned, delay = %ld\n", ch, gdly);
    else
      printf("Fail to align DRAM(%ld)!\n", ch);
  }

  // exit DRAM test mode
  NKFADC500Swrite_DRAMTEST(sid, 0);
}

