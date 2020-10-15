#ifndef I2C_SINGLETON_H_
#define I2C_SINGLETON_H_

#include <stdint.h>
#include <string.h>

void SetupI2c();
int WriteI2cMemAddr(uint32_t memAddr, uint32_t memAddrSize);
int WriteI2cBuffer(uint32_t i2cAddr, uint32_t memAddr, uint32_t memAddrSize,
                   uint8_t *pBuffer, size_t size);
int ReadI2cBuffer(uint32_t i2cAddr, uint32_t memAddr, uint32_t memAddrSize,
                  uint8_t *pBuffer, size_t size);

#endif /* I2C_SINGLETON_H_ */
