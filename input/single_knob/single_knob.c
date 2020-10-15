#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/qei.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"

#include "../input.h"
#include "platform/platform.h"
#include "utils/uartstdio.h"
#include "ufp_config.h"

#define LONG_PRESS_TIME 1000
#define DEFAULT_QEI_POS 500
#define SINGLE_NOTCH    4
#define DEFAULT_POW_DIR -1

#define SIZE_VEL_TABLE  3
uint32_t velTable[SIZE_VEL_TABLE][2] = {{120, 20},
                                        {60, 10},
                                        {0, 1}};


enum STATE {
    PRESSED,
    LONGPRESS,
    UNPRESSED
};

static enum STATE curState;
uint32_t timePressed;

void SetupInput()
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI0);

    QEIDisable(QEI0_BASE);
    QEIConfigure(QEI0_BASE,
                 QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_QUADRATURE | QEI_CONFIG_SWAP | QEI_CONFIG_NO_RESET,
                65535);
    QEIFilterEnable(QEI0_BASE);
    QEIFilterConfigure(QEI0_BASE, QEI_FILTCNT_10);
    QEIVelocityConfigure(QEI0_BASE, QEI_VELDIV_1,  SysCtlClockGet());
    QEIVelocityEnable(QEI0_BASE);
    QEIEnable(QEI0_BASE);
    QEIPositionSet(QEI0_BASE, DEFAULT_QEI_POS);

    curState = UNPRESSED;
    timePressed = 0;
}

static void ProcessBtn()
{
    int32_t pins = GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7);
    bool pressed = (pins == GPIO_PIN_7);
    if (pressed == true)
    {
        if (curState != LONGPRESS)
        {
            curState = PRESSED;
            timePressed += CYCLE_MS;
            if (timePressed > LONG_PRESS_TIME)
            {
                timePressed = LONG_PRESS_TIME;
                curState = LONGPRESS;

                SwapAB();
            }
        }
    }
    else
    {
        if (curState == PRESSED && timePressed < 1000)
        {
            //ChangePower(DEFAULT_POW_DIR);
        }
        timePressed = 0;
        curState = UNPRESSED;
    }
}

static uint32_t getVelPower(uint32_t curVel)
{
    for (int i = 0; i < SIZE_VEL_TABLE; i++)
    {
        if (curVel >= velTable[i][0])
        {
            return velTable[i][1];
        }
    }

    return 1;
}

static void ProcessQei()
{
    uint32_t curPosition = QEIPositionGet(QEI0_BASE);
    uint32_t curVel = QEIVelocityGet(QEI0_BASE);
    uint32_t velPow = getVelPower(curVel);
    if (curPosition != DEFAULT_QEI_POS)
    {
        uint32_t positionDelta = (curPosition - DEFAULT_QEI_POS);
        if (positionDelta >= SINGLE_NOTCH)
        {
            TuneFreq(positionDelta/SINGLE_NOTCH * velPow);
            QEIPositionSet(QEI0_BASE, DEFAULT_QEI_POS + positionDelta%SINGLE_NOTCH);
        }
    }
}

void ProcessInput()
{
    ProcessBtn();
    ProcessQei();
}
