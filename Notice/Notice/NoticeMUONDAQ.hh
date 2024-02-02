
#ifndef MUONDAQ_H
#define MUONDAQ_H

#include <libusb.h>
#ifdef __cplusplus
extern "C" {
#endif

#define MUONDAQ_VENDOR_ID (0x0547)
#define MUONDAQ_PRODUCT_ID (0x2010)

extern int MUONDAQopen(int sid, libusb_context *ctx);
extern void MUONDAQclose(int sid);
extern int MUONDAQread_BCOUNT(int sid);
extern int MUONDAQread_DATA(int sid, int bcount, unsigned char * data, unsigned int timeout = 0);
extern void MUONDAQflush_DATA(int sid);

#ifdef __cplusplus
}
#endif

#endif
