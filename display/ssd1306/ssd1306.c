#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "display/display.h"
#include "font8x8_basic.h"
#include "platform/platform.h"
#include "ufp_config.h"
#include "i2c_singleton.h"

#define DEFAULT_ADDRESS (0x3C)

#define CHAR_LINE           16
#define INFO_LINE           0
#define MODE_LINE           3
#define FREQ_LINE           5
#define POW_LINE            6
#define STATUS_LINE         7

#define VOLT_OFFSET         96
#define MODE_OFFSET         8

#define DISP_NUMLINES       8
#define DISP_BYTESPERLINE   128

static uint8_t vmem[DISP_NUMLINES][DISP_BYTESPERLINE];
static uint64_t frequency = 0;
static uint8_t abSide = A_SIDE;
static uint8_t displayNeedsUpdate = 0xFF;
static int_fast16_t vccMv = 0;

enum dispOnOff {
    OFF = 0,
    ON
};


static void SendCommand(uint8_t addr, uint8_t cmd)
{
    WriteI2cBuffer(addr, 0x00, 1, &cmd, 1);
}

static void RefreshDisplay()
{
    SendCommand(DEFAULT_ADDRESS, 0x21); // SSD1306_COLUMNADDR
    SendCommand(DEFAULT_ADDRESS, 0);    // column start
    SendCommand(DEFAULT_ADDRESS, 127);  // column end
    SendCommand(DEFAULT_ADDRESS, 0x22); // SSD1306_PAGEADDR
    SendCommand(DEFAULT_ADDRESS, 0);    // page start
    SendCommand(DEFAULT_ADDRESS, 7);    // page end (8 pages for 64 rows OLED)

    WriteI2cBuffer(DEFAULT_ADDRESS, 0x40, 1, (uint8_t*) vmem,
                   DISP_NUMLINES * DISP_BYTESPERLINE);
}

static void RefreshLine(uint32_t line)
{
    SendCommand(DEFAULT_ADDRESS, 0x21); // SSD1306_COLUMNADDR
    SendCommand(DEFAULT_ADDRESS, 0);    // column start
    SendCommand(DEFAULT_ADDRESS, 127);  // column end
    SendCommand(DEFAULT_ADDRESS, 0x22); // SSD1306_PAGEADDR
    SendCommand(DEFAULT_ADDRESS, line);    // page start
    SendCommand(DEFAULT_ADDRESS, 7);    // page end (8 pages for 64 rows OLED)

    WriteI2cBuffer(DEFAULT_ADDRESS, 0x40, 1, vmem[line], DISP_BYTESPERLINE);
}

static void ClearVmem()
{
    memset(vmem, 0, sizeof(vmem));
}

static void WriteChar(uint8_t *dest, char *ch)
{
    for (int i = 0; i < 8; i++)
    {
        dest[i] = ch[0]>>i&0x1 | (ch[1]>>i&0x1)<<1 | (ch[2]>>i&0x1)<<2 |
                (ch[3]>>i&0x1)<<3 | (ch[4]>>i&0x1)<<4 | (ch[5]>>i&0x1)<<5 |
                (ch[6]>>i&0x1)<<6 | (ch[7]>>i&0x1)<<7;
    }
}

static void WriteStr(uint8_t *dest, char *str, size_t strlen)
{
    int index = 0;
    int offset = 0;
    while (str[index] != '\0' && index < strlen)
    {
        WriteChar(&dest[offset], (char *) &font8x8_basic[str[index]]);
        offset += 8;
        index++;
    }
}

static void SetPanelOnOff(enum dispOnOff val)
{
    if (val == ON)
    {
        SendCommand(DEFAULT_ADDRESS, 0xaf);
    }
    else
    {
        SendCommand(DEFAULT_ADDRESS, 0xae);
    }
}

