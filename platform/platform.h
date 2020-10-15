#ifndef PLATFORM_PLATFORM_H_
#define PLATFORM_PLATFORM_H_

#include <storage/storage.h>

#define A_SIDE (0)
#define B_SIDE (1)

typedef enum {
    NoMode = 0,
    LsbMode,
    UsbMode,
    CwMode,
    AmMode,
    FskMode,
    CwrMode,
    TuneMode,
    FsrMode
} platformModes;

void SetupPlatform();
void ProcessPlatform();

void TuneFreq(int8_t tune);
uint64_t GetFreq(uint8_t side);
void SetFreq(uint64_t freq, uint8_t side);
void SwapAB();
void SetMode(platformModes mode);
platformModes GetMode();

MemChannelData GetChannel(ChannelType type, uint32_t chan);
bool SetChannel(ChannelType type, uint32_t chan, MemChannelData *pData);

void UpdateVcc(int_fast16_t mv);

#endif
