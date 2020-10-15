#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/i2c.h"

#include "utils/delay.h"
#include "../storage.h"
#include "i2c_singleton.h"

#define ADDR_STORAGE    (0x50)

#define MAXCHAN         (100)
#define RXCHAN_START    (0x0)
#define TXCHAN_START    (RXCHAN_START + MAXCHAN * MEMCHAN_SIZE)
#define MEMCHAN_SIZE    (32)

static void ReadBytes(uint16_t addr, uint8_t *pData, size_t size)
{
    while(ReadI2cBuffer(ADDR_STORAGE, addr, 2, pData, size) == -1){}
}

static bool VerifyBytes(uint16_t addr, uint8_t *pData, size_t size)
{
    uint8_t curByte;

    while (WriteI2cMemAddr(addr,2) == -1) {}

    I2CMasterSlaveAddrSet(I2C0_BASE, ADDR_STORAGE, true);

    for (int i = 0; i < size; i++)
    {
        if (i == 0)
        {
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);
        }
        else
        {
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        }
        while (I2CMasterBusy(I2C0_BASE) == true) {};
        curByte = I2CMasterDataGet(I2C0_BASE);

        if (curByte != pData[i])
        {
            I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
            while (I2CMasterBusy(I2C0_BASE) == true) {};
            return false;
        }
    }

    I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    while (I2CMasterBusy(I2C0_BASE) == true) {};
    return true;
}

static void WriteBytes(uint16_t addr, uint8_t *pData, size_t size)
{
    WriteI2cBuffer(ADDR_STORAGE, addr, 2, pData, size);
}

static bool VerifiedWrite(uint16_t addr, uint8_t *pData, size_t size)
{
    WriteBytes(addr, pData, size);
    if (VerifyBytes(addr, pData, size) == false)
    {
        return false;
    }

    return true;
}

void SetupStorage()
{
    SetupI2c();
}

void ProcessStorage()
{

}

MemChannelData GetMemoryChannel(ChannelType type, uint8_t channel)
{
    MemChannelData readChannel = {};
    uint32_t offset = RXCHAN_START;
    if (type == TxChannel)
    {
        offset = TXCHAN_START;
    }

    uint32_t chanStart = channel * MEMCHAN_SIZE + offset;
    ReadBytes(chanStart, (uint8_t*) &readChannel, sizeof(MemChannelData));

    return readChannel;
}

bool SetMemoryChannel(ChannelType type, uint8_t channel, MemChannelData *pData)
{
    uint32_t offset = RXCHAN_START;
    if (type == TxChannel)
    {
        offset = TXCHAN_START;
    }

    uint32_t chanStart = channel * MEMCHAN_SIZE + offset;
    return VerifiedWrite(chanStart, (uint8_t*) pData, sizeof(MemChannelData));
}
