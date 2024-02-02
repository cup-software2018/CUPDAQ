
#ifndef AMOREADC_H
#define AMOREADC_H

#include <libusb.h>
#ifdef __cplusplus
extern "C" {
#endif

#define AMOREADC_VENDOR_ID (0x0547)
#define AMOREADC_PRODUCT_ID (0x1000)

extern int AMOREADCopen(int sid, libusb_context * ctx);
extern void AMOREADCclose(int sid);
extern int AMOREADCread_BCOUNT(int sid);
extern int AMOREADCread_DATA(int sid, int bcount, unsigned char * data);

#ifdef __cplusplus
}
#endif

#endif
