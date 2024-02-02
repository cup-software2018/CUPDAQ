
#ifndef M64ADC_H
#define M64ADC_H

#include <libusb.h>
#ifdef __cplusplus
extern "C" {
#endif

#define M64ADC_VENDOR_ID (0x0547)
#define M64ADC_PRODUCT_ID (0x1503)

extern int M64ADCopen(int sid, libusb_context * ctx);
extern void M64ADCclose(int sid);
extern int M64ADCread_BCOUNT(int sid);
extern int M64ADCread_DATA(int sid, int bcount, unsigned char * data);

#ifdef __cplusplus
}
#endif

#endif
