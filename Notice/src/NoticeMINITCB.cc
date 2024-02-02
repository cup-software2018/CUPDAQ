#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "Notice/NoticeMINITCB.hh"

// open MINITCB
int MINITCBopen(char *ip)
{
  struct sockaddr_in serv_addr;
  int tcp_Handle;
  const int disable = 1;
        
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_port        = htons(5000);
        
  if ( (tcp_Handle = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Can't open MINITCB\n");
    return -1;
  }

  setsockopt(tcp_Handle, IPPROTO_TCP,TCP_NODELAY,(char *) &disable, sizeof(disable)); 

  if (connect(tcp_Handle, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    printf("client: can't connect to server\n");
    printf("ip %s , port 5000 \n", ip);
    printf("error number is %d \n", connect(tcp_Handle, (struct sockaddr *) &serv_addr,sizeof(serv_addr)));

    return -2;
  } 
  
  return tcp_Handle;
}

// close MINITCB
void MINITCBclose(int tcp_Handle)
{
  close(tcp_Handle);
}

// transmit characters to MINITCB
int MINITCBtransmit(int tcp_Handle, char *buf, int len)
{
  int result;
  int bytes_more;
  int  bytes_xferd;
  char *idxPtr;

  bytes_more = len;
  idxPtr = buf;
  bytes_xferd = 0;
  while (1) {
    idxPtr = buf + bytes_xferd;
    result=write (tcp_Handle, (char *) idxPtr, bytes_more);

    if (result<0) {
      printf("Could not write the rest of the block successfully, returned: %d\n",bytes_more);
      return -1;
    }
    
    bytes_xferd += result;
    bytes_more -= result;
    
    if (bytes_more <= 0)
      break;
  }

  return 0;
}

// receive characters from MINITCB
int MINITCBreceive(int tcp_Handle, char *buf, int len)
{
  int result;
  int accum;
  int space_left;
  int bytes_more;
  int buf_count;
  char *idxPtr;

  fd_set rfds;
  //struct timeval tval;

  //tval.tv_sec = MAX_TCP_READ;
  //tval.tv_usec = 0;

  FD_ZERO(&rfds);
  FD_SET(tcp_Handle, &rfds);

  if (buf==NULL)
    return -1;

  idxPtr = buf;

  buf_count = 0;
  space_left = len;
  while (1) {
    accum = 0;
    while (1) {
      idxPtr = buf + (buf_count + accum);
      bytes_more = space_left;
      
      if ((result = read(tcp_Handle, (char *) idxPtr, (bytes_more>2048)?2048:bytes_more)) < 0) {
        printf("Unable to receive data from the server.\n");
        return -1;
      }
      
      accum += result;
      if ((accum + buf_count) >= len)
	break;

      if(result<bytes_more) {
        printf("wanted %d got %d \n",bytes_more,result);
        return accum+buf_count;
      }
    }
    
    buf_count += accum;
    space_left -= accum;

    if (space_left <= 0)
      break;
  }

  return buf_count;
}

// write to MINITCB
void MINITCBwrite(int tcp_Handle, unsigned long address, unsigned long data)
{
  char tcpBuf[3];

  tcpBuf[0] = 11;
  tcpBuf[1] = address & 0xFF;
  tcpBuf[2] = data & 0xFF;

  MINITCBtransmit(tcp_Handle, tcpBuf, 3);
  
  MINITCBreceive(tcp_Handle, tcpBuf, 1);
}

// read from MINITCB
unsigned long MINITCBread(int tcp_Handle, unsigned long address)
{
  char tcpBuf[2];
  unsigned long data;

  tcpBuf[0] = 12;
  tcpBuf[1] = address & 0xFF;

  MINITCBtransmit(tcp_Handle, tcpBuf, 2);
  
  MINITCBreceive(tcp_Handle, tcpBuf, 1);

  data = tcpBuf[0] & 0xFF;

  return data;
}

// write to module
void MINITCBwrite_Module(int tcp_Handle, unsigned long mid, unsigned long address, unsigned long data)
{
  // set mid
  MINITCBwrite(tcp_Handle, 0x17, mid);

  // set address
  MINITCBwrite(tcp_Handle, 0x18, address & 0xFF);
  MINITCBwrite(tcp_Handle, 0x19, (address >> 8) & 0xFF);
  MINITCBwrite(tcp_Handle, 0x1A, (address >> 16) & 0xFF);
  MINITCBwrite(tcp_Handle, 0x1B, (address >> 24) & 0xFF);

  // set data
  MINITCBwrite(tcp_Handle, 0x1C, data & 0xFF);
  MINITCBwrite(tcp_Handle, 0x1D, (data >> 8) & 0xFF);
  MINITCBwrite(tcp_Handle, 0x1E, (data >> 16) & 0xFF);
  MINITCBwrite(tcp_Handle, 0x1F, (data >> 24) & 0xFF);

  // send command
  MINITCBwrite(tcp_Handle, 0x15, 0);
}

// read from module
unsigned long MINITCBread_Module(int tcp_Handle, unsigned long mid, unsigned long address)
{
  unsigned long data;
  unsigned long value;

  // set mid
  MINITCBwrite(tcp_Handle, 0x17, mid);

  // set address
  MINITCBwrite(tcp_Handle, 0x18, address & 0xFF);
  MINITCBwrite(tcp_Handle, 0x19, (address >> 8) & 0xFF);
  MINITCBwrite(tcp_Handle, 0x1A, (address >> 16) & 0xFF);
  MINITCBwrite(tcp_Handle, 0x1B, (address >> 24) & 0xFF);

  // send command
  MINITCBwrite(tcp_Handle, 0x16, 0);

  // get data
  data = MINITCBread(tcp_Handle, 0x1C);
  value = MINITCBread(tcp_Handle, 0x1D);
  data = data + (value << 8);
  value = MINITCBread(tcp_Handle, 0x1E);
  data = data + (value << 16);
  value = MINITCBread(tcp_Handle, 0x1F);
  data = data + (value << 24);

  return data;
}

// reset data acquisition
void MINITCBreset(int tcp_Handle)
{
  MINITCBwrite(tcp_Handle, 0x00, 4);
}

// reset timer
void MINITCBresetTIMER(int tcp_Handle)
{
  MINITCBwrite(tcp_Handle, 0x00, 1);
}

// start data acquisition
void MINITCBstart(int tcp_Handle)
{
  MINITCBwrite(tcp_Handle, 0x00, 8);
}

// stop data acquisition
void MINITCBstop(int tcp_Handle)
{
  MINITCBwrite(tcp_Handle, 0x00, 0);
}

// read RUN status
unsigned long MINITCBread_RUN(int tcp_Handle, unsigned long mid)
{
  unsigned long data;

  if (mid)
    data = MINITCBread_Module(tcp_Handle, mid, 0x20000000);
  else
    data = MINITCBread(tcp_Handle, 0x00);

  return data;
}

// write coincidence window
void MINITCBwrite_CW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;;

  if(mid > 0) {
    addr = 0x20000001 + (((ch - 1) & 0xFF) << 16);
    MINITCBwrite_Module(tcp_Handle, mid, addr, data);
  }
  else {
    MINITCBwrite(tcp_Handle, 0x02, data & 0xFF);
    MINITCBwrite(tcp_Handle, 0x03, (data >> 8) & 0xFF);
  }
}

// read coincidence window
unsigned long MINITCBread_CW(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr;
  unsigned long temp;

  if(mid > 0) {
    addr = 0x20000001 + (((ch - 1) & 0xFF) << 16);
    data = MINITCBread_Module(tcp_Handle, mid, addr);
  }
  else {
    data = MINITCBread(tcp_Handle, 0x02);
    temp = MINITCBread(tcp_Handle, 0x03);
    data = data + (temp << 8);
  }

  return data;
}

void MINITCBwrite_GW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;;

  if(mid > 0) {
    addr = 0x20000002 + (((ch - 1) & 0xFF) << 16);
    MINITCBwrite_Module(tcp_Handle, mid, addr, data);
  }
  else {
    MINITCBwrite(tcp_Handle, 0x02, data & 0xFF);
    MINITCBwrite(tcp_Handle, 0x03, (data >> 8) & 0xFF);
  }
}

