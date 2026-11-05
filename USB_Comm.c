#include "USB_Comm.h"
#include "App_State.h"
#include "App_UI.h"
#include "UART.h"

#include "USB_API/USB_Common/device.h"
#include "USB_API/USB_Common/types.h"
#include "USB_API/USB_Common/usb.h"
#include "USB_API/USB_CDC_API/UsbCdc.h"
#include "USB_config/descriptors.h"
#include "USB_User/usbConstructs.h"

#include <string.h>

/*
 * Initialize the USB peripheral and connect it when the cable is already
 * plugged in.  This is the same sequence as the SD-card reader demo.
 */
void USB_Comm_Init(void)
{
    USB_init();
    USB_setEnabledEvents(kUSB_allUsbEvents);

    /* Let the USB LDO / VBUS comparator settle before sampling VBUS. */
    __delay_cycles(120000);

    /* If the cable was already plugged in, there is no VBUS-on event.
     * Only connect when PUR is still low, so a VBUS ISR that has already
     * started enumeration is not reset a second time. */
    if ((USB_connectionInfo() & kUSB_vbusPresent) &&
        (USB_connectionState() == ST_USB_CONNECTED_NO_ENUM))
    {
        if (USB_enable() == kUSB_succeed)
        {
            USB_reset();
            USB_connect();
        }
    }
}

u8 USB_Comm_IsReady(void)
{
    return (USB_connectionState() == ST_ENUM_ACTIVE) ? 1 : 0;
}

/*
 * Poll the CDC OUT endpoint and feed received digits into the same password
 * buffer that the UART receive interrupt uses.
 */
void USB_Comm_Poll(void)
{
    static u8 rx_buf[128];
    WORD rx_cnt;
    WORD i;
    u8 added = 0;
    u8 input_state;

    if (!USB_Comm_IsReady()) return;

    /* Always drain the endpoint, so old bytes cannot appear later when the
     * user enters the password page. */
    rx_cnt = cdcReceiveDataInBuffer(rx_buf, sizeof(rx_buf), CDC0_INTFNUM);
    if (rx_cnt == 0) return;

    /* USB 数据同样先经过时间同步解析；时间帧中的数字不进入密码框 */
    input_state = (!g_is_locked &&
                   APP_STATE_IS_PASSWORD_INPUT(g_ui_state));

    for (i = 0; i < rx_cnt; i++)
    {
        u8 is_link = UART_Link_Parse(rx_buf[i]);
        u8 is_time_sync = is_link ? 0 : UART_TimeSync_Parse(rx_buf[i]);

        if (!is_link && !is_time_sync && input_state &&
            rx_buf[i] >= '0' && rx_buf[i] <= '9' && g_pwd_cnt < 6)
        {
            g_pwd_buf[g_pwd_cnt++] = rx_buf[i];
            added = 1;
        }
    }

    if (added)
    {
        Restore_Hint_Line();
        Update_Input_UI(g_pwd_cnt);
    }
}

/*
 * Send a string through the CDC IN endpoint.  If USB is not enumerated the
 * call is silently ignored so that the normal UART path is never blocked.
 */
void USB_Comm_SendString(const unsigned char *str)
{
    WORD len;

    if (str == 0 || *str == 0) return;
    if (!USB_Comm_IsReady()) return;

    len = (WORD)strlen((const char *)str);
    if (len == 0) return;

    /* 1000 retries: never let a stalled USB host block the password UI. */
    cdcSendDataInBackground((BYTE *)str, len, CDC0_INTFNUM, 1000);
}

/*
 * System NMI: clear oscillator faults and recover from a USB bus error in
 * the same way as the SD-card reader demo.
 */
#pragma vector = UNMI_VECTOR
__interrupt void UNMI_ISR(void)
{
    switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG))
    {
        case SYSUNIV_NONE:
            break;

        case SYSUNIV_NMIIFG:
            break;

        case SYSUNIV_OFIFG:
            UCSCTL7 &= ~(DCOFFG + XT1LFOFFG + XT2OFFG);
            SFRIFG1 &= ~OFIFG;
            break;

        case SYSUNIV_ACCVIFG:
            break;

        case SYSUNIV_BUSIFG:
            SYSBERRIV = 0;
            USB_disable();
            break;

        default:
            break;
    }
}
