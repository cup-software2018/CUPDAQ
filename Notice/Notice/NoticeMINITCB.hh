#ifndef MINITCB_H
#define MINITCB_H

#define MAX_TCP_CONNECT         5       /* time in secs to get a connection */
#define MAX_TCP_READ            3       /* time in secs to wait for the DSO
                                           to respond to a read request */
#define BOOL                    int
#define TRUE                    1
#define FALSE                   0

#ifdef __cplusplus
extern  "C" {
#endif

extern int MINITCBopen(char *ip);
extern void MINITCBclose(int tcp_Handle);
extern int MINITCBtransmit(int tcp_Handle, char *buf, int len);
extern int MINITCBreceive(int tcp_Handle, char *buf, int len);
extern void MINITCBwrite(int tcp_Handle, unsigned long address, unsigned long data);
extern unsigned long MINITCBread(int tcp_Handle, unsigned long address);
extern void MINITCBwrite_Module(int tcp_Handle, unsigned long mid, unsigned long address, unsigned long data);
extern unsigned long MINITCBread_Module(int tcp_Handle, unsigned long mid, unsigned long address);
extern void MINITCBreset(int tcp_Handle);
extern void MINITCBresetTIMER(int tcp_Handle);
extern void MINITCBstart(int tcp_Handle);
extern void MINITCBstop(int tcp_Handle);
extern unsigned long MINITCBread_RUN(int tcp_Handle, unsigned long mid);
extern void MINITCBwrite_CW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_CW(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_GW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_GW(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_RL(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCBread_RL(int tcp_Handle, unsigned long mid);
extern void MINITCBwrite_DRAMON(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCBread_DRAMON(int tcp_Handle, unsigned long mid);
extern void MINITCBwrite_DACOFF(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_DACOFF(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBmeasure_PED(int tcp_Handle, unsigned long mid, unsigned long ch);
extern unsigned long MINITCBread_PED(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_DLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_DLY(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_AMOREDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_AMOREDLY(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_THR(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_THR(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_POL(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_POL(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_PSW(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_PSW(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_AMODE(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_AMODE(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_PCT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_PCT(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_PCI(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_PCI(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_PWT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_PWT(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_DT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_DT(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_TM(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_TM(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_TLT(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCBread_TLT(int tcp_Handle, unsigned long mid);
extern void MINITCBwrite_STLT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_STLT(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_ZEROSUP(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_ZEROSUP(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBsend_ADCRST(int tcp_Handle, unsigned long mid);
extern void MINITCBsend_ADCCAL(int tcp_Handle, unsigned long mid);
extern void MINITCBwrite_ADCDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern void MINITCBwrite_ADCALIGN(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCBread_ADCSTAT(int tcp_Handle, unsigned long mid);
extern void MINITCBwrite_BITSLIP(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern void MINITCBwrite_FMUX(int tcp_Handle, unsigned long mid, unsigned long ch);
extern unsigned long MINITCBread_FMUX(int tcp_Handle, unsigned long mid);
extern void MINITCBarm_FADC(int tcp_Handle, unsigned long mid);
extern unsigned long MINITCBread_FREADY(int tcp_Handle, unsigned long mid);
extern void MINITCBwrite_ZSFD(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCBread_ZSFD(int tcp_Handle, unsigned long mid);
extern void MINITCBwrite_DSR(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCBread_DSR(int tcp_Handle, unsigned long mid);
extern void MINITCBread_FADCBUF(int tcp_Handle, unsigned long mid, unsigned long *data);
extern void MINITCB_ADCALIGN(int tcp_Handle, unsigned long mid);
extern void MINITCB_ADCALIGN_64(int tcp_Handle, unsigned long mid);
extern void MINITCBwrite_PSS(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_PSS(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_RT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_RT(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_SR(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_SR(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_DACGAIN(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_DACGAIN(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_ST(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_ST(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_PT(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern unsigned long MINITCBread_PT(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_RUNNO(int tcp_Handle, unsigned long data);
extern unsigned long MINITCBread_RUNNO(int tcp_Handle);
extern void MINITCBsend_TRIG(int tcp_Handle);
extern unsigned long MINITCBread_LNSTAT(int tcp_Handle);
extern unsigned long MINITCBread_MIDS(int tcp_Handle, unsigned long ch);
extern void MINITCBwrite_PTRIG(int tcp_Handle, unsigned long data);
extern unsigned long MINITCBread_PTRIG(int tcp_Handle);
extern void MINITCBwrite_TRIGENABLE(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCBread_TRIGENABLE(int tcp_Handle, unsigned long mid);
extern void MINITCBwrite_MTHR(int tcp_Handle, unsigned long data);
extern unsigned long MINITCBread_MTHR(int tcp_Handle);
extern void MINITCBwrite_PSCALE(int tcp_Handle, unsigned long data);
extern unsigned long MINITCBread_PSCALE(int tcp_Handle);
extern void MINITCBwrite_DRAMDLY(int tcp_Handle, unsigned long mid, unsigned long ch, unsigned long data);
extern void MINITCBwrite_DRAMBITSLIP(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCBwrite_DRAMTEST(int tcp_Handle, unsigned long mid, unsigned long data);
extern unsigned long MINITCBread_DRAMTEST(int tcp_Handle, unsigned long mid, unsigned long ch);
extern void MINITCB_ADCALIGN_DRAM(int tcp_Handle, unsigned long mid);
#ifdef __cplusplus
}
#endif

#endif
