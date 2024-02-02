#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Notice/usb3com.hh"
#include "Notice/NoticeNKFADC125S.hh"

// open NKFADC125S
int NKFADC125Sopen(int sid, libusb_context * ctx)
{
  int status;

  status = USB3Open(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, ctx);
  USB3ClaimInterface(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0);

  return status;
}

// close NKFADC125S
void NKFADC125Sclose(int sid)
{
  USB3ReleaseInterface(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0);
  USB3Close(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid);
}

// reset data acquisition
void NKFADC125Sreset(int sid)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000000, 1 << 2);
}

// reset timer
void NKFADC125SresetTIMER(int sid)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000000, 1);
}

// start data acquisition
void NKFADC125Sstart(int sid)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000000, 1 << 3);
}

// stop data acquisition
void NKFADC125Sstop(int sid)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000000, 0 << 3);
}

// read RUN status
unsigned long NKFADC125Sread_RUN(int sid)
{
  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000000);
}

// write coincidence window
void NKFADC125Swrite_CW(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000001 + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
}

// read coincidence windows
unsigned long NKFADC125Sread_CW(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000001 + (((ch - 1) & 0xFF) << 16);

  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr);
}

// write recording length
void NKFADC125Swrite_RL(int sid, unsigned long data)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000002, data);
}

// read recording length
unsigned long NKFADC125Sread_RL(int sid)
{
  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000002);
}

// turn on/off DRAM
// 0 = off, 1 = on
void NKFADC125Swrite_DRAMON(int sid, unsigned long data)
{
  unsigned long status;

  // turn on DRAM
  if (data) {
    // check DRAM is on
    status = USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000003);

    // when DRAM is on now, turn it off
    if (status) {
      USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000003, 0);
    }

    // turn on DRAM
    USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000003, 1);

    // wait for DRAM ready
    status = 0;
    while (!status) {
      status = USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000003);
    }
  }
    // trun off DRAM
  else {
    USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000003, 0);
  }
}

// read DRAM status
unsigned long NKFADC125Sread_DRAMON(int sid)
{
  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000003);
}

// write offset adjustment
void NKFADC125Swrite_DACOFF(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;
  unsigned long nch;
  unsigned long chan;

  if (ch) {
    addr = 0x20000004 + (((ch - 1) & 0xFF) << 16);
    USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
  }
  else {
    nch = NKFADC125Sread_CHNUM(sid);
    for (chan = 1; chan <= nch; chan++) {
      addr = 0x20000004 + (((chan - 1) & 0xFF) << 16);
      USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
    }
  }
}

// read offset adjustment
unsigned long NKFADC125Sread_DACOFF(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000004 + (((ch - 1) & 0xFF) << 16);

  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr);
}

// measure pedestal
void NKFADC125Smeasure_PED(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000005 + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, 0);
}

// read pedestal
unsigned long NKFADC125Sread_PED(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000006 + (((ch - 1) & 0xFF) << 16);

  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr);
}

// write input delay
void NKFADC125Swrite_DLY(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;
  unsigned long value;

  addr = 0x20000007;
  value = ((data/1000) << 10) | (data%1000);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, value);
}

// read input delay
unsigned long NKFADC125Sread_DLY(int sid, unsigned long ch)
{
  unsigned long addr;
  unsigned long value;
  unsigned long data;

  addr = 0x20000007;
  value = USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr);
  data = (value >> 10)*1000 + (value & 0x3FF);

  return data;
}

// write discriminator threshold
void NKFADC125Swrite_THR(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000008 + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
}

// read discriminator threshold
unsigned long NKFADC125Sread_THR(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000008 + (((ch - 1) & 0xFF) << 16);

  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr);
}

// write input pulse polarity
void NKFADC125Swrite_POL(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000009 + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
}

// read input pulse polarity
unsigned long NKFADC125Sread_POL(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000009 + (((ch - 1) & 0xFF) << 16);

  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr);
}

// write pulse sum trigger width
void NKFADC125Swrite_PSW(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000A + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
}

// read pulse sum trigger width
unsigned long NKFADC125Sread_PSW(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000A + (((ch - 1) & 0xFF) << 16);

  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr);
}

