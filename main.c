#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/debug.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/pwm.h"
#include "driverlib/ssi.h"
#include "driverlib/systick.h"
#include "utils/uartstdio.h"

#include "pinout.h"
#include "utils/delay.h"

#include "platform/platform.h"
#include "input/input.h"
#include "display/display.h"
#include "powmon/powmon.h"
#include "pcControl/pcControl.h"
#include "radio/radio.h"
#include "storage/storage.h"
#include "ufp_config.h"

#define CLOCK_ROLLOVER  (MsToCycles(CYCLE_MS))

#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

sig_atomic_t sysTickWakeup;
sig_atomic_t pcWakeup;

void SysTickHandler(void)
{
    sysTickWakeup = true;
}

void ConfigureUART(void)
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);
    UARTStdioConfig(0, 115200, 16000000);
}

static void SetupSleep()
{
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_USB0);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_QEI0);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPeripheralClockGating(true);

    sysTickWakeup = false;
    SysTickEnable();
    SysTickPeriodSet(CLOCK_ROLLOVER);
    SysTickIntEnable();

    pcWakeup = false;

}

int main(void)
{
    ROM_FPULazyStackingEnable();
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);

    PinoutSet();
    ConfigureUART();

    SetupPlatform();
    SetupInput();
    SetupDisplay();
    SetupStorage();
    SetupPowMon();
    SetupPcControl();
    SetupRadio();

    SetupSleep();

    UARTprintf("UFP Starting...\n");

    while(1)
    {
        while (sysTickWakeup == true || pcWakeup == true)
        {
            if (sysTickWakeup == true)
            {
                ProcessInput();
                ProcessPowMon();
                ProcessDisplay();
                ProcessStorage();

                sysTickWakeup = false;
            }

            if (pcWakeup == true)
            {
                ProcessPcControl();

                pcWakeup = false;
            }

            ProcessPlatform();

            ProcessRadio();
        }


        SysCtlSleep();
    }
}