void SetupPanel()
{
    SendCommand(DEFAULT_ADDRESS, 0xd5); // SSD1306_SETDISPLAYCLOCKDIV
    SendCommand(DEFAULT_ADDRESS, 0x80); // Suggested value 0x80
    SendCommand(DEFAULT_ADDRESS, 0xa8); // SSD1306_SETMULTIPLEX
    SendCommand(DEFAULT_ADDRESS, 0x3f); // 1/64
    SendCommand(DEFAULT_ADDRESS, 0xd3); // SSD1306_SETDISPLAYOFFSET
    SendCommand(DEFAULT_ADDRESS, 0x00); // 0 no offset
    SendCommand(DEFAULT_ADDRESS, 0x40); // SSD1306_SETSTARTLINE line #0
    SendCommand(DEFAULT_ADDRESS, 0x20); // SSD1306_MEMORYMODE
    SendCommand(DEFAULT_ADDRESS, 0x00); // 0x0 act like ks0108
    SendCommand(DEFAULT_ADDRESS, 0xa1); // SSD1306_SEGREMAP | 1
    SendCommand(DEFAULT_ADDRESS, 0xc8); // SSD1306_COMSCANDEC
    SendCommand(DEFAULT_ADDRESS, 0xda); // SSD1306_SETCOMPINS
    SendCommand(DEFAULT_ADDRESS, 0x12);
    SendCommand(DEFAULT_ADDRESS, 0x81); // SSD1306_SETCONTRAST
    SendCommand(DEFAULT_ADDRESS, 0xcf);
    SendCommand(DEFAULT_ADDRESS, 0xd9); // SSD1306_SETPRECHARGE
    SendCommand(DEFAULT_ADDRESS, 0xf1);
    SendCommand(DEFAULT_ADDRESS, 0xdb); // SSD1306_SETVCOMDETECT
    SendCommand(DEFAULT_ADDRESS, 0x30);
    SendCommand(DEFAULT_ADDRESS, 0x8d); // SSD1306_CHARGEPUMP
    SendCommand(DEFAULT_ADDRESS, 0x14); // Charge pump on
    SendCommand(DEFAULT_ADDRESS, 0x2e); // SSD1306_DEACTIVATE_SCROLL
    SendCommand(DEFAULT_ADDRESS, 0xa4); // SSD1306_DISPLAYALLON_RESUME
    SendCommand(DEFAULT_ADDRESS, 0xa6); // SSD1306_NORMALDISPLAY
}

void SetupDisplay()
{
    SetupI2c();

    SetPanelOnOff(OFF);
    SetupPanel();

    ClearVmem();
    RefreshDisplay();

    SetPanelOnOff(ON);
}

void ZeroPaddedString(int_fast16_t val, char *pBuf, size_t positions)
{
    for (int i = 1, pow = 1; i <= positions; i++)
    {
        pBuf[positions-i] = ((val/pow)%10) + '0';
        pow *= 10;
    }
}

void FreqToString(uint64_t freq, char *pBuf)
{
    int_fast16_t mhz = frequency/1000000;
    int_fast16_t khz = frequency/1000%1000;
    int_fast16_t hz = frequency/10%100;

    ZeroPaddedString(mhz, &pBuf[0], 3);
    pBuf[3] = '.';
    ZeroPaddedString(khz, &pBuf[4], 3);
    pBuf[7] = '.';
    ZeroPaddedString(hz, &pBuf[8], 2);
}

void ProcessDisplay()
{
    char line[16];

    if (displayNeedsUpdate != 0)
    {
        int_fast16_t len;
        int_fast16_t offset;

        ClearVmem();

        if ((displayNeedsUpdate & 1<<INFO_LINE) != 0)
        {
            line[0] = vccMv/1000%10 + '0';
            line[1] = '.';
            line[2] = vccMv/100%10 + '0';
            line[3] = 'v';
            WriteStr((uint8_t*) &vmem[INFO_LINE][VOLT_OFFSET], line, CHAR_LINE);
            RefreshLine(INFO_LINE);
        }

        if ((displayNeedsUpdate & 1<<MODE_LINE) != 0)
        {
            WriteStr((uint8_t*) &vmem[MODE_LINE][MODE_OFFSET],
                     "Frequency Mode", CHAR_LINE);
            RefreshLine(MODE_LINE);
        }

        if ((displayNeedsUpdate & 1<<FREQ_LINE) != 0)
        {
            len = 10;
            FreqToString(frequency, line);
            offset = ((16-len)/2)*8;
            WriteStr((uint8_t*) &vmem[FREQ_LINE][offset], line, CHAR_LINE);
            RefreshLine(FREQ_LINE);
        }

        if ((displayNeedsUpdate & 1<<STATUS_LINE) != 0)
        {
            if (abSide == A_SIDE)
            {
                WriteStr((uint8_t*) &vmem[STATUS_LINE], "A", 2);
            }
            else
            {
                WriteStr((uint8_t*) &vmem[STATUS_LINE], "B", 2);
            }
            RefreshLine(STATUS_LINE);
        }

        displayNeedsUpdate = 0;
    }
}

void UpdateDisplayPow(uint8_t pow)
{
    displayNeedsUpdate |= 1<<POW_LINE;
}

void UpdateDisplayFreq(uint64_t freqHz)
{
    frequency = freqHz;
    displayNeedsUpdate |= 1<<FREQ_LINE;
}

void UpdateDisplayAB(uint8_t side)
{
    abSide = side;
    displayNeedsUpdate |= 1<<STATUS_LINE;
}

void UpdateDispalyVcc(int_fast16_t mv)
{
    if ((vccMv - (vccMv%100)) != (mv - (mv%100)))
    {
        vccMv = mv;
        displayNeedsUpdate |= 1<<INFO_LINE;
    }
}