// write pulse count threshold
void NKFADC125Swrite_PCT(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000C + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
}

// read pulse count threshold
unsigned long NKFADC125Sread_PCT(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000C + (((ch - 1) & 0xFF) << 16);

  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr);
}

// write pulse count interval
void NKFADC125Swrite_PCI(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000D + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
}

// write pulse count interval
unsigned long NKFADC125Sread_PCI(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000D + (((ch - 1) & 0xFF) << 16);

  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr);
}

// write pulse width threshold
void NKFADC125Swrite_PWT(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000E + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
}

// read pulse width threshold
unsigned long NKFADC125Sread_PWT(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000E + (((ch - 1) & 0xFF) << 16);

  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr);
}

// write deadtime
void NKFADC125Swrite_DT(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000F + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
}

// read deadtime
unsigned long NKFADC125Sread_DT(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000F + (((ch - 1) & 0xFF) << 16);

  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr);
}

// Read data buffer count, 1 buffer count = 1 kbyte data
unsigned long NKFADC125Sread_BCOUNT(int sid)
{
  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000010);
}

// write pedestal trigger interval in ms;
void NKFADC125Swrite_PTRIG(int sid, unsigned long data)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000011, data);
}

// read pedestal trigger interval in ms;
unsigned long NKFADC125Sread_PTRIG(int sid)
{
  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000011);
}

// send trigger
void NKFADC125Ssend_TRIG(int sid)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000012, 0);
}

// read # of channel 
unsigned long NKFADC125Sread_CHNUM(int sid)
{
  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000012);
}

// write trigger enable
void NKFADC125Swrite_TRIGENABLE(int sid, unsigned long data)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000013, data);
}

// read trigger enable
unsigned long NKFADC125Sread_TRIGENABLE(int sid)
{
  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000013);
}

// write trigger mode
void NKFADC125Swrite_TM(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000014 + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
}

// read trigger mode
unsigned long NKFADC125Sread_TM(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000014 + (((ch - 1) & 0xFF) << 16);

  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr);
}

// write trigger lookup table(module's multiplicity threshold)
void NKFADC125Swrite_TLT(int sid, unsigned long data)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000015, data);
}

// read trigger lookup table(module's multiplicity threshold)
unsigned long NKFADC125Sread_TLT(int sid)
{
  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000015);
}

// send ADC reset signal
void NKFADC125Ssend_ADCRST(int sid)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000017, 0);
}

// send ADC calibration signal
void NKFADC125Ssend_ADCCAL(int sid)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000018, 0);
}

// write ADC calibration delay
void NKFADC125Swrite_ADCDLY(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000019 + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
}

// write ADC align delay
void NKFADC125Swrite_ADCsetup(int sid, unsigned long addr, unsigned long data)
{
  unsigned long value;

  value = (addr << 8) | (data & 0xFF);
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x2000001A, value);
}

// read ADC status
unsigned long NKFADC125Sread_ADCSTAT(int sid)
{
  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x2000001A);
}

// write DRAM calibration delay
void NKFADC125Swrite_DRAMDLY(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000001B + ((ch & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
}

// write DRAM bitslip
void NKFADC125Swrite_DRAMBITSLIP(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000001C + ((ch & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, 0);
}

// write DRAM test 
void NKFADC125Swrite_DRAMTEST(int sid, unsigned long data)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x2000001D, data);
}

// read DRAM alignment
unsigned long NKFADC125Sread_DRAMTEST(int sid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000001D + ((ch & 0xFF) << 16);

  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr);
}

// write ADC bitslip
void NKFADC125Swrite_ADCBITSLIP(int sid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000001E + (((ch - 1) & 0xFF) << 16);

  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, addr, data);
}

// write self trigger prescale
void NKFADC125Swrite_PSCALE(int sid, unsigned long data)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x20000006, data);
}

// read self trigger prescale
unsigned long NKFADC125Sread_PSCALE(int sid)
{
  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x2000001E);
}


// write sampling rate /1/2/4/8;
void NKFADC125Swrite_DSR(int sid, unsigned long data)
{
  USB3Write(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x2000001F, data);
}

