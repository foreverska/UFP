#include <stdint.h>
#include <stdbool.h>

#include "platform.h"
#include "display/display.h"
#include "radio/radio.h"
#include "ufp_config.h"

static uint8_t abSide;
static uint64_t freqSides[2];
static platformModes curMode;

void SetupPlatform()
{
    freqSides[A_SIDE] = radioBands[0].minFreq;
    freqSides[B_SIDE] = radioBands[0].minFreq;
    abSide = A_SIDE;
    UpdateDisplayAB(abSide);
    UpdateDisplayFreq(freqSides[abSide]);
    curMode = UsbMode;
}

void TuneFreq(int8_t tune)
{
    int64_t tuneAmt = DEF_TUN_POW * tune;
    int64_t newFreq = freqSides[abSide] + tuneAmt;
    if (newFreq < 0)
    {
        newFreq = 0;
    }
    SetFreq(newFreq, abSide);
}

void SwapAB()
{
    if (abSide == A_SIDE)
    {
        abSide = B_SIDE;
    }
    else
    {
        abSide = A_SIDE;
    }

    UpdateDisplayAB(abSide);
    UpdateDisplayFreq(freqSides[abSide]);
}

bool isFreqInBand(uint64_t freq)
{
    for (int i = 0; i < NUM_BANDS; i++)
    {
        if (freq >= radioBands[i].minFreq && freq <= radioBands[i].maxFreq)
        {
            return true;
        }
    }

    return false;
}

void SetFreq(uint64_t freq, uint8_t side)
{
    freq = freq - (freq %10);

    if (isFreqInBand(freq) == false)
    {
        return;
    }

    freqSides[side] = freq;
    UpdateDisplayFreq(freq);
    TuneRadio(freq);
}

uint64_t GetFreq(uint8_t side)
{
    return freqSides[side];
}

void SetMode(platformModes mode)
{
    curMode = mode;
}

platformModes GetMode()
{
    return curMode;
}

MemChannelData GetChannel(ChannelType type, uint32_t chan)
{
    return GetMemoryChannel(type, chan);
}

bool SetChannel(ChannelType type, uint32_t chan, MemChannelData *pData)
{
    return SetMemoryChannel(type, chan, pData);
}

void UpdateVcc(int_fast16_t mv)
{
    UpdateDispalyVcc(mv);
}

void ProcessPlatform()
{

}
