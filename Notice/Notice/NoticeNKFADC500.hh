
#ifndef NKFADC500_H
#define NKFADC500_H

#include <libusb.h>
#ifdef __cplusplus
extern "C" {
#endif

#define NKFADC500_VENDOR_ID (0x0547)
#define NKFADC500_PRODUCT_ID (0x1502)

extern int NKFADC500open(int sid, libusb_context * ctx);
extern void NKFADC500close(int sid);
extern int NKFADC500read_BCOUNT(int sid);
extern int NKFADC500read_DATA(int sid, int bcount, unsigned char * data, unsigned int timeout = 0);

#ifdef __cplusplus
}
#endif

#endif
