/*
 * libusb example program to manipulate U.are.U 4000B fingerprint scanner.
 * Copyright (C) 2007 Daniel Drake <dsd@gentoo.org>
 *
 * Basic image capture program only, does not consider the powerup quirks or
 * the fact that image encryption may be enabled. Not expected to work
 * flawlessly all of the time.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#ifndef NKUSB_H
#define NKUSB_H

#include <libusb.h>
#include "Notice/nkusbconst.hh"

#ifdef __cplusplus
extern "C" {
#endif

//extern struct dev_open *ldev_open;
extern int nkusb_init(libusb_context ** ctx);
extern int nkusb_open_device(uint16_t vendor_id, uint16_t product_id, int sid, libusb_context * ctx);
extern int nkusb_claim_interface(uint16_t vendor_id, uint16_t product_id, int sid, int interface);
extern int nkusb_release_interface(uint16_t vendor_id, uint16_t product_id, int sid, int interface);
extern void nkusb_close_device(uint16_t vendor_id, uint16_t product_id, int sid);
extern void nkusb_exit(libusb_context * ctx);
extern void nkusb_print_open_devices(void);
extern int nkusb_is_device_open(uint16_t vendor_id, uint16_t product_id, int sid);
extern libusb_device_handle * nkusb_get_device_handle(uint16_t vendor_id, uint16_t product_id, int sid);

#ifdef __cplusplus
}
#endif

#endif
