#include <stdint.h>
#include <stdbool.h>

#include "driverlib/sysctl.h"

#define CycleToMs(x) (x / (SysCtlClockGet() / 3 / 1000))
#define MsToCycles(x) (x * (SysCtlClockGet() / 3 / 1000))


#define DelayMs(x) { \
    SysCtlDelay(x * (SysCtlClockGet() / 3 / 1000)); \
}

#define DelayUs(x) { \
    SysCtlDelay(x * (SysCtlClockGet() / 3/ 1000000)); \
}

#define DelayTicks(x) { \
    SysCtlDelay(x); \
}

