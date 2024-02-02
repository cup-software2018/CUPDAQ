
#ifndef M64ADCS_H
#define M64ADCS_H

#include <libusb.h>
#ifdef __cplusplus
extern "C" {
#endif

#define M64ADCS_VENDOR_ID (0x0547)
#define M64ADCS_PRODUCT_ID (0x1503)

extern int M64ADCSopen(int sid, libusb_context * ctx);
extern void M64ADCSclose(int sid);
extern void M64ADCSresetTIMER(int sid);
extern void M64ADCSreset(int sid);
extern void M64ADCSstart(int sid);
extern void M64ADCSstop(int sid);

extern unsigned long M64ADCSread_RUN(int sid);

extern void M64ADCSwrite_CW(int sid, unsigned long data);
extern unsigned long M64ADCSread_CW(int sid);

extern void M64ADCSwrite_DRAMON(int sid, unsigned long data);
extern unsigned long M64ADCSread_DRAMON(int sid);

extern unsigned long M64ADCSread_PED(int sid, unsigned long ch);

extern void M64ADCSwrite_DLY(int sid, unsigned long ch, unsigned long data);
extern unsigned long M64ADCSread_DLY(int sid, unsigned long ch);

extern void M64ADCSwrite_THR(int sid, unsigned long ch, unsigned long data);
extern unsigned long M64ADCSread_THR(int sid, unsigned long ch);

extern void M64ADCSwrite_PSW(int sid, unsigned long ch, unsigned long data);
extern unsigned long M64ADCSread_PSW(int sid, unsigned long ch);

extern int M64ADCSread_BCOUNT(int sid);

extern void M64ADCSwrite_PTRIG(int sid, unsigned long data);
extern unsigned long M64ADCSread_PTRIG(int sid);

extern void M64ADCSsend_TRIG(int sid);

extern void M64ADCSwrite_TM(int sid, unsigned long data);
extern unsigned long M64ADCSread_TM(int sid);

extern void M64ADCSwrite_MTHR(int sid, unsigned long data);
extern unsigned long M64ADCSread_MTHR(int sid);

extern void M64ADCSsend_ADCRST(int sid);
extern void M64ADCSsend_ADCCAL(int sid);
extern void M64ADCSwrite_ADCDLY(int sid, unsigned long ch, unsigned long data);
extern void M64ADCSwrite_ADCALIGN(int sid, unsigned long data);
extern unsigned long M64ADCSread_ADCSTAT(int sid);
extern void M64ADCSwrite_BITSLIP(int sid, unsigned long ch, unsigned long data);
extern void M64ADCSwrite_FMUX(int sid, unsigned long ch);
extern unsigned long M64ADCSread_FMUX(int sid);
extern void M64ADCSarm_FADC(int sid);
extern unsigned long M64ADCSread_FREADY(int sid);
extern void M64ADCS_ADCALIGN_64(int sid);
extern void M64ADCSread_FADCBUF(int sid, unsigned long * data);
extern int M64ADCSread_DATA(int sid, int bcount, unsigned char * data);

#ifdef __cplusplus
}
#endif

#endif