// read sampling rate
unsigned long NKFADC125Sread_DSR(int sid)
{
  return USB3ReadReg(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, 0x2000001F);
}

// read data, reads bcount * 1 kbytes data from NKFADC125S DRAM
// returns character raw data, needs sorting after data acquisition
unsigned long NKFADC125Sread_DATA(int sid, int bcount, unsigned char * data)
{
  int count;

  // maximum data size is 64 Mbyte
  count = bcount*256;

  return USB3Read(NKFADC125S_VENDOR_ID, NKFADC125S_PRODUCT_ID, sid, count, 0x40000000, data);
}

// align ADC for NKFADC125S
void NKFADC125S_ADCALIGN_125(int sid)
{
  int mini;
  unsigned long nch;
  unsigned long ch;
  unsigned long dly;
  unsigned long value;
  int bit_okay;
  int word_okay;
  int flag;
  int count;
  int sum;
  unsigned long gdly;
  unsigned long bitslip;
  unsigned long gbitslip;

  nch = NKFADC125Sread_CHNUM(sid);
  if (nch > 4) {
    mini = 0;
  }
  else {
    mini = 1;
  }

  NKFADC125Ssend_ADCRST(sid);
  usleep(1000000);
  NKFADC125Ssend_ADCCAL(sid);

  // ADC initialization codes
  NKFADC125Swrite_ADCsetup(sid, 0x009, 0x02);

  for (ch = 1; ch <= nch; ch++) {
    // set deskew pattern
    NKFADC125Swrite_ADCsetup(sid, 0x0006, 0x02);
    NKFADC125Swrite_ADCsetup(sid, 0x000A, 0x33);
    NKFADC125Swrite_ADCsetup(sid, 0x000B, 0x33);

    // bitslip = 0;
    NKFADC125Swrite_ADCBITSLIP(sid, ch, 0);

    sum = 0;
    count = 0;
    flag = 0;
    gbitslip = 0;

    for (dly = 0; dly < 32; dly++) {
      // set ADC delay
      NKFADC125Swrite_ADCDLY(sid, ch, dly);

      // check word alignment
      value = NKFADC125Sread_ADCSTAT(sid);
      bit_okay = (value >> (ch - 1)) & 0x1;

      if (bit_okay) {
        count = count + 1;
        sum = sum + dly;
        if (count > 5) {
          flag = flag + 1;
        }
      }
      else {
        if (flag) {
          dly = 32;
        }
        else {
          sum = 0;
          count = 0;
        }
      }
    }

    // get good center
    if (count) {
      gdly = sum/count;
    }
    else {
      gdly = 0;
    }

    // set good delay
    NKFADC125Swrite_ADCDLY(sid, ch, gdly);

    // set sync pattern
    NKFADC125Swrite_ADCsetup(sid, 0x000E, 0x81);
    NKFADC125Swrite_ADCsetup(sid, 0x000F, 0x00);
    NKFADC125Swrite_ADCsetup(sid, 0x000A, 0x55);
    NKFADC125Swrite_ADCsetup(sid, 0x000B, 0x55);

    for (bitslip = 0; bitslip < 7; bitslip++) {
      if (!mini) {
        // set bitslip
        NKFADC125Swrite_ADCBITSLIP(sid, ch, bitslip);
      }

      // check word alignment
      value = NKFADC125Sread_ADCSTAT(sid);
      word_okay = (value >> (ch + 15)) & 0x1;

      if (word_okay) {
        flag = flag + 1;
        gbitslip = bitslip;
        bitslip = 7;
      }
      else {
        if (mini) {
          NKFADC125Swrite_ADCBITSLIP(sid, ch, bitslip);
        }
      }
    }

    if (flag > 1) {
      printf("ADC(%ld) is aligned, delay = %ld, bitslip = %ld\n", ch, gdly, gbitslip);
    }
    else {
      printf("Fail to align ADC(%ld)!\n", ch);
    }
  }

  //set ADC normal operation
  NKFADC125Swrite_ADCsetup(sid, 0x122, 0x02);
  NKFADC125Swrite_ADCsetup(sid, 0x222, 0x02);
  NKFADC125Swrite_ADCsetup(sid, 0x422, 0x02);
  NKFADC125Swrite_ADCsetup(sid, 0x522, 0x02);

  NKFADC125Swrite_ADCsetup(sid, 0x0009, 0x01);
  NKFADC125Swrite_ADCsetup(sid, 0x0006, 0x00);
  NKFADC125Swrite_ADCsetup(sid, 0x000A, 0x00);
  NKFADC125Swrite_ADCsetup(sid, 0x000B, 0x00);
}