unsigned long MINITCBread_GW(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr;
  unsigned long temp;

  if(mid > 0) {
    addr = 0x20000002 + (((ch - 1) & 0xFF) << 16);
    data = MINITCBread_Module(tcp_Handle, mid, addr);
  }
  else {
    data = MINITCBread(tcp_Handle, 0x02);
    temp = MINITCBread(tcp_Handle, 0x03);
    data = data + (temp << 8);
  }

  return data;
}


// write segment setting 
void MINITCBwrite_RL(int tcp_Handle, unsigned long mid, unsigned long data)
{
  MINITCBwrite_Module(tcp_Handle, mid, 0x20000002, data);
}

// read segment setting 
unsigned long MINITCBread_RL(int tcp_Handle, unsigned long mid)
{
  return MINITCBread_Module(tcp_Handle, mid, 0x20000002);
}

// turn on/off DRAM
// 0 = off, 1 = on
void MINITCBwrite_DRAMON(int tcp_Handle, unsigned long mid, unsigned long data)
{
  unsigned long status;

  // turn on DRAM
  if (data) {
    // check DRAM is on
    status = MINITCBread_Module(tcp_Handle, mid, 0x20000003);

    // when DRAM is on now, turn it off
    if (status)
      MINITCBwrite_Module(tcp_Handle, mid, 0x20000003, 0);

    // turn on DRAM
    MINITCBwrite_Module(tcp_Handle, mid, 0x20000003, 1);

    // wait for DRAM ready
    status = 0;
    while (!status)
      status = MINITCBread_Module(tcp_Handle, mid, 0x20000003);
  }
  // trun off DRAM
  else
    MINITCBwrite_Module(tcp_Handle, mid, 0x20000003, 0);
}

