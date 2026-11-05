#ifndef __USB_COMM_H
#define __USB_COMM_H

#include "Hardware_Profile.h"

/*
 * USB CDC (virtual COM port) communication.
 *
 * The initialization / connection sequence follows the USB logic used by
 * the SD-card reader demo:
 *     USB_init()
 *     USB_setEnabledEvents(kUSB_allUsbEvents)
 *     if VBUS is present: USB_enable() -> USB_reset() -> USB_connect()
 */
void USB_Comm_Init(void);
void USB_Comm_Poll(void);
void USB_Comm_SendString(const unsigned char *str);
u8 USB_Comm_IsReady(void);

#endif
