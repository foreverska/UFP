#include "signal.h"
#include "string.h"
#include "stdbool.h"
#include "stdint.h"

#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"

#include "i2c_singleton.h"

sig_atomic_t setupDone = false;

void SetupI2c()
{
    if (setupDone == true)
    {
        return;
    }

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);
    I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), true);

    setupDone = true;
}

int WriteI2cMemAddr(uint32_t memAddr, uint32_t memAddrSize)
{
    uint8_t *pMemAddrByte = (uint8_t*) &memAddr;

    for (int i = 0; i < memAddrSize; i++)
    {
        I2CMasterDataPut(I2C0_BASE, pMemAddrByte[i]);
        if (i == 0)
        {
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_START);
        }
        else
        {
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
        }
        while (I2CMasterBusy(I2C0_BASE) == true) {};
        if (I2CMasterErr(I2C0_BASE) != I2C_MASTER_ERR_NONE)
        {
            return -1;
        }
    }

    return memAddrSize;
}

int WriteI2cBuffer(uint32_t i2cAddr, uint32_t memAddr, uint32_t memAddrSize,
                   uint8_t *pBuffer, size_t size)
{
    I2CMasterSlaveAddrSet(I2C0_BASE, i2cAddr, false);
    if (WriteI2cMemAddr(memAddr,memAddrSize) == -1)
    {
        return -1;
    }

    for (int i = 0; i < size; i++)
    {
        I2CMasterDataPut(I2C0_BASE, pBuffer[i]);
        if (size-1 != i)
        {
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_CONT);
        }
        else
        {
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_SEND_FINISH);
        }
        while (I2CMasterBusy(I2C0_BASE) == true) {};
        if (I2CMasterErr(I2C0_BASE) != I2C_MASTER_ERR_NONE)
        {
            return i;
        }
    }

    return size;
}

int ReadI2cBuffer(uint32_t i2cAddr, uint32_t memAddr, uint32_t memAddrSize,
                  uint8_t *pBuffer, size_t size)
{
    I2CMasterSlaveAddrSet(I2C0_BASE, i2cAddr, false);
    if (WriteI2cMemAddr(memAddr,memAddrSize) == -1)
    {
        return -1;
    }

    I2CMasterSlaveAddrSet(I2C0_BASE, i2cAddr, true);
    for (int i = 0; i < size; i++)
    {
        if (i == 0)
        {
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
        }
        if (size-1 != i)
        {
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        }
        else
        {
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
        }
        while (I2CMasterBusy(I2C0_BASE) == true) {};
        pBuffer[i] = I2CMasterDataGet(I2C0_BASE);
    }

    return size;
}

