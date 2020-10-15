#ifndef STORAGE_STORAGE_H_
#define STORAGE_STORAGE_H_

#include <stdint.h>

#define CHANNEL_NAME_SIZE 8

typedef enum {
    RxChannel = 0,
    TxChannel
} ChannelType;

typedef struct {
    ChannelType type;
    uint64_t frequency;
    char name[CHANNEL_NAME_SIZE];
} MemChannelData;

void SetupStorage();
void ProcessStorage();

MemChannelData GetMemoryChannel(ChannelType type, uint8_t channel);
bool SetMemoryChannel(ChannelType type, uint8_t channel, MemChannelData *pData);

#endif /* STORAGE_STORAGE_H_ */
