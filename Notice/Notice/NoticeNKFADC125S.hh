
#ifndef NKFADC125S_H
#define NKFADC125S_H

#include <libusb.h>
#ifdef __cplusplus
extern "C" {
#endif

#define NKFADC125S_VENDOR_ID (0x0547)
#define NKFADC125S_PRODUCT_ID (0x1903)

extern int NKFADC125Sopen(int sid, libusb_context * ctx);
extern void NKFADC125Sclose(int sid);
extern void NKFADC125Sreset(int sid);
extern void NKFADC125SresetTIMER(int sid);
extern void NKFADC125Sstart(int sid);
extern void NKFADC125Sstop(int sid);
extern unsigned long NKFADC125Sread_RUN(int sid);
extern void NKFADC125Swrite_CW(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC125Sread_CW(int sid, unsigned long ch);
extern void NKFADC125Swrite_RL(int sid, unsigned long data);
extern unsigned long NKFADC125Sread_RL(int sid);
extern void NKFADC125Swrite_DRAMON(int sid, unsigned long data);
extern unsigned long NKFADC125Sread_DRAMON(int sid);
extern void NKFADC125Swrite_DACOFF(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC125Sread_DACOFF(int sid, unsigned long ch);
extern void NKFADC125Smeasure_PED(int sid, unsigned long ch);
extern unsigned long NKFADC125Sread_PED(int sid, unsigned long ch);
extern void NKFADC125Swrite_DLY(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC125Sread_DLY(int sid, unsigned long ch);
extern void NKFADC125Swrite_THR(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC125Sread_THR(int sid, unsigned long ch);
extern void NKFADC125Swrite_POL(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC125Sread_POL(int sid, unsigned long ch);
extern void NKFADC125Swrite_PSW(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC125Sread_PSW(int sid, unsigned long ch);
extern void NKFADC125Swrite_PCT(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC125Sread_PCT(int sid, unsigned long ch);
extern void NKFADC125Swrite_PCI(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC125Sread_PCI(int sid, unsigned long ch);
extern void NKFADC125Swrite_PWT(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC125Sread_PWT(int sid, unsigned long ch);
extern void NKFADC125Swrite_DT(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC125Sread_DT(int sid, unsigned long ch);
extern unsigned long NKFADC125Sread_BCOUNT(int sid);
extern void NKFADC125Swrite_PTRIG(int sid, unsigned long data);
extern unsigned long NKFADC125Sread_PTRIG(int sid);
extern void NKFADC125Ssend_TRIG(int sid);
extern void NKFADC125Swrite_TRIGENABLE(int sid, unsigned long data);
extern unsigned long NKFADC125Sread_TRIGENABLE(int sid);
extern void NKFADC125Swrite_TM(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC125Sread_TM(int sid, unsigned long ch);
extern void NKFADC125Swrite_TLT(int sid, unsigned long data);
extern unsigned long NKFADC125Sread_TLT(int sid);
extern void NKFADC125Swrite_PSCALE(int sid, unsigned long data);
extern unsigned long NKFADC125Sread_PSCALE(int sid);
extern void NKFADC125Swrite_DSR(int sid, unsigned long data);
extern unsigned long NKFADC125Sread_DSR(int sid);
extern unsigned long NKFADC125Sread_DATA(int sid, int bcount, unsigned char * data);
extern void NKFADC125S_ADCALIGN_125(int sid);
extern void NKFADC125S_ADCALIGN_DRAM(int sid);
extern void NKFADC125Sflush_DATA(int sid);
extern void NKFADC125Ssend_ADCRST(int sid);
extern void NKFADC125Ssend_ADCCAL(int sid);
extern void NKFADC125Swrite_ADCDLY(int sid, unsigned long ch, unsigned long data);
extern void NKFADC125Swrite_ADCsetup(int sid, unsigned long addr, unsigned long data);
extern unsigned long NKFADC125Sread_ADCSTAT(int sid);
extern void NKFADC125Swrite_DRAMDLY(int sid, unsigned long ch, unsigned long data);
extern void NKFADC125Swrite_DRAMBITSLIP(int sid, unsigned long ch);
extern void NKFADC125Swrite_DRAMTEST(int sid, unsigned long data);
extern unsigned long NKFADC125Sread_DRAMTEST(int sid, unsigned long ch);
extern void NKFADC125Swrite_ADCBITSLIP(int sid, unsigned long ch, unsigned long data);
extern unsigned long NKFADC125Sread_CHNUM(int sid);
#ifdef __cplusplus
}
#endif

#endif
