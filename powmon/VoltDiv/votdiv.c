#include <stdint.h>
#include <stdbool.h>

#include "ufp_config.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/gpio.h"

#include "platform/platform.h"
#include "utils/delay.h"

#define ADC_MAX_MV      3300
#define ADC_MAX_MEAS    4096
#define MAGIC_VOLTDIV   2447
#define MAGIC_TO_MV     1000
#define VCC_MEAS_DELAY  100
#define VCC_MEAS_SEQ    1
#define VCC_MEAS_SW     GPIO_PIN_6

static uint32_t vccMeasRaw[2] = {0,0};
static int_fast16_t delay = 0;

void GrabVccMeas()
{
    static uint32_t prevAdjMv = 0;

    GPIOPinWrite(GPIO_PORTB_BASE, VCC_MEAS_SW, VCC_MEAS_SW);
    DelayTicks(50);

    ADCProcessorTrigger(ADC0_BASE, VCC_MEAS_SEQ);
    while (ADCBusy(ADC0_BASE) == true){DelayTicks(5);}
    ADCSequenceDataGet(ADC0_BASE, VCC_MEAS_SEQ, vccMeasRaw);
    uint32_t unadjMv = (vccMeasRaw[0] * ADC_MAX_MV)/ADC_MAX_MEAS;
    uint32_t adjMv = unadjMv * MAGIC_VOLTDIV/MAGIC_TO_MV;

    if (adjMv != prevAdjMv)
    {
        UpdateVcc(adjMv);
        prevAdjMv = adjMv;
    }

    GPIOPinWrite(GPIO_PORTB_BASE, VCC_MEAS_SW, 0);
}

void SetupPowMon()
{
    delay = 0;

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    ADCSequenceConfigure(ADC0_BASE, VCC_MEAS_SEQ, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, VCC_MEAS_SEQ, 0, ADC_CTL_CH0 | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, VCC_MEAS_SEQ);

    GrabVccMeas();
}

void ProcessPowMon()
{
    if (delay >= VCC_MEAS_DELAY)
    {
        GrabVccMeas();
        delay = 0;
    }

    delay += CYCLE_MS;
}