// read DRAM status
unsigned long MINITCBread_DRAMON(int tcp_Handle, unsigned long mid)
{
  return MINITCBread_Module(tcp_Handle, mid, 0x20000003);
}

// write offset adjustment
void MINITCBwrite_DACOFF(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000004 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
  //sleep(1);
}

// read offset adjustment
unsigned long MINITCBread_DACOFF(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000004 + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// measure pedestal
void MINITCBmeasure_PED(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000005 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, 0);
}

// read pedestal
unsigned long MINITCBread_PED(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000006 + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write input delay
void MINITCBwrite_DLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;
  unsigned long value;

  addr = 0x20000007 + (((ch - 1) & 0xFF) << 16);

  value = ((data / 1000) << 10) | (data % 1000);

  MINITCBwrite_Module(tcp_Handle, mid, addr, value);
}

// read input delay
unsigned long MINITCBread_DLY(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr;
  unsigned long value;

  addr = 0x20000007 + (((ch - 1) & 0xFF) << 16);

  value = MINITCBread_Module(tcp_Handle, mid, addr);

  data = (value >> 10)*1000 + (value&0X3FF);
  
  return data;
}

// write input delay
void MINITCBwrite_AMOREDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000007 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read input delay
unsigned long MINITCBread_AMOREDLY(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr;

  addr = 0x20000007 + (((ch - 1) & 0xFF) << 16);

  data = MINITCBread_Module(tcp_Handle, mid, addr);
  
  return data;
}

