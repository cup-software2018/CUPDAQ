/*
 *
 */
#ifndef USB3COMROOT_H
#define USB3COMROOT_H

#include "TObject.h"
#include "Notice/nkusbconst.hh"

struct libusb_context;
struct libusb_device;
struct libusb_device_handle;

class usb3comroot : public TObject {
public:

  usb3comroot(libusb_context ** ctx = 0);
  virtual ~usb3comroot();

  Int_t USB3Init(libusb_context ** ctx = 0);
  void USB3PrintOpenDevices();
  Int_t USB3Open(UShort_t vendor_id, UShort_t product_id, UChar_t sid = NK_SID_ANY, libusb_context * ctx = 0);
  Int_t USB3ClaimInterface(UShort_t vendor_id, UShort_t product_id, UChar_t sid = NK_SID_ANY, Int_t interface = 0);
  Int_t USB3ReleaseInterface(UShort_t vendor_id, UShort_t product_id, UChar_t sid = NK_SID_ANY, Int_t interface = 0);
  void USB3Close(UShort_t vendor_id, UShort_t product_id, UChar_t sid = NK_SID_ANY);
  void USB3Exit(libusb_context * ctx = 0);
  Int_t USB3Write(UShort_t vendor_id, UShort_t product_id, UChar_t sid, UInt_t addr, UInt_t data);
  Int_t USB3Read(UShort_t vendor_id, UShort_t product_id, UChar_t sid, UInt_t count, UInt_t addr, unsigned char * data);
  unsigned int USB3ReadReg(UShort_t vendor_id, UShort_t product_id, UChar_t sid, UInt_t addr);
  unsigned long USB3ReadRegL(UShort_t vendor_id, UShort_t product_id, UChar_t sid, UInt_t addr);
  unsigned char USB3CheckFPGADone(UShort_t vendor_id, UShort_t product_id, UChar_t sid);
  void USB3InitFPGA(UShort_t vendor_id, UShort_t product_id, UChar_t sid);
  unsigned char USB3CheckFPGADoneNoAVR(UShort_t vendor_id, UShort_t product_id, UChar_t sid);
  void USB3InitFPGANoAVR(UShort_t vendor_id, UShort_t product_id, UChar_t sid);
  void USB3ResetEP2(UShort_t vendor_id, UShort_t product_id, UChar_t sid);
  void USB3ResetEP6(UShort_t vendor_id, UShort_t product_id, UChar_t sid);

ClassDef(usb3comroot, 0) // USB3COM wrapper class for root
};

#endif
