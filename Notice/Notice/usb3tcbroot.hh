/*
 *
 */
#ifndef USB3TCBROOT_H
#define USB3TCBROOT_H

#include "TObject.h"
#include "Notice/nkusbconst.hh"

/* modification from usb3comroot.h
 * USB3TCBRead
 * USB3TCBWrite
 * USB3TCBReadReg
 * USB3TCBReadRegL
 */

struct libusb_context;
//struct libusb_device;
//struct libusb_device_handle;

class usb3tcbroot : public TObject {
public:

  usb3tcbroot(libusb_context ** ctx = 0);
  virtual ~usb3tcbroot();

  Int_t USB3TCBInit(libusb_context ** ctx = 0);
  void USB3TCBPrintOpenDevices();
  Int_t USB3TCBOpen(UShort_t vendor_id, UShort_t product_id, UChar_t sid = NK_SID_ANY, libusb_context * ctx = 0);
  Int_t USB3TCBClaimInterface(UShort_t vendor_id, UShort_t product_id, UChar_t sid = NK_SID_ANY, Int_t interface = 0);
  Int_t USB3TCBReleaseInterface(UShort_t vendor_id, UShort_t product_id, UChar_t sid = NK_SID_ANY, Int_t interface = 0);
  void USB3TCBClose(UShort_t vendor_id, UShort_t product_id, UChar_t sid = NK_SID_ANY);
  void USB3TCBExit(libusb_context * ctx = 0);
  Int_t USB3TCBWrite(UShort_t vendor_id, UShort_t product_id, UChar_t sid, UInt_t mid, UInt_t addr, UInt_t data);
  Int_t USB3TCBRead(UShort_t vendor_id,
                    UShort_t product_id,
                    UChar_t sid,
                    UInt_t mid,
                    UInt_t count,
                    UInt_t addr,
                    unsigned char * data);
  unsigned int USB3TCBReadReg(UShort_t vendor_id, UShort_t product_id, UChar_t sid, UInt_t mid, UInt_t addr);
  unsigned long USB3TCBReadRegL(UShort_t vendor_id, UShort_t product_id, UChar_t sid, UInt_t mid, UInt_t addr);
  unsigned char USB3TCBCheckFPGADone(UShort_t vendor_id, UShort_t product_id, UChar_t sid);
  void USB3TCBInitFPGA(UShort_t vendor_id, UShort_t product_id, UChar_t sid);
  unsigned char USB3TCBCheckFPGADoneNoAVR(UShort_t vendor_id, UShort_t product_id, UChar_t sid);
  void USB3TCBInitFPGANoAVR(UShort_t vendor_id, UShort_t product_id, UChar_t sid);

ClassDef(usb3tcbroot, 0) // USB3TCB wrapper class for root
};

#endif