// write discriminator threshold
void MINITCBwrite_THR(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000008 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read discriminator threshold
unsigned long MINITCBread_THR(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000008 + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write input pulse polarity
void MINITCBwrite_POL(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000009 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read input pulse polarity
unsigned long MINITCBread_POL(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000009 + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write pulse sum trigger width
void MINITCBwrite_PSW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000A + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read pulse sum trigger width
unsigned long MINITCBread_PSW(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000A + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write ADC mode
void MINITCBwrite_AMODE(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000B + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read ADC mode
unsigned long MINITCBread_AMODE(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000B + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write pulse count threshold ; TFADC500
void MINITCBwrite_PCT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000C + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read pulse count threshold ; TFADC500
unsigned long MINITCBread_PCT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000C + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write pulse count interval
void MINITCBwrite_PCI(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000D + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read pulse count interval
unsigned long MINITCBread_PCI(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000D + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write pulse width threshold
void MINITCBwrite_PWT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000E + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read pulse width threshold
unsigned long MINITCBread_PWT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000000E + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write deadtime
void MINITCBwrite_DT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  if(mid > 0) {
    addr = 0x2000000F + (((ch - 1) & 0xFF) << 16);
    MINITCBwrite_Module(tcp_Handle, mid, addr, data);
  }
  else {
    MINITCBwrite(tcp_Handle, 0x01, data & 0xFF);
    MINITCBwrite(tcp_Handle, 0x0B, (data >> 8) & 0xFF);
    MINITCBwrite(tcp_Handle, 0x0D, (data >> 16) & 0xFF);
  }
}

// read deadtime
unsigned long MINITCBread_DT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long data;
  unsigned long addr;
  unsigned long temp;

  if(mid > 0) {
    addr = 0x2000000F + (((ch - 1) & 0xFF) << 16);
    data = MINITCBread_Module(tcp_Handle, mid, addr);
  }
  else {
    data = MINITCBread(tcp_Handle, 0x01);
    temp = MINITCBread(tcp_Handle, 0x0B);
    data = data + (temp << 8);
    temp = MINITCBread(tcp_Handle, 0x0D);
    data = data + (temp << 16);
  }

  return data;
}

// write trigger mode
void MINITCBwrite_TM(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000014 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read trigger mode
unsigned long MINITCBread_TM(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000014 + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write trigger lookup table
void MINITCBwrite_TLT(int tcp_Handle, unsigned long mid, unsigned long data)
{
  MINITCBwrite_Module(tcp_Handle, mid, 0x20000015, data);
}

// read trigger lookup table
unsigned long MINITCBread_TLT(int tcp_Handle, unsigned long mid)
{
  return MINITCBread_Module(tcp_Handle, mid, 0x20000015);
}

// write trigger lookup table for M64ADC only
void MINITCBwrite_STLT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000015 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read trigger lookup table for M64ADC only
unsigned long MINITCBread_STLT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000015 + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write zero suppression
void MINITCBwrite_ZEROSUP(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000016 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read zero suppression
unsigned long MINITCBread_ZEROSUP(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000016 + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// send ADC reset signal
void MINITCBsend_ADCRST(int tcp_Handle, unsigned long mid)
{
  MINITCBwrite_Module(tcp_Handle, mid, 0x20000017, 0);
}

// send ADC calibration signal
void MINITCBsend_ADCCAL(int tcp_Handle, unsigned long mid)
{
  MINITCBwrite_Module(tcp_Handle, mid, 0x20000018, 0);
}

// write ADC calibration delay
void MINITCBwrite_ADCDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000019 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// write ADC align delay
void MINITCBwrite_ADCALIGN(int tcp_Handle, unsigned long mid, unsigned long data)
{
  unsigned long addr = 0x2000001A;
  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read ADC status; TFADC500
unsigned long MINITCBread_ADCSTAT(int tcp_Handle, unsigned long mid)
{
  return MINITCBread_Module(tcp_Handle, mid, 0x2000001A);
}

// write BitSlip : TFADC64
void MINITCBwrite_BITSLIP(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000001B + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// write flash ADC buffer mux ; TFADC64
void MINITCBwrite_FMUX(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x2000001C;

  MINITCBwrite_Module(tcp_Handle, mid, addr, ch -1);
}

// read flash ADC buffer mux ; TFADC64
unsigned long MINITCBread_FMUX(int tcp_Handle, unsigned long mid)
{
  return MINITCBread_Module(tcp_Handle, mid, 0x2000001C);
}

// arm flash ADC buffer ; TFADC64
void MINITCBarm_FADC(int tcp_Handle, unsigned long mid)
{
  MINITCBwrite_Module(tcp_Handle, mid, 0x2000001D, 0);
}

// read flash ADC buffer ready ; TFADC64
unsigned long MINITCBread_FREADY(int tcp_Handle, unsigned long mid)
{
  return MINITCBread_Module(tcp_Handle, mid, 0x2000001D);
}

// write zerosuppression flag duration (in ns 8 ~ 65532)
void MINITCBwrite_ZSFD(int tcp_Handle, unsigned long mid, unsigned long data)
{
  MINITCBwrite_Module(tcp_Handle, mid, 0x2000001E, data);
}

// read zerosuppression flag duration 
unsigned long MINITCBread_ZSFD(int tcp_Handle, unsigned long mid)
{
  return MINITCBread_Module(tcp_Handle, mid, 0x2000001E);
}

// write ADC down-sampling rate
void MINITCBwrite_DSR(int tcp_Handle, unsigned long mid, unsigned long data)
{
  MINITCBwrite_Module(tcp_Handle, mid, 0x2000001F, data);
}

// read ADC down-sampling rate
unsigned long MINITCBread_DSR(int tcp_Handle, unsigned long mid)
{
  return MINITCBread_Module(tcp_Handle, mid, 0x2000001F);
}

// read flash ADC buffer : TFADC64
void MINITCBread_FADCBUF(int tcp_Handle, unsigned long mid, unsigned long *data)
{
  unsigned long addr = 0x20008000;
  unsigned long i;

  for (i = 0; i < 2048; i++) 
    data[i] = MINITCBread_Module(tcp_Handle, mid, addr + i);
}

// align ADC for NKFADC500
void MINITCB_ADCALIGN(int tcp_Handle, unsigned long mid)
{
  unsigned long ch, dly, value;
  int count, sum, center;
  unsigned long gdly;
  int flag;

  MINITCBsend_ADCRST(tcp_Handle, mid); 
  usleep(500000);
  MINITCBsend_ADCCAL(tcp_Handle, mid); 
  MINITCBwrite_ADCALIGN(tcp_Handle, mid, 1);

  for (ch = 1; ch <= 4; ch++) {
    count = 0;
    sum = 0;
    flag = 0;

    for (dly = 0; dly < 32; dly++) {
      MINITCBwrite_ADCDLY(tcp_Handle, mid, ch, dly); 
      value = (MINITCBread_ADCSTAT(tcp_Handle, mid) >> (ch -1)) & 0x1; 
      
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
    MINITCBwrite_ADCDLY(tcp_Handle, mid, ch, gdly); 
    printf("ch%ld calibration delay = %ld\n", ch, gdly);
  }

  MINITCBwrite_ADCALIGN(tcp_Handle, mid, 0); 
  MINITCBsend_ADCCAL(tcp_Handle, mid); 
}

// align ADC for M64ADC
void MINITCB_ADCALIGN_64(int tcp_Handle, unsigned long mid)
{
  unsigned long ch, dly, value;
  int count, sum, center;
  unsigned long bitslip;
  unsigned long gdly;
  unsigned long gbitslip;
  int flag;

  MINITCBsend_ADCRST(tcp_Handle, mid); 
  usleep(500000);
  MINITCBsend_ADCCAL(tcp_Handle, mid); 

  for (ch = 1; ch <= 4; ch++) {
    count = 0;
    sum = 0;
    flag = 0;
    gbitslip = 0;

    // ADC initialization codes
    MINITCBwrite_ADCALIGN(tcp_Handle, mid, 0x030002); 
    usleep(100);
    MINITCBwrite_ADCALIGN(tcp_Handle, mid, 0x010010); 
    usleep(100);
    MINITCBwrite_ADCALIGN(tcp_Handle, mid, 0xC78001); 
    usleep(100);
    MINITCBwrite_ADCALIGN(tcp_Handle, mid, 0xDE01C0); 
    usleep(100);

    // set deskew pattern
    MINITCBwrite_ADCALIGN(tcp_Handle, mid, 0x450001); 

    // set bitslip = 0
    MINITCBwrite_BITSLIP(tcp_Handle, mid, ch, 0); 

    for (dly = 0; dly < 32; dly++) {
      MINITCBwrite_ADCDLY(tcp_Handle, mid, ch, dly); 
      value = (MINITCBread_ADCSTAT(tcp_Handle, mid) >> (ch -1)) & 0x1; 
      
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
    MINITCBwrite_ADCDLY(tcp_Handle, mid, ch, gdly); 

    // set sync pattern
    MINITCBwrite_ADCALIGN(tcp_Handle, mid, 0x450002); 
    usleep(100);

    for (bitslip = 0; bitslip < 12; bitslip++) {
      MINITCBwrite_BITSLIP(tcp_Handle, mid, ch, bitslip); 

      value = (MINITCBread_ADCSTAT(tcp_Handle, mid) >> ((ch -1) + 4)) & 0x1; 
      if (value) {
        gbitslip = bitslip;
        bitslip = 12;
      }
    }

    // set good bitslip
    MINITCBwrite_BITSLIP(tcp_Handle, mid, ch, gbitslip); 

    printf("ch%ld calibration delay = %ld, bitslip = %ld\n", ch, gdly, gbitslip);
  }

  // set normal ADC mode
  MINITCBwrite_ADCALIGN(tcp_Handle, mid, 0x450000); 
  usleep(100);
  MINITCBsend_ADCCAL(tcp_Handle, mid); 
}

// write peak sum scale - peak sum out = peak area/peak sum scale ; TFADC64
void MINITCBwrite_PSS(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000010 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read peak sum sacle ; TFADC64
unsigned long MINITCBread_PSS(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000010 + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write rise time, rise time = set value * 2 * sampling period ; AMOREADC
void MINITCBwrite_RT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000011 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read rise time ; AMOREADC
unsigned long MINITCBread_RT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000011 + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write sampling rate, sampling rate = 1 MHz / setting value; AMOREADC
void MINITCBwrite_SR(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000012 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read sampling rate ; AMOREADC
unsigned long MINITCBread_SR(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000012 + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write gain, value is 1/2/4, gain = 2^setting; AMOREADC
void MINITCBwrite_DACGAIN(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000013 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read gain ; AMOREADC
unsigned long MINITCBread_DACGAIN(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000013 + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write signal window length in # of ADC samples ; AMOREADC
void MINITCBwrite_ST(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000020 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read signal window length in # of ADC samples ; AMOREADC
unsigned long MINITCBread_ST(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000020 + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// write pedestal window length in # of ADC samples ; AMOREADC
void MINITCBwrite_PT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000021 + (((ch - 1) & 0xFF) << 16);

  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// read pedestal window length in # of ADC samples ; AMOREADC
unsigned long MINITCBread_PT(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000021 + (((ch - 1) & 0xFF) << 16);

  return MINITCBread_Module(tcp_Handle, mid, addr);
}



// write run number
void MINITCBwrite_RUNNO(int tcp_Handle, unsigned long data)
{
  MINITCBwrite(tcp_Handle, 0x04, data & 0xFF);
  MINITCBwrite(tcp_Handle, 0x05, (data >> 8) & 0xFF);
}

// read run number
unsigned long MINITCBread_RUNNO(int tcp_Handle)
{
  unsigned long data;
  unsigned long temp;

  data = MINITCBread(tcp_Handle, 0x04);
  temp = MINITCBread(tcp_Handle, 0x05);
  data = data + (temp << 8);

  return data;
}

// send trigger
void MINITCBsend_TRIG(int tcp_Handle)
{
  MINITCBwrite(tcp_Handle, 0x06, 0);
}

// read link status
unsigned long MINITCBread_LNSTAT(int tcp_Handle)
{
  return MINITCBread(tcp_Handle, 0x10);
}

// read mids
unsigned long MINITCBread_MIDS(int tcp_Handle, unsigned long ch)
{
  unsigned long addr;

  addr = 0x10 + ch;

  return  MINITCBread(tcp_Handle, addr);
}

// write pedestal trigger interval in ms
void MINITCBwrite_PTRIG(int tcp_Handle, unsigned long data)
{
  MINITCBwrite(tcp_Handle, 0x08, data & 0xFF);
  MINITCBwrite(tcp_Handle, 0x09, (data >> 8) & 0xFF);
}

// read pedestal trigger interval in ms
unsigned long MINITCBread_PTRIG(int tcp_Handle)
{
  unsigned long data;
  unsigned long temp;

  data = MINITCBread(tcp_Handle, 0x08);
  temp = MINITCBread(tcp_Handle, 0x09);
  data = data + (temp << 8);

  return data;
}

// write trigger enable, D0 = self, D1 = pedestal, D2 = software, D3 = external
void MINITCBwrite_TRIGENABLE(int tcp_Handle, unsigned long mid, unsigned long data)
{
  if (mid)
    MINITCBwrite_Module(tcp_Handle, mid, 0x20000030, data);
  else
    MINITCBwrite(tcp_Handle, 0x0A, data & 0xFF);
}

// read trigger enable
unsigned long MINITCBread_TRIGENABLE(int tcp_Handle, unsigned long mid)
{
  unsigned long data;

  if (mid)
    data = MINITCBread_Module(tcp_Handle, mid, 0x20000030);
  else data = MINITCBread(tcp_Handle, 0x0A);

  return data;
}

// write multiplicity trigger threshold
void MINITCBwrite_MTHR(int tcp_Handle, unsigned long data)
{
  MINITCBwrite(tcp_Handle, 0x0C, data & 0xFF);
}

// read multiplicity trigger threshold
unsigned long MINITCBread_MTHR(int tcp_Handle)
{
  return MINITCBread(tcp_Handle, 0x0C);
}

// write trigger prescale
void MINITCBwrite_PSCALE(int tcp_Handle, unsigned long data)
{
  MINITCBwrite(tcp_Handle, 0x0E, data & 0xFF);
  MINITCBwrite(tcp_Handle, 0x0F, (data >> 8) & 0xFF);
}

// read trigger prescale
unsigned long MINITCBread_PSCALE(int tcp_Handle)
{
  unsigned long data;
  unsigned long temp;

  data = MINITCBread(tcp_Handle, 0x0E);
  temp = MINITCBread(tcp_Handle, 0x0F);
  data = data + (temp << 8);

  return data;
}

// write DRAM input delay
void MINITCBwrite_DRAMDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000022 + ((ch & 0xFF) << 16);
  MINITCBwrite_Module(tcp_Handle, mid, addr, data);
}

// write DRAM input bitslip
void MINITCBwrite_DRAMBITSLIP(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000023 + ((ch & 0xFF) << 16);
  MINITCBwrite_Module(tcp_Handle, mid, addr, 0);
}

// write DRAM test mode
void MINITCBwrite_DRAMTEST(int tcp_Handle, unsigned long mid, unsigned long data)
{
  MINITCBwrite_Module(tcp_Handle, mid, 0x20000024, data);
}

// read DRAM test mode
unsigned long MINITCBread_DRAMTEST(int tcp_Handle, unsigned long mid, unsigned long ch)
{
  unsigned long addr;

  addr = 0x20000024 + ((ch & 0xFF) << 16);
  return MINITCBread_Module(tcp_Handle, mid, addr);
}

// Align DRAM input
void MINITCB_ADCALIGN_DRAM(int tcp_Handle, unsigned long mid)
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
  MINITCBwrite_DRAMON(tcp_Handle, mid, 1);

  // enter DRAM test mode
  MINITCBwrite_DRAMTEST(tcp_Handle, mid, 1);

  // send reset to iodelay  
  MINITCBsend_ADCCAL(tcp_Handle, mid);

  // fill DRAM test pattern
  MINITCBwrite_DRAMTEST(tcp_Handle, mid, 2);

  for (ch = 0; ch < 8; ch++) {
    count = 0;
    sum = 0;
    flag = 0;

    // search delay
    for (dly = 0; dly < 32; dly++) {
      // set delay
      MINITCBwrite_DRAMDLY(tcp_Handle, mid, ch, dly);

      // read DRAM test pattern
      MINITCBwrite_DRAMTEST(tcp_Handle, mid, 3);
      value = MINITCBread_DRAMTEST(tcp_Handle, mid, ch);

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
    MINITCBwrite_DRAMDLY(tcp_Handle, mid, ch, gdly); 
  
    // get bitslip
    for (bitslip = 0; bitslip < 4; bitslip++) {
      // read DRAM test pattern
      MINITCBwrite_DRAMTEST(tcp_Handle, mid, 3);
      value = MINITCBread_DRAMTEST(tcp_Handle, mid, ch);

      if (value == 0xFFAA5500) {
        aflag = 1;
        bitslip = 4;
      }
      else {
        aflag = 0;
        MINITCBwrite_DRAMBITSLIP(tcp_Handle, mid, ch);
      }
    }

    if (aflag)
      printf("DRAM(%ld) is aligned, delay = %ld\n", ch, gdly);
    else
      printf("Fail to align DRAM(%ld)!\n", ch);
  }

  // exit DRAM test mode
  MINITCBwrite_DRAMTEST(tcp_Handle, mid, 0);
}

