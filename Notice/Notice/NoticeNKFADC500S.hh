
#ifndef NKFADC500S_H
#define NKFADC500S_H

#include <libusb.h>
#ifdef __cplusplus
extern "C" {
#endif

#define NKFADC500S_VENDOR_ID (0x0547)
#define NKFADC500S_PRODUCT_ID (0x1502)

extern int NKFADC500Sopen(int sid, libusb_context *ctx);
extern void NKFADC500Sclose(int sid);
extern void NKFADC500Sreset(int sid);
extern void NKFADC500SresetTIMER(int sid);
extern void NKFADC500Sstart(int sid);
extern void NKFADC500Sstop(int sid);
extern unsigned long NKFADC500Sread_RUN(int sid);
extern void NKFADC500Swrite_CW(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC500Sread_CW(int sid, unsigned long ch);
extern void NKFADC500Swrite_RL(int sid, unsigned long data);
extern unsigned long NKFADC500Sread_RL(int sid);
extern void NKFADC500Swrite_DRAMON(int sid, unsigned long data);
extern unsigned long NKFADC500Sread_DRAMON(int sid);
extern void NKFADC500Swrite_DACOFF(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC500Sread_DACOFF(int sid, unsigned long ch);
extern void NKFADC500Smeasure_PED(int sid, unsigned long ch);
extern unsigned long NKFADC500Sread_PED(int sid, unsigned long ch);
extern void NKFADC500Swrite_DLY(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC500Sread_DLY(int sid, unsigned long ch);
extern void NKFADC500Swrite_THR(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC500Sread_THR(int sid, unsigned long ch);
extern void NKFADC500Swrite_POL(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC500Sread_POL(int sid, unsigned long ch);
extern void NKFADC500Swrite_PSW(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC500Sread_PSW(int sid, unsigned long ch);
extern void NKFADC500Swrite_AMODE(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC500Sread_AMODE(int sid, unsigned long ch);
extern void NKFADC500Swrite_PCT(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC500Sread_PCT(int sid, unsigned long ch);
extern void NKFADC500Swrite_PCI(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC500Sread_PCI(int sid, unsigned long ch);
extern void NKFADC500Swrite_PWT(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC500Sread_PWT(int sid, unsigned long ch);
extern void NKFADC500Swrite_DT(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC500Sread_DT(int sid, unsigned long ch);
extern unsigned long NKFADC500Sread_BCOUNT(int sid);
extern void NKFADC500Swrite_PTRIG(int sid, unsigned long data);
extern unsigned long NKFADC500Sread_PTRIG(int sid);
extern void NKFADC500Ssend_TRIG(int sid);
extern void NKFADC500Swrite_TRIGENABLE(int sid, unsigned long data);
extern unsigned long NKFADC500Sread_TRIGENABLE(int sid);
extern void NKFADC500Swrite_TM(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC500Sread_TM(int sid, unsigned long ch);
extern void NKFADC500Swrite_TLT(int sid, unsigned long data);
extern unsigned long NKFADC500Sread_TLT(int sid);
extern void NKFADC500Swrite_ZEROSUP(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC500Sread_ZEROSUP(int sid, unsigned long ch);
extern void NKFADC500Ssend_ADCRST(int sid);
extern void NKFADC500Ssend_ADCCAL(int sid);
extern void NKFADC500Swrite_ADCDLY(int sid, unsigned long ch, unsigned long data);
extern void NKFADC500Swrite_ADCALIGN(int sid, unsigned long data);
extern unsigned long NKFADC500Sread_ADCSTAT(int sid);
extern void NKFADC500Swrite_DRAMDLY(int sid, unsigned long ch, unsigned long data);
extern void NKFADC500Swrite_BITSLIP(int sid, unsigned long ch);
extern void NKFADC500Swrite_DRAMTEST(int sid, unsigned long data);
extern unsigned long NKFADC500Sread_DRAMTEST(int sid, unsigned long ch);
extern void NKFADC500Swrite_PSCALE(int sid, unsigned long data);
extern unsigned long NKFADC500Sread_PSCALE(int sid);
extern void NKFADC500Swrite_DSR(int sid, unsigned long data);
extern unsigned long NKFADC500Sread_DSR(int sid);
extern int NKFADC500Sread_DATA(int sid, int bcount, unsigned char *data, unsigned int timeout = 0);
extern void NKFADC500S_ADCALIGN_500(int sid);
extern void NKFADC500S_ADCALIGN_DRAM(int sid);
#ifdef __cplusplus
}
#endif

#endif
