#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>

#include "driverlib/uart.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"

#include <pcControl/pcControl.h>
#include <pcControl/ts480Emu/tm4cUsbSerial/usb_serial_structs.h>
#include <pcControl/ts480Emu/ts480Emu.h>


extern sig_atomic_t pcWakeup;
static tLineCoding curLineCoding;

void SetupTS480EmuSerial()
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_USB0);

    USBBufferInit(&g_sTxBuffer);
    USBBufferInit(&g_sRxBuffer);

    USBStackModeSet(0, eUSBModeForceDevice, 0);
    USBDCDCInit(0, &g_sCDCDevice);
}

static void SetControlLineState(uint16_t state)
{
}


static bool SetLineCoding(tLineCoding *pLineCoding)
{
    memcpy(&curLineCoding, pLineCoding, sizeof(tLineCoding));
    return true;
}

static void GetLineCoding(tLineCoding *pLineCoding)
{
    memcpy(pLineCoding, &curLineCoding, sizeof(tLineCoding));
}

static void
SendBreak(bool bSend)
{
    //I Don't think we care about breaks
}

uint32_t ControlHandler(void *pvCBData, uint32_t ui32Event,
                        uint32_t ui32MsgValue, void *pvMsgData)
{
    switch(ui32Event)
    {
        case USB_EVENT_CONNECTED:
            USBBufferFlush(&g_sTxBuffer);
            USBBufferFlush(&g_sRxBuffer);
            break;
        case USB_EVENT_DISCONNECTED:
            break;
        case USBD_CDC_EVENT_GET_LINE_CODING:
            GetLineCoding(pvMsgData);
            break;
        case USBD_CDC_EVENT_SET_LINE_CODING:
            SetLineCoding(pvMsgData);
            break;
        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
            SetControlLineState((uint16_t)ui32MsgValue);
            break;
        case USBD_CDC_EVENT_SEND_BREAK:
            SendBreak(true);
            break;
        case USBD_CDC_EVENT_CLEAR_BREAK:
            SendBreak(false);
            break;
        case USB_EVENT_SUSPEND:
        case USB_EVENT_RESUME:
            break;
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif

    }

    return(0);
}

extern uint32_t RxHandler(void *pCBData, uint32_t event,
                          uint32_t msgValue, void *pMsgData)
{
    size_t read;
    switch(event)
    {
    case USB_EVENT_RX_AVAILABLE:
        read = USBBufferRead(&g_sRxBuffer,
                             &protocolBuffer[protocolWriteIndex],
                             PROTOCOL_BUF_SIZE/2);
        protocolWriteIndex += read;
        protocolNewData = true;
        pcWakeup = true;
        break;
    case USB_EVENT_DATA_REMAINING:
        return 0;
    case USB_EVENT_REQUEST_BUFFER:
        return 0;
    default:
        break;
    }

    return 0;
}

extern uint32_t TxHandler(void *pvi32CBData, uint32_t ui32Event,
                          uint32_t ui32MsgValue, void *pvMsgData)
{

    return 0;
}

void SendTS480EmuSerial(uint8_t *pBuffer, size_t len)
{
    USBBufferWrite(&g_sTxBuffer, pBuffer, len);
}

void ProcessTS480EmuSerial()
{

}