// align DRAM
void NKFADC125S_ADCALIGN_DRAM(int sid)
{
  int mini;
  unsigned long ch;
  unsigned long dly;
  unsigned long value;
  int flag;
  int count;
  int sum;
  int aflag;
  unsigned long gdly;
  int bitslip;
  int gbitslip;

  value = NKFADC125Sread_CHNUM(sid);
  if (value > 4) {
    mini = 0;
  }
  else {
    mini = 1;
  }

  // turn on DRAM    
  NKFADC125Swrite_DRAMON(sid, 1);

  if (mini) {
    printf("DRAM is aligned\n");
  }
  else {
    // enter DRAM test mode
    NKFADC125Swrite_DRAMTEST(sid, 1);

    // send reset to iodelay  
    NKFADC125Ssend_ADCCAL(sid);

    // fill DRAM test pattern
    NKFADC125Swrite_DRAMTEST(sid, 2);

    for (ch = 0; ch < 8; ch++) {
      count = 0;
      sum = 0;
      flag = 0;

      // search delay
      for (dly = 0; dly < 32; dly++) {
        // set delay
        NKFADC125Swrite_DRAMDLY(sid, ch, dly);

        // read DRAM test pattern
        NKFADC125Swrite_DRAMTEST(sid, 3);
        value = NKFADC125Sread_DRAMTEST(sid, ch);

        aflag = 0;
        if (value == 0xFFAA5500) {
          aflag = 1;
        }
        else if (value == 0xAA5500FF) {
          aflag = 1;
        }
        else if (value == 0x5500FFAA) {
          aflag = 1;
        }
        else if (value == 0x00FFAA55) {
          aflag = 1;
        }

        if (aflag) {
          count = count + 1;
          sum = sum + dly;
          if (count > 4) {
            flag = 1;
          }
        }
        else {
          if (flag) {
            dly = 32;
          }
          else {
            count = 0;
            sum = 0;
          }
        }
      }

      // get bad delay center
      if (count) {
        gdly = sum/count;
      }
      else {
        gdly = 9;
      }

      // set delay
      NKFADC125Swrite_DRAMDLY(sid, ch, gdly);

      // get bitslip
      for (bitslip = 0; bitslip < 4; bitslip++) {
        // read DRAM test pattern
        NKFADC125Swrite_DRAMTEST(sid, 3);
        value = NKFADC125Sread_DRAMTEST(sid, ch);

        if (value == 0xFFAA5500) {
          aflag = 1;
          gbitslip = bitslip;
          bitslip = 4;
        }
        else {
          aflag = 0;
          NKFADC125Swrite_DRAMBITSLIP(sid, ch);
        }
      }

      if (aflag) {
        printf("DRAM(%ld) is aligned, delay = %ld, bitslip = %d\n", ch, gdly, gbitslip);
      }
      else {
        printf("Fail to align DRAM(%ld)!\n", ch);
      }
    }

    // exit DRAM test mode
    NKFADC125Swrite_DRAMTEST(sid, 0);
  }
}

// flush remaining data
void NKFADC125Sflush_DATA(int sid)
{
  // max buffer size is 10 MB
  unsigned char data[10485760];
  unsigned long bcount;
  unsigned long chunk;
  unsigned long slice;
  unsigned long i;

  // check data size to be flushed
  bcount = NKFADC125Sread_BCOUNT(sid);
  chunk = bcount/10240;
  slice = bcount%10240;

  for (i = 0; i < chunk; i++) {
    NKFADC125Sread_DATA(sid, 10240, data);
  }

  if (slice) {
    NKFADC125Sread_DATA(sid, slice, data);
  }
}


