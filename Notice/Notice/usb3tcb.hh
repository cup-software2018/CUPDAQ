#ifndef USB3TCB_H
#define USB3TCB_H

#include <libusb.h>

struct libusb_context;

#ifdef __cplusplus
extern "C" {
#endif

#define USB3_SF_READ (0x82)
#define USB3_SF_WRITE (0x06)

// request command
#define NKPROGRAMMER_CMD_INIT          (10)
#define NKPROGRAMMER_CMD_CHECK_DONE    (11)

// Request target 
#define NKPROGRAMMER_TARGET_FPGA            (0xF0)
#define NKPROGRAMMER_TARGET_AVR             (0xF1)
#define NKPROGRAMMER_TARGET_CLOCKDRIVER     (0xF2)
#define NKPROGRAMMER_TARGET_USB3            (0xF3)
#define NKPROGRAMMER_RESET_FIFO             (0xD7)
#define NKPROGRAMMER_CHECK_DONE             (0xD8)

// Request commands 
#define NKPROGRAMMER_CMD_WRITE_DATA         (0)
#define NKPROGRAMMER_CMD_READ_DATA          (1)
#define NKPROGRAMMER_CMD_WRITE_REGISTER     (2)
#define NKPROGRAMMER_CMD_READ_REGISTER      (3)
#define NKPROGRAMMER_CMD_RESET              (4)
#define NKPROGRAMMER_CMD_ERASE_BLOCK        (5)
#define NKPROGRAMMER_CMD_ERASE_CHIP         (6)
#define NKPROGRAMMER_CMD_ENTER_PROGRAM_MODE (7)
#define NKPROGRAMMER_CMD_LEAVE_PROGRAM_MODE (8)
#define NKPROGRAMMER_CMD_SYNC               (9)
#define NKPROGRAMMER_CMD_INIT              (10)
#define NKPROGRAMMER_CMD_CHECK_DONE        (11)

extern int USB3TCBInit(libusb_context ** ctx);
extern int USB3TCBOpen(uint16_t vendor_id, uint16_t product_id, int sid, libusb_context * ctx);
extern void USB3TCBPrintOpenDevices(void);
extern int USB3TCBClaimInterface(uint16_t vendor_id, uint16_t product_id, int sid, int interface);
extern int USB3TCBReleaseInterface(uint16_t vendor_id, uint16_t product_id, int sid, int interface);
extern void USB3TCBClose(uint16_t vendor_id, uint16_t product_id, int sid);
extern void USB3TCBExit(libusb_context * ctx);
extern int USB3TCBWrite(uint16_t vendor_id, uint16_t product_id, int sid, uint32_t mid, uint32_t addr, uint32_t data);
extern int USB3TCBRead(uint16_t vendor_id,
                       uint16_t product_id,
                       int sid,
                       uint32_t mid,
                       uint32_t count,
                       uint32_t addr,
                       unsigned char * data);
extern int USB3TCBWriteControl(uint16_t vendor_id,
                               uint16_t product_id,
                               int sid,
                               uint8_t bRequest,
                               uint16_t wValue,
                               uint16_t wIndex,
                               unsigned char * data,
                               uint16_t wLength);
extern int USB3TCBReadControl(uint16_t vendor_id,
                              uint16_t product_id,
                              int sid,
                              uint8_t bRequest,
                              uint16_t wValue,
                              uint16_t wIndex,
                              unsigned char * data,
                              uint16_t wLength);
extern unsigned int USB3TCBReadReg(uint16_t vendor_id, uint16_t product_id, int sid, uint32_t mid, uint32_t addr);
extern unsigned long USB3TCBReadRegL(uint16_t vendor_id, uint16_t product_id, int sid, uint32_t mid, uint32_t addr);
extern unsigned char USB3TCBCheckFPGADone(uint16_t vendor_id, uint16_t product_id, int sid);
extern void USB3TCBInitFPGA(uint16_t vendor_id, uint16_t product_id, int sid);
extern unsigned char USB3TCBCheckFPGADoneNoAVR(uint16_t vendor_id, uint16_t product_id, int sid);
extern void USB3TCBInitFPGANoAVR(uint16_t vendor_id, uint16_t product_id, int sid);

#ifdef __cplusplus
}
#endif

#endif

