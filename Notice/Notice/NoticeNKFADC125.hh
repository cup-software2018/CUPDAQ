
#ifndef NKFADC125_H
#define NKFADC125_H

#include <libusb.h>
#ifdef __cplusplus
extern "C" {
#endif

#define NKFADC125_VENDOR_ID (0x0547)
#define NKFADC125_PRODUCT_ID (0x1903)

extern int NKFADC125open(int sid, libusb_context * ctx);
extern void NKFADC125close(int sid);
extern int NKFADC125read_BCOUNT(int sid);
extern int NKFADC125read_DATA(int sid, int bcount, unsigned char * data, unsigned int timeout = 0);

#ifdef __cplusplus
}
#endif

#endif
