/*
 *
 */
#include "Notice/usb3tcb.hh"
#include "Notice/usb3tcbroot.hh"

/* modification from usb3comroot.cc
 * USB3TCBRead
 * USB3TCBWrite
 * USB3TCBReadReg
 * USB3TCBReadRegL
 */

ClassImp(usb3tcbroot)

///////////////////////////////////////////////////////////////////////////////
usb3tcbroot::usb3tcbroot(libusb_context ** ctx)
{
}

///////////////////////////////////////////////////////////////////////////////
usb3tcbroot::~usb3tcbroot()
{
}

///////////////////////////////////////////////////////////////////////////////
Int_t usb3tcbroot::USB3TCBInit(libusb_context ** ctx)
{
  return ::USB3TCBInit(ctx);
}

///////////////////////////////////////////////////////////////////////////////
void usb3tcbroot::USB3TCBPrintOpenDevices()
{
  ::USB3TCBPrintOpenDevices();
}

///////////////////////////////////////////////////////////////////////////////
Int_t usb3tcbroot::USB3TCBOpen(UShort_t vendor_id, UShort_t product_id, UChar_t sid, libusb_context * ctx)
{
  return ::USB3TCBOpen(vendor_id, product_id, sid, ctx);
}

///////////////////////////////////////////////////////////////////////////////
Int_t usb3tcbroot::USB3TCBClaimInterface(UShort_t vendor_id, UShort_t product_id, UChar_t sid, Int_t interface)
{
  return ::USB3TCBClaimInterface(vendor_id, product_id, sid, interface);
}

///////////////////////////////////////////////////////////////////////////////
Int_t usb3tcbroot::USB3TCBReleaseInterface(UShort_t vendor_id, UShort_t product_id, UChar_t sid, Int_t interface)
{
  return ::USB3TCBReleaseInterface(vendor_id, product_id, sid, interface);
}

///////////////////////////////////////////////////////////////////////////////
void usb3tcbroot::USB3TCBClose(UShort_t vendor_id, UShort_t product_id, UChar_t sid)
{
  ::USB3TCBClose(vendor_id, product_id, sid);
}

///////////////////////////////////////////////////////////////////////////////
void usb3tcbroot::USB3TCBExit(libusb_context * ctx)
{
  ::USB3TCBExit(ctx);
}

///////////////////////////////////////////////////////////////////////////////
Int_t usb3tcbroot::USB3TCBWrite(UShort_t vendor_id,
                                UShort_t product_id,
                                UChar_t sid,
                                UInt_t mid,
                                UInt_t addr,
                                UInt_t data)
{
  return ::USB3TCBWrite(vendor_id, product_id, sid, mid, addr, data);
}

///////////////////////////////////////////////////////////////////////////////
Int_t usb3tcbroot::USB3TCBRead(UShort_t vendor_id,
                               UShort_t product_id,
                               UChar_t sid,
                               UInt_t mid,
                               UInt_t count,
                               UInt_t addr,
                               unsigned char * data)
{
  return ::USB3TCBRead(vendor_id, product_id, sid, mid, count, addr, data);
}

///////////////////////////////////////////////////////////////////////////////
unsigned int usb3tcbroot::USB3TCBReadReg(UShort_t vendor_id, UShort_t product_id, UChar_t sid, UInt_t mid, UInt_t addr)
{
  return ::USB3TCBReadReg(vendor_id, product_id, sid, mid, addr);
}

///////////////////////////////////////////////////////////////////////////////
unsigned long usb3tcbroot::USB3TCBReadRegL(UShort_t vendor_id,
                                           UShort_t product_id,
                                           UChar_t sid,
                                           UInt_t mid,
                                           UInt_t addr)
{
  return ::USB3TCBReadRegL(vendor_id, product_id, sid, mid, addr);
}

///////////////////////////////////////////////////////////////////////////////
unsigned char usb3tcbroot::USB3TCBCheckFPGADone(uint16_t vendor_id, uint16_t product_id, unsigned char sid)
{
  return ::USB3TCBCheckFPGADone(vendor_id, product_id, sid);
}

///////////////////////////////////////////////////////////////////////////////
void usb3tcbroot::USB3TCBInitFPGA(uint16_t vendor_id, uint16_t product_id, unsigned char sid)
{
  ::USB3TCBInitFPGA(vendor_id, product_id, sid);
}

///////////////////////////////////////////////////////////////////////////////
unsigned char usb3tcbroot::USB3TCBCheckFPGADoneNoAVR(uint16_t vendor_id, uint16_t product_id, unsigned char sid)
{
  return ::USB3TCBCheckFPGADoneNoAVR(vendor_id, product_id, sid);
}

///////////////////////////////////////////////////////////////////////////////
void usb3tcbroot::USB3TCBInitFPGANoAVR(uint16_t vendor_id, uint16_t product_id, unsigned char sid)
{
  ::USB3TCBInitFPGANoAVR(vendor_id, product_id, sid);
}
