/*
 *
 */
#include "Notice/usb3com.hh"
#include "Notice/usb3comroot.hh"

ClassImp(usb3comroot)

///////////////////////////////////////////////////////////////////////////////
usb3comroot::usb3comroot(libusb_context ** ctx)
{
}

///////////////////////////////////////////////////////////////////////////////
usb3comroot::~usb3comroot()
{
}

///////////////////////////////////////////////////////////////////////////////
Int_t usb3comroot::USB3Init(libusb_context ** ctx)
{
  return ::USB3Init(ctx);
}

///////////////////////////////////////////////////////////////////////////////
void usb3comroot::USB3PrintOpenDevices()
{
  ::USB3PrintOpenDevices();
}

///////////////////////////////////////////////////////////////////////////////
Int_t usb3comroot::USB3Open(UShort_t vendor_id, UShort_t product_id, UChar_t sid, libusb_context * ctx)
{
  return ::USB3Open(vendor_id, product_id, sid, ctx);
}

///////////////////////////////////////////////////////////////////////////////
Int_t usb3comroot::USB3ClaimInterface(UShort_t vendor_id, UShort_t product_id, UChar_t sid, Int_t interface)
{
  return ::USB3ClaimInterface(vendor_id, product_id, sid, interface);
}

///////////////////////////////////////////////////////////////////////////////
Int_t usb3comroot::USB3ReleaseInterface(UShort_t vendor_id, UShort_t product_id, UChar_t sid, Int_t interface)
{
  return ::USB3ReleaseInterface(vendor_id, product_id, sid, interface);
}

///////////////////////////////////////////////////////////////////////////////
void usb3comroot::USB3Close(UShort_t vendor_id, UShort_t product_id, UChar_t sid)
{
  ::USB3Close(vendor_id, product_id, sid);
}

///////////////////////////////////////////////////////////////////////////////
void usb3comroot::USB3Exit(libusb_context * ctx)
{
  ::USB3Exit(ctx);
}

///////////////////////////////////////////////////////////////////////////////
Int_t usb3comroot::USB3Write(UShort_t vendor_id, UShort_t product_id, UChar_t sid, UInt_t addr, UInt_t data)
{
  return ::USB3Write(vendor_id, product_id, sid, addr, data);
}

///////////////////////////////////////////////////////////////////////////////
Int_t usb3comroot::USB3Read(UShort_t vendor_id,
                            UShort_t product_id,
                            UChar_t sid,
                            UInt_t count,
                            UInt_t addr,
                            unsigned char * data)
{
  return ::USB3Read(vendor_id, product_id, sid, count, addr, data);
}

///////////////////////////////////////////////////////////////////////////////
unsigned int usb3comroot::USB3ReadReg(UShort_t vendor_id, UShort_t product_id, UChar_t sid, UInt_t addr)
{
  return ::USB3ReadReg(vendor_id, product_id, sid, addr);
}

///////////////////////////////////////////////////////////////////////////////
unsigned long usb3comroot::USB3ReadRegL(UShort_t vendor_id, UShort_t product_id, UChar_t sid, UInt_t addr)
{
  return ::USB3ReadRegL(vendor_id, product_id, sid, addr);
}

///////////////////////////////////////////////////////////////////////////////
unsigned char usb3comroot::USB3CheckFPGADone(uint16_t vendor_id, uint16_t product_id, unsigned char sid)
{
  return ::USB3CheckFPGADone(vendor_id, product_id, sid);
}

///////////////////////////////////////////////////////////////////////////////
void usb3comroot::USB3InitFPGA(uint16_t vendor_id, uint16_t product_id, unsigned char sid)
{
  ::USB3InitFPGA(vendor_id, product_id, sid);
}

///////////////////////////////////////////////////////////////////////////////
unsigned char usb3comroot::USB3CheckFPGADoneNoAVR(uint16_t vendor_id, uint16_t product_id, unsigned char sid)
{
  return ::USB3CheckFPGADoneNoAVR(vendor_id, product_id, sid);
}

///////////////////////////////////////////////////////////////////////////////
void usb3comroot::USB3InitFPGANoAVR(uint16_t vendor_id, uint16_t product_id, unsigned char sid)
{
  ::USB3InitFPGANoAVR(vendor_id, product_id, sid);
}

///////////////////////////////////////////////////////////////////////////////
void usb3comroot::USB3ResetEP2(uint16_t vendor_id, uint16_t product_id, unsigned char sid)
{
  ::USB3ResetEP2(vendor_id, product_id, sid);
}

///////////////////////////////////////////////////////////////////////////////
void usb3comroot::USB3ResetEP6(uint16_t vendor_id, uint16_t product_id, unsigned char sid)
{
  ::USB3ResetEP6(vendor_id, product_id, sid);
}
