#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "Notice/usb3tcb.hh"
#include "Notice/NoticeTCB.hh"

// open TCB
int TCBopen(int sid, libusb_context* ctx)
{
  int status;

  status = USB3TCBOpen(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, ctx);
  USB3TCBClaimInterface(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0);

  return status;
}

// close TCB
void TCBclose(int sid)
{
  USB3TCBReleaseInterface(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0);
  USB3TCBClose(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid);
}

// write lookup table ; TFADC500, TFADC64, AMOREADC, TCB
void TCBwrite_LT(int sid, unsigned long mid, char* data, int len)
{
  int nword = len / 4;
  uint32_t buffer;
  int loop;

  for(loop = 0; loop < nword; loop++) {
    memcpy(&buffer, data + loop * 4, 4);
    USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, loop, buffer);
  }
}

// reset data acquisition
void TCBreset(int sid)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000000, 1 << 2);
}

// reset timer
void TCBresetTIMER(int sid)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000000, 1);
}

// start data acquisition
void TCBstart(int sid)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000000, 1 << 3);
}

// stop data acquisition
void TCBstop(int sid)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000000, 0 << 3);
}

// read RUN status
unsigned long TCBread_RUN(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr = 0x20000000;

  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write coincidence window ; TFADC500, TFADC64, AMOREADC, cw
void TCBwrite_CW(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000001;

  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_CW(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000001;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write segment setting ; TFADC500, AMOREADC
void TCBwrite_RL(int sid, unsigned long mid, unsigned long data)
{
  unsigned long addr = 0x20000002;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_RL(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr = 0x20000002;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write Gate width
void TCBwrite_GW(int sid, unsigned long mid, unsigned long data)
{
  unsigned long addr = 0x20000002;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_GW(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr = 0x20000002;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// turn on/off DRAM ; TFADC500, TFADC64, AMOREADC
// 0 = off, 1 = on
void TCBwrite_DRAMON(int sid, unsigned long mid, unsigned long data)
{
  unsigned long addr;
  unsigned long status;

  if (mid)
    addr = 0x20000003;
  else
    addr = 0x2000003E;

  // turn on DRAM
  if (data) {
    // check DRAM is on
    status = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);

    // when DRAM is on now, turn it off
    if (status)
      USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, 0);

    // turn on DRAM
    USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, 1);

    // wait for DRAM ready
    status = 0;
    while (!status)
      status = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  }
  // trun off DRAM
  else 
    USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, 0);
}

// read DRAM status ; TFADC500, TFADC64, AMOREADC
unsigned long TCBread_DRAMON(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr;

  if (mid)
    addr = 0x20000003;
  else
    addr = 0x2000003E;

  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write offset adjustment ; TFADC500, AMOREADC
void TCBwrite_DACOFF(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000004;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
  sleep(1);
}
unsigned long TCBread_DACOFF(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000004;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// measure pedestal ; TFADC500, AMOREADC
void TCBmeasure_PED(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long addr = 0x20000005;

  if(mid > 0)
    addr += ((ch-1) & 0xFF) << 16;

  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, 0);
}

// read pedestal ; TFADC500, TFADC64, AMOREADC
unsigned long TCBread_PED(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000006;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write input delay ; TFADC500, TFADC64, AMOREADC
void TCBwrite_DLY(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000007;
  unsigned long value;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  value = ((data / 1000) << 10) | (data % 1000);

  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, value);
}
unsigned long TCBread_DLY(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long value;
  unsigned long data;
  unsigned long addr = 0x20000007;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  value = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  data = (value >> 10) * 1000 + (value & 0x3FF);
  
  return data;
}

// write input delay ; TFADC500, TFADC64, AMOREADC
void TCBwrite_AMOREDLY(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000007;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;

  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_AMOREDLY(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000007;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  
  return data;
}

// write discriminator threshold ; TFADC500, TFADC64, AMOREADC
void TCBwrite_THR(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000008;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_THR(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000008;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write input pulse polarity ; TFADC500, TFADC64
void TCBwrite_POL(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000009;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_POL(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000009;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write pulse sum trigger width ; TFADC500, TFADC64
void TCBwrite_PSW(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x2000000A;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_PSW(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x2000000A;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write ADC mode ; TFADC500, TFADC64
void TCBwrite_AMODE(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x2000000B;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_AMODE(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x2000000B;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write pulse count threshold ; TFADC500
void TCBwrite_PCT(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x2000000C;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_PCT(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x2000000C;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write pulse count interval ; TFADC500
void TCBwrite_PCI(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x2000000D;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_PCI(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x2000000D;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write pulse width threshold ; TFADC500
void TCBwrite_PWT(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x2000000E;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_PWT(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x2000000E;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write peak sum scale - peak sum out = peak area/peak sum scale ; TFADC64
void TCBwrite_PSS(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000010;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_PSS(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000010;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write rise time, rise time = set value * 2 * sampling period ; AMOREADC
void TCBwrite_RT(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000011;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_RT(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000011;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write sampling rate, sampling rate = 1 MHz / setting value; AMOREADC
void TCBwrite_SR(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000012;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_SR(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000012;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write gain, value is 1/2/4
void TCBwrite_DACGAIN(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000013;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_DACGAIN(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000013;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write trigger mode ; TFADC500
void TCBwrite_TM(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000014;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_TM(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000014;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write trigger lookup table ; TFADC500, TFADC64(multiplicity)
void TCBwrite_TLT(int sid, unsigned long mid, unsigned long data)
{
  unsigned long addr = 0x20000015;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_TLT(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr = 0x20000015;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write trigger lookup table ; TFADC64 only; 
// to write lookup table ch 0~3, put ch any value from 0 to 3, for example
void TCBwrite_STLT(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000015;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_STLT(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000015;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write zero suppression ; TFADC500
void TCBwrite_ZEROSUP(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000016;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_ZEROSUP(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000016;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// send ADC reset signal ; TFADC500, TFADC64
void TCBsend_ADCRST(int sid, unsigned long mid)
{
  unsigned long addr = 0x20000017;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, 0);
}

// send ADC calibration signal; TFADC500, TFADC64
void TCBsend_ADCCAL(int sid, unsigned long mid)
{
  unsigned long addr;
  if(mid > 0)
    addr = 0x20000018;
  else
    addr = 0x20000040;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, 0);
}

// write ADC calibration delay; TFADC500, TFADC64
void TCBwrite_ADCDLY(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000019;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}

// write ADC align delay; TFADC500, TFADC64
void TCBwrite_ADCALIGN(int sid, unsigned long mid, unsigned long data)
{
  unsigned long addr = 0x2000001A;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}

// read ADC status; TFADC500, TFADC64
unsigned long TCBread_ADCSTAT(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr = 0x2000001A;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write BitSlip : TFADC64
void TCBwrite_BITSLIP(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x2000001B;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}

// write flash ADC buffer mux ; TFADC64
void TCBwrite_FMUX(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long addr = 0x2000001C;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, ch - 1);
}
unsigned long TCBread_FMUX(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr = 0x2000001C;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// arm flash ADC buffer, read buffer ready ; TFADC64
void TCBarm_FADC(int sid, unsigned long mid)
{
  unsigned long addr = 0x2000001D;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, 0);
}
unsigned long TCBread_FREADY(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr = 0x2000001D;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write zerosuppression flag duration (in ns 8 ~ 65532)
void TCBwrite_ZSFD(int sid, unsigned long mid, unsigned long data)
{
  unsigned long addr = 0x2000001E;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_ZSFD(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr = 0x2000001E;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write ADC down-sampling rate
void TCBwrite_DSR(int sid, unsigned long mid, unsigned long data)
{
  unsigned long addr = 0x2000001F;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_DSR(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr = 0x2000001F;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write signal window length in # of ADC samples
void TCBwrite_ST(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000020;
  if(mid>0)
    addr += ((ch-1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_ST(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000020;
  if(mid>0)
    addr += ((ch-1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write pedestal window length in # of ADC samples
void TCBwrite_PT(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000021;
  if(mid>0)
    addr += ((ch-1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_PT(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000021;
  if(mid>0)
    addr += ((ch-1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write DRAM calibration delay
void TCBwrite_DRAMDLY(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;
  if(mid > 0)
    addr = 0x20000022 + ((ch & 0xFF) << 16);
  else
    addr = 0x20000041;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}

// write DRAM bitslip
void TCBwrite_DRAMBITSLIP(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long addr;
  if(mid > 0)
    addr = 0x20000023 + ((ch & 0xFF) << 16);
  else
    addr = 0x20000042;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, 0);
}

// write DRAM test 
void TCBwrite_DRAMTEST(int sid, unsigned long mid, unsigned long data)
{
  unsigned long addr;
  if(mid > 0)
    addr = 0x20000024;
  else
    addr = 0x2000003F;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}

// read DRAM alignment
unsigned long TCBread_DRAMTEST(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr;
  if(mid > 0)
    addr = 0x20000024 + ((ch & 0xFF) << 16);
  else
    addr = 0x2000003F;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write DAQ mode
void TCBwrite_DAQMODE(int sid, unsigned long mid, unsigned long data)
{
  unsigned long addr = 0x20000025;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_DAQMODE(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr = 0x20000025;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write high voltage
void TCBwrite_HV(int sid, unsigned long mid, unsigned long ch, float data)
{
  float fval;
  int value;
  unsigned long addr = 0x20000026;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  fval = 4.49 * (data - 3.2);
  value = (int)(fval);
  if (value > 254)
    value = 254;
  else if (value < 0)
    value = 0;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, value);
}
float TCBread_HV(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  float value;
  unsigned long addr = 0x20000026;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  value = data;
  value = value / 4.49 + 3.2;
  return value;
}

// read temperature
float TCBread_TEMP(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  float value;
  unsigned long addr = 0x20000027;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);

  value = data;
  value = value * 250.0 / 4096.0 - 50.0;
  value = value * 5.7 / 4.7;

  return value;
}

// write ADC mux
void TCBwrite_ADCMUX(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000028;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}

// read flash ADC buffer : TFADC64
void TCBread_FADCBUF(int sid, unsigned long mid, unsigned long *data)
{
  unsigned long addr = 0x20008000;
  unsigned long i;

  for (i = 0; i < 2048; i++) 
    data[i] = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr + i);
}

// align ADC for NKFADC500
void TCB_ADCALIGN_500(int sid, unsigned long mid)
{
  unsigned long ch, dly, value;
  int count, sum, center;
  unsigned long gdly;
  int flag;

  TCBsend_ADCRST(sid, mid); 
  usleep(500000);
  TCBsend_ADCCAL(sid, mid); 
  TCBwrite_ADCALIGN(sid, mid, 1);

  for (ch = 1; ch <= 4; ch++) {
    count = 0;
    sum = 0;
    flag = 0;

    for (dly = 0; dly < 32; dly++) {
      TCBwrite_ADCDLY(sid, mid, ch, dly); 
      value = (TCBread_ADCSTAT(sid, mid) >> (ch -1)) & 0x1; 
      
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
    TCBwrite_ADCDLY(sid, mid, ch, gdly); 
    printf("ch%ld calibration delay = %ld\n", ch, gdly);
  }

  TCBwrite_ADCALIGN(sid, mid, 0); 
  TCBsend_ADCCAL(sid, mid); 
}

// align ADC for M64ADC
void TCB_ADCALIGN_64(int sid, unsigned long mid)
{
  unsigned long ch, dly, value;
  int count, sum, center;
  unsigned long bitslip;
  unsigned long gdly;
  unsigned long gbitslip;
  int flag;

  TCBsend_ADCRST(sid, mid); 
  usleep(500000);
  TCBsend_ADCCAL(sid, mid); 

  for (ch = 1; ch <= 4; ch++) {
    count = 0;
    sum = 0;
    flag = 0;
    gbitslip = 0;

    // ADC initialization codes
    TCBwrite_ADCALIGN(sid, mid, 0x030002); 
    usleep(100);
    TCBwrite_ADCALIGN(sid, mid, 0x010010); 
    usleep(100);
    TCBwrite_ADCALIGN(sid, mid, 0xC78001); 
    usleep(100);
    TCBwrite_ADCALIGN(sid, mid, 0xDE01C0); 
    usleep(100);

    // set deskew pattern
    TCBwrite_ADCALIGN(sid, mid, 0x450001); 

    // set bitslip = 0
    TCBwrite_BITSLIP(sid, mid, ch, 0); 

    for (dly = 0; dly < 32; dly++) {
      TCBwrite_ADCDLY(sid, mid, ch, dly); 
      value = (TCBread_ADCSTAT(sid, mid) >> (ch -1)) & 0x1; 
      
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
    if (center < 9)
      gdly = center + 9;
    else
      gdly = center - 9;

    // sets delay
    TCBwrite_ADCDLY(sid, mid, ch, gdly); 

    // set sync pattern
    TCBwrite_ADCALIGN(sid, mid, 0x450002); 
    usleep(100);

    for (bitslip = 0; bitslip < 12; bitslip++) {
      TCBwrite_BITSLIP(sid, mid, ch, bitslip); 

      value = (TCBread_ADCSTAT(sid, mid) >> ((ch -1) + 4)) & 0x1; 
      if (value) {
        gbitslip = bitslip;
        bitslip = 12;
      }
    }

    // set good bitslip
    TCBwrite_BITSLIP(sid, mid, ch, gbitslip); 

    printf("ch%ld calibration delay = %ld, bitslip = %ld\n", ch, gdly, gbitslip);
  }

  // set normal ADC mode
  TCBwrite_ADCALIGN(sid, mid, 0x450000); 
  usleep(100);
  TCBsend_ADCCAL(sid, mid); 
}

// Align DRAM input
void TCB_ADCALIGN_DRAM(int sid, unsigned long mid)
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
  int gbitslip;

  // turn on DRAM    
  TCBwrite_DRAMON(sid, mid, 1);

  // enter DRAM test mode
  TCBwrite_DRAMTEST(sid, mid, 1);

  // send reset to iodelay  
  TCBsend_ADCCAL(sid, mid);

  // fill DRAM test pattern
  TCBwrite_DRAMTEST(sid, mid, 2);

  for (ch = 0; ch < 8; ch++) {
    count = 0;
    sum = 0;
    flag = 0;

    // search delay
    for (dly = 0; dly < 32; dly++) {
      // set delay
      TCBwrite_DRAMDLY(sid, mid, ch, dly);

      // read DRAM test pattern
      TCBwrite_DRAMTEST(sid, mid, 3);
      value = TCBread_DRAMTEST(sid, mid, ch);
//printf("ch = %ld, dly = %ld, value = %lX\n", ch, dly, value);      

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
//      gdly = 0;

    // set delay
    TCBwrite_DRAMDLY(sid, mid, ch, gdly); 
  
    // get bitslip
    for (bitslip = 0; bitslip < 4; bitslip++) {
      // read DRAM test pattern
      TCBwrite_DRAMTEST(sid, mid, 3);
      value = TCBread_DRAMTEST(sid, mid, ch);
//printf("ch = %ld, bitslip = %d, value = %lX\n", ch, bitslip, value);      

      if (value == 0xFFAA5500) {
        aflag = 1;
        gbitslip = bitslip;
        bitslip = 4;
      }
      else {
        aflag = 0;
        TCBwrite_DRAMBITSLIP(sid, mid, ch);
      }
    }

    if (aflag)
      printf("DRAM(%ld) is aligned, delay = %ld, bitslip = %d\n", ch, gdly, gbitslip);
    else
      printf("Fail to align DRAM(%ld)!\n", ch);
  }
   
  // exit DRAM test mode
  TCBwrite_DRAMTEST(sid, mid, 0);
}

// write run number ; TCB
void TCBwrite_RUNNO(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000002, data);
}
unsigned long TCBread_RUNNO(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000002);
  return data;
}

// write gate delay ; TCB; 0 ~ 1,048,568 ns
void TCBwrite_GATEDLY(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000003, data);
}
unsigned long TCBread_GATEDLY(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000003);
  return data;
}

// send trigger ; TCB
void TCBsend_TRIG(int sid)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000004, 0);
}

// read link status ; TCB
void TCBread_LNSTAT(int sid, unsigned long *data)
{
  unsigned long addr = 0x20000005;
  unsigned char rdat[8];
  unsigned long ltmp;
  int i;
  USB3TCBRead(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 2, addr, rdat);
  for (i=0; i <2; i++) {
    data[i] = rdat[i * 4] & 0xFF;
    ltmp = rdat[i * 4 + 1] & 0xFF;
    data[i] = data[i] + (unsigned long)(ltmp << 8);
    ltmp = rdat[i * 4 + 2] & 0xFF;
    data[i] = data[i] + (unsigned long)(ltmp << 16);
    ltmp = rdat[i * 4 + 3] & 0xFF;
    data[i] = data[i] + (unsigned long)(ltmp << 24);
  }
}

// read mids ; TCB
void TCBread_MIDS(int sid, unsigned long *data)
{
  unsigned long addr = 0x20000007;
  unsigned char rdat[160];
  unsigned long ltmp;
  int i;
  USB3TCBRead(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 40, addr, rdat);
  for (i = 0; i < 40; i ++) {
    data[i] = rdat[i * 4] & 0xFF;
    ltmp = rdat[i * 4 + 1] & 0xFF;
    data[i] = data[i] + (unsigned long)(ltmp << 8);
    ltmp = rdat[i * 4 + 2] & 0xFF;
    data[i] = data[i] + (unsigned long)(ltmp << 16);
    ltmp = rdat[i * 4 + 3] & 0xFF;
    data[i] = data[i] + (unsigned long)(ltmp << 24);
  }
}

// write pedestal trigger interval in ms; TCB
void TCBwrite_PTRIG(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x2000002F, data);
}
unsigned long TCBread_PTRIG(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x2000002F);
  return data;
}

// write trigger enable flag; TCB
void TCBwrite_TRIGENABLE(int sid, unsigned long mid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, 0x20000030, data);
}
unsigned long TCBread_TRIGENABLE(int sid, unsigned long mid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, 0x20000030);
  return data;
}

// write external trigger output select; TCB
void TCBwrite_EXTOUT(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000039, data);
}
unsigned long TCBread_EXTOUT(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000039);
  return data;
}

// write gate width; TCB;  0 ~ 4,294,967,288 ns
void TCBwrite_GATEWIDTH(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x2000003E, data);
}
unsigned long TCBread_GATEWIDTH(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x2000003E);
  return data;
}

// write external output width; TCB;  0 ~ 1,048,568 ns
void TCBwrite_EXTOUTWIDTH(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x2000003F, data);
}
unsigned long TCBread_EXTOUTWIDTH(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x2000003F);
  return data;
}

// read block counts of data, 1 buffer count = 1 kbyte ; TCB
unsigned long TCBread_BCOUNT(int sid, unsigned long mid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, 0x30000000);
  return data;
}

// read data, reads bcount * 1KB ; TCB
int TCBread_DATA(int sid, unsigned long mid, unsigned long bcount, unsigned char* data)
{
  unsigned long count = bcount * 256;
  return USB3TCBRead(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, count, 0x40000000, data);
}

// align ADC for NKFADC125
void TCB_ADCALIGN_125(int sid, unsigned long mid)
{
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

  TCBsend_ADCRST(sid, mid); 
  usleep(1000000);
  TCBsend_ADCCAL(sid, mid); 

  // ADC initialization codes
  TCBwrite_ADCALIGN(sid, mid, 0x00902);
  TCBwrite_ADCALIGN(sid, mid, 0x70A01);

  for (ch = 1; ch <= 16; ch++) {
    // set deskew pattern
    TCBwrite_ADCALIGN(sid, mid, 0x000602);
    TCBwrite_ADCALIGN(sid, mid, 0x000A33);
    TCBwrite_ADCALIGN(sid, mid, 0x000B33);

    // bitslip = 0;
    TCBwrite_BITSLIP(sid, mid, ch, 0);

    sum = 0;
    count = 0;
    flag = 0;
    gbitslip = 0;
  
    for(dly = 0; dly < 32; dly++) {
      // set ADC delay
      TCBwrite_ADCDLY(sid, mid, ch, dly); 

      // check word alignment
      value = TCBread_ADCSTAT(sid, mid); 
      bit_okay = (value >> (ch - 1)) & 0x1;
      
      if(bit_okay) {
        count = count + 1;
        sum = sum + dly;
        if(count > 5) 
          flag = flag + 1;
      }
      else{
        if (flag)
          dly = 32;
        else {
          sum = 0;
          count = 0;
        }
      }
    }
    
    // get good center
    if (count)
      gdly = sum / count;
    else
      gdly = 0;
    
    // set good delay
    TCBwrite_ADCDLY(sid, mid, ch, gdly); 

    // set sync pattern
    TCBwrite_ADCALIGN(sid, mid, 0x000E81);
    TCBwrite_ADCALIGN(sid, mid, 0x000F00);
    TCBwrite_ADCALIGN(sid, mid, 0x000A55);
    TCBwrite_ADCALIGN(sid, mid, 0x000B55);

    for(bitslip = 0; bitslip < 7; bitslip++) {
      // set bitslip
      TCBwrite_BITSLIP(sid, mid, ch, bitslip);
  
      // check word alignment
      value = TCBread_ADCSTAT(sid, mid); 
      word_okay = (value >> (ch + 15)) & 0x1;

      if(word_okay) {
        flag = flag + 1;
        gbitslip = bitslip;
        bitslip = 7;
      }
    }

    if (flag > 1) 
      printf("ADC(%ld) is aligned, delay = %ld, bitslip = %ld\n", ch, gdly, gbitslip);
    else 
      printf("Fail to align ADC(%ld)!\n", ch);
  }

  //set ADC normal operation
  TCBwrite_ADCALIGN(sid, mid, 0x12202);
  TCBwrite_ADCALIGN(sid, mid, 0x22202);
  TCBwrite_ADCALIGN(sid, mid, 0x42202);
  TCBwrite_ADCALIGN(sid, mid, 0x52202);

  TCBwrite_ADCALIGN(sid, mid, 0x000901);
  TCBwrite_ADCALIGN(sid, mid, 0x000600);
  TCBwrite_ADCALIGN(sid, mid, 0x000A00);
  TCBwrite_ADCALIGN(sid, mid, 0x000B00);
}

// align ADC for Muon DAQ
void TCB_ADCALIGN_MUONDAQ(int sid, unsigned long mid)
{
  unsigned long ch;
  unsigned long dly;
  unsigned long value;
  int flag;
  int count;
  int sum;
  unsigned long gdly;
  unsigned long bitslip;
  unsigned long gbitslip;

  TCBsend_ADCRST(sid, mid); 
  usleep(500000);
  TCBsend_ADCCAL(sid, mid); 

  // ADC initialization codes
  TCBwrite_ADCALIGN(sid, mid, 0x030002); 
  TCBwrite_ADCALIGN(sid, mid, 0x010010); 
  TCBwrite_ADCALIGN(sid, mid, 0xC78001); 
  TCBwrite_ADCALIGN(sid, mid, 0xDE01C0); 

  for (ch = 1; ch <= 5; ch++) {
    // set deskew pattern
    TCBwrite_ADCALIGN(sid, mid, 0x450001); 

    sum = 0;
    count = 0;
    flag = 0;
    gbitslip = 0;

    for (dly = 0; dly < 32; dly++) {
      TCBwrite_ADCDLY(sid, mid, ch, dly); 
      value = (TCBread_ADCSTAT(sid, mid) >> (ch -1)) & 0x1; 

      if (value) {
        count = count + 1;
        sum = sum + dly;
        if(count > 8) 
          flag = flag + 1;
      }
      else {
        if (flag)
          dly = 32;
        else {
          sum = 0;
          count = 0;
        }
      }
    }

    // get good center
    if (count)
      gdly = sum / count;
    else
      gdly = 0;
      
    // sets delay
    TCBwrite_ADCDLY(sid, mid, ch, gdly); 

    // set sync pattern
    TCBwrite_ADCALIGN(sid, mid, 0x450002); 

    for (bitslip = 0; bitslip < 12; bitslip++) {
      if (bitslip > 5)
        TCBwrite_ADCMUX(sid, mid, ch, 1);
      else
        TCBwrite_ADCMUX(sid, mid, ch, 0);
        
      value = (TCBread_ADCSTAT(sid, mid) >> ((ch -1) + 8)) & 0x1; 
      if (value) {
        flag = flag + 1;
        gbitslip = bitslip;
        bitslip = 12;
      }
      else 
        TCBwrite_BITSLIP(sid, mid, ch, 0); 
    }

    if (flag > 1)
      printf("ch%ld calibration delay = %ld, bitslip = %ld\n", ch, gdly, gbitslip);
    else
      printf("ch%ld fail to align ADC!\n", ch);
  }

  // set normal ADC mode
  TCBwrite_ADCALIGN(sid, mid, 0x450000); 
  usleep(100);
  TCBsend_ADCCAL(sid, mid); 
}

// write gain, value is 1/2/4/8/16
void TCBwrite_GAIN(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr = 0x20000013;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_GAIN(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr = 0x20000013;
  if(mid > 0)
    addr += ((ch - 1) & 0xFF) << 16;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write module's multiplicity threhold
void TCBwrite_MTHR(int sid, unsigned long mid, unsigned long data)
{
  unsigned long addr = 0x20000015;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_MTHR(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr = 0x20000015;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write PSD delay in ns
void TCBwrite_PSD_DLY(int sid, unsigned long mid, unsigned long data)
{
  unsigned long addr = 0x20000029;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_PSD_DLY(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr = 0x20000029;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write PSD threshold for neutron
void TCBwrite_PSD_THR(int sid, unsigned long mid, float data)
{
  unsigned long addr = 0x2000002A;
  unsigned long value;
  float fval;
  
  fval = data * 128.0;
  value = (unsigned long)(fval);
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, value);
}
float TCBread_PSD_THR(int sid, unsigned long mid)
{
  float fval;
  unsigned long data;
  unsigned long addr = 0x2000002A;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  fval = data;
  fval = fval / 128.0;
  return fval;
}

unsigned long TCBread_ADCSTAT_WORD(int sid, unsigned long mid)
{
  unsigned long data;
  unsigned long addr = 0x2000001B;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

//***************************************************************************


// write deadtime ; ch : 0 = FADC, 1 = SADC_MUON, 2 = SADC_LS, 3 = MUONDAQ
void TCBwrite_DT(int sid, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;
  if(mid > 0) {
    addr = 0x2000000F;
    addr += ((ch - 1) & 0xFF) << 16;
  }
  else 
    addr = 0x2000003A + ch;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr, data);
}
unsigned long TCBread_DT(int sid, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr;
  if(mid > 0) {
    addr = 0x2000000F;
    addr += ((ch - 1) & 0xFF) << 16;
  }
  else 
    addr = 0x2000003A + ch;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, mid, addr);
  return data;
}

// write multiplicity threshold for NKFADC500; TCB
void TCBwrite_MTHR_NKFADC500(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000031, data);
}
unsigned long TCBread_MTHR_NKFADC500(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000031);
  return data;
}

// write prescale for NKFADC500; TCB
void TCBwrite_PSCALE_NKFADC500(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000032, data);
}
unsigned long TCBread_PSCALE_NKFADC500(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000032);
  return data;
}

// write multiplicity threshold for M64ADC_MUON; TCB
void TCBwrite_MTHR_M64ADC_MUON(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000033, data);
}
unsigned long TCBread_MTHR_M64ADC_MUON(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000033);
  return data;
}

// write prescale for M64ADC_MUON; TCB
void TCBwrite_PSCALE_M64ADC_MUON(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000034, data);
}
unsigned long TCBread_PSCALE_M64ADC_MUON(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000034);
  return data;
}

// write multiplicity threshold for M64ADC_LS; TCB
void TCBwrite_MTHR_M64ADC_LS(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000035, data);
}
unsigned long TCBread_MTHR_M64ADC_LS(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000035);
  return data;
}

// write prescale for M64ADC_LS; TCB
void TCBwrite_PSCALE_M64ADC_LS(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000036, data);
}
unsigned long TCBread_PSCALE_M64ADC_LS(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000036);
  return data;
}

// write multiplicity threshold for MUONDAQ; TCB
void TCBwrite_MTHR_MUONDAQ(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000037, data);
}
unsigned long TCBread_MTHR_MUONDAQ(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000037);
  return data;
}

// write prescale for MUONDAQ; TCB
void TCBwrite_PSCALE_MUONDAQ(int sid, unsigned long data)
{
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000038, data);
}
unsigned long TCBread_PSCALE_MUONDAQ(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000038);
  return data;
}

// write NKFADC500 trigger switch
void TCBwrite_TRIG_SWITCH_NKFADC500(int sid, unsigned long fadc, unsigned long sadc_muon, unsigned long sadc_ls, unsigned long MUONDAQ)
{
  unsigned long data;

  data = 0;
  if (fadc)
    data = data + 1;
  if (sadc_muon)
    data = data + 2;
  if (sadc_ls)
    data = data + 4;
  if (MUONDAQ)
    data = data + 8;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000043, data);
}
unsigned long TCBread_SWITCH_NKFADC500(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000043);
  return data;
}

// write M64ADC_MUON trigger switch
void TCBwrite_TRIG_SWITCH_M64ADC_MUON(int sid, unsigned long fadc, unsigned long sadc_muon, unsigned long sadc_ls, unsigned long MUONDAQ)
{
  unsigned long data;

  data = 0;
  if (fadc)
    data = data + 1;
  if (sadc_muon)
    data = data + 2;
  if (sadc_ls)
    data = data + 4;
  if (MUONDAQ)
    data = data + 8;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000044, data);
}
unsigned long TCBread_SWITCH_M64ADC_MUON(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000044);
  return data;
}

// write M64ADC_LS trigger switch
void TCBwrite_TRIG_SWITCH_M64ADC_LS(int sid, unsigned long fadc, unsigned long sadc_muon, unsigned long sadc_ls, unsigned long MUONDAQ)
{
  unsigned long data;

  data = 0;
  if (fadc)
    data = data + 1;
  if (sadc_muon)
    data = data + 2;
  if (sadc_ls)
    data = data + 4;
  if (MUONDAQ)
    data = data + 8;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000045, data);
}
unsigned long TCBread_SWITCH_M64ADC_LS(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000045);
  return data;
}

// write MUON DAQ trigger switch
void TCBwrite_TRIG_SWITCH_MUONDAQ(int sid, unsigned long fadc, unsigned long sadc_muon, unsigned long sadc_ls, unsigned long MUONDAQ)
{
  unsigned long data;

  data = 0;
  if (fadc)
    data = data + 1;
  if (sadc_muon)
    data = data + 2;
  if (sadc_ls)
    data = data + 4;
  if (MUONDAQ)
    data = data + 8;
  USB3TCBWrite(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000046, data);
}
unsigned long TCBread_SWITCH_MUONDAQ(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000046);
  return data;
}

unsigned long TCBread_DBG(int sid)
{
  unsigned long data;
  data = USB3TCBReadReg(TCB_VENDOR_ID, TCB_PRODUCT_ID, sid, 0, 0x20000050);
  return data;
}




