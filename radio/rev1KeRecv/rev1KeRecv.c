#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/i2c.h"

#include "utils/delay.h"
#include "platform/platform.h"
#include "../radio.h"
#include "i2c_singleton.h"

#define MHZ             (1000000.0)

#define SI514_ADDR      (0x55)
#define REG_LP          (0)
#define REG_MFRAC_0     (5)
#define REG_MFRAC_8     (6)
#define REG_MFRAC_16    (7)
#define REG_MINTFRAC    (8)
#define REG_MINT_3      (9)
#define REG_HSDIV_0     (10)
#define REG_LS0_HS8     (11)
#define REG_OE_STATE    (14)
#define REG_RST         (128)
#define REG_OE_FCAL     (132)


//OE_FCAL
#define DISABLE_OUT     (0x00)
#define ENABLE_OUT      (0x04)
#define ENABLE_FCAL     (0x01)

#define MFRAC_CONST     (0x20000000)

#define LPTABLE_SIZE    (5)

static double lpTable[5] = {
    65.040650407,
    65.259980246,
    67.859763463,
    72.937624981,
    75.843265046
};

#define LP1     (0)
#define LP2     (1)

static uint8_t lpVals[5][2] = {
   {2, 2},
   {2, 3},
   {3, 3},
   {3, 4},
   {4, 4}
};

typedef struct {
    uint16_t hsDiv;
    uint8_t lsDiv;
    uint16_t mInt;
    uint32_t mFrac;
    uint8_t lp;
} freqSettings;

static void WriteRegister(uint8_t reg, uint8_t val)
{
    WriteI2cBuffer(SI514_ADDR, reg, 1, &val, 1);
}

static void ReadRegister(uint8_t reg, uint32_t *pValue)
{
    ReadI2cBuffer(SI514_ADDR, reg, 1, (uint8_t*) pValue, 1);
}

static void CalcFreqSettings(uint64_t freq, freqSettings *pSettings)
{
    double freqMhz = freq/MHZ;
    double frac;

    frac = 2080/(freqMhz * 1022);
    if (frac < 1)
    {
        pSettings->lsDiv = 1;
    }
    else
    {
        while(1){}
    }

    frac = 2080/(freqMhz * pSettings->lsDiv);
    pSettings->hsDiv = ceil(frac);
    pSettings->hsDiv += pSettings->hsDiv%2;

    frac = pSettings->lsDiv * pSettings->hsDiv * (freqMhz/31.98);
    pSettings->mInt = floor(frac);
    pSettings->mFrac = (frac - pSettings->mInt) * MFRAC_CONST;

    for (int i = 0; i < LPTABLE_SIZE; i++)
    {
        if (lpTable[i] > frac)
        {
            pSettings->lp = lpVals[i-1][0] << 4 | lpVals[i-1][1];
            break;
        }
    }

    pSettings->lsDiv = pSettings->lsDiv - 1;
}

static void SetFreqSettings(freqSettings *pSettings)
{
    uint8_t out;
    WriteRegister(REG_LP, pSettings->lp);

    out = pSettings->mFrac & 0xFF;
    WriteRegister(REG_MFRAC_0, out);

    out = (pSettings->mFrac  >> 8) & 0xFF;
    WriteRegister(REG_MFRAC_8, out);

    out = (pSettings->mFrac >> 16) & 0xFF;
    WriteRegister(REG_MFRAC_16, out);

    out = ((pSettings->mFrac >> 24) & 0x1F) | ((pSettings->mInt & 0x7) << 5);
    WriteRegister(REG_MINTFRAC, out);

    out = (pSettings->mInt  >> 3) & 0x3F;
    WriteRegister(REG_MINT_3, out);

    out = (pSettings->hsDiv & 0xFF);
    WriteRegister(REG_HSDIV_0, out);

    out = ((pSettings->lsDiv & 0x07) << 4) | ((pSettings->hsDiv >>8) & 0x3);
    WriteRegister(REG_LS0_HS8, out);
}

void WriteFrequency(uint64_t freq)
{
    freqSettings settings;
    freq = 4915200;

    CalcFreqSettings(freq, &settings);

    WriteRegister(REG_OE_FCAL, DISABLE_OUT);

    SetFreqSettings(&settings);

    WriteRegister(REG_OE_FCAL, ENABLE_FCAL);
    WriteRegister(REG_OE_FCAL, ENABLE_OUT);
}

void SetupRadio()
{
    WriteFrequency(GetFreq(A_SIDE));
}

void TuneRadio(uint64_t frequency)
{
    WriteFrequency(GetFreq(A_SIDE));
}

void ProcessRadio()
{

}
