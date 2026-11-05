// USB event callbacks.
// The host-visible device is CDC (virtual COM port) only.  The MSC/HID
// sources are still part of the CCS project because the TI USB ISR keeps
// their endpoint slots, so the unused entry points are provided as no-ops.

#include "..\USB_API\USB_Common\device.h"
#include "..\USB_API\USB_Common\types.h"
#include "descriptors.h"
#include "..\USB_API\USB_Common\usb.h"
#include "..\USB_API\USB_CDC_API\UsbCdc.h"
#include "..\USB_API\USB_HID_API\UsbHid.h"
#include "..\USB_API\USB_MSC_API\UsbMsc.h"

/* ------------------------------------------------------------------------- */
/* System level USB events                                                   */
/* ------------------------------------------------------------------------- */
BYTE USB_handleClockEvent(VOID)
{
    return TRUE;
}

BYTE USB_handleVbusOnEvent(VOID)
{
    if (USB_enable() == kUSB_succeed)
    {
        USB_reset();
        USB_connect();
    }
    return TRUE;
}

BYTE USB_handleVbusOffEvent(VOID)
{
    return TRUE;
}

BYTE USB_handleResetEvent(VOID)
{
    return TRUE;
}

BYTE USB_handleSuspendEvent(VOID)
{
    return TRUE;
}

BYTE USB_handleResumeEvent(VOID)
{
    return TRUE;
}

BYTE USB_handleEnumCompleteEvent(VOID)
{
    return TRUE;
}

/* ------------------------------------------------------------------------- */
/* CDC (virtual COM port) events                                             */
/* ------------------------------------------------------------------------- */
BYTE USBCDC_handleDataReceived(BYTE intfNum)
{
    return TRUE;
}

BYTE USBCDC_handleSendCompleted(BYTE intfNum)
{
    return FALSE;
}

BYTE USBCDC_handleReceiveCompleted(BYTE intfNum)
{
    return FALSE;
}

BYTE USBCDC_handleSetLineCoding(BYTE intfNum, ULONG lBaudrate)
{
    return FALSE;
}

BYTE USBCDC_handleSetControlLineState(BYTE intfNum, BYTE lineState)
{
    return FALSE;
}

/* ------------------------------------------------------------------------- */
/* Unused MSC/HID entry points required by the shared USB ISR.               */
/* _MSC_ and _HID_ are not defined, so the real implementations are not      */
/* compiled into this project.                                               */
/* ------------------------------------------------------------------------- */
BYTE usbGetHidDescriptor(VOID) { return 0; }
BYTE usbGetIdle(VOID) { return 0; }
BYTE usbGetProtocol(VOID) { return 0; }
BYTE usbGetReport(VOID) { return 0; }
BYTE usbGetReportDescriptor(VOID) { return 0; }
BYTE usbSetIdle(VOID) { return 0; }
BYTE usbSetProtocol(VOID) { return 0; }
BYTE usbSetReport(VOID) { return 0; }

BYTE HidIsReceiveInProgress(BYTE intfNum) { return 0; }
BYTE HidToBufferFromHost(BYTE intfNum) { return 0; }
BYTE HidToHostFromBuffer(BYTE intfNum) { return 0; }
BYTE USBHID_handleDataReceived(BYTE intfNum) { return 0; }

BYTE USBMSC_reset(VOID) { return 0; }
BOOL MSCFromHostToBuffer(VOID) { return FALSE; }
BOOL MSCToHostFromBuffer(VOID) { return FALSE; }
BYTE Get_MaxLUN(VOID) { return 0; }
