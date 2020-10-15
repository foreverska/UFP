#include <pcControl/pcControl.h>
#include <pcControl/ts480Emu/ts480Emu.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "utils/uartstdio.h"
#include <platform/platform.h>
#include "ts480Cmd.h"

uint8_t protocolBuffer[PROTOCOL_BUF_SIZE];
size_t protocolWriteIndex;
bool protocolNewData;

void SetupPcControl()
{
    protocolWriteIndex = 0;
    protocolNewData = false;

    SetupTS480EmuSerial();
}

void ProcessPcControl()
{
    size_t commandStart = 0;
    size_t index = 0;

    ProcessTS480EmuSerial();

    if (protocolNewData == false)
    {
        return;
    }

    for (index = 0; index < protocolWriteIndex; index++)
    {
        if (protocolBuffer[index] == ';')
        {
            uint8_t commandLen = index - commandStart + 1;
            if (commandLen >= 3)
            {
                ProcessCommand(&protocolBuffer[commandStart], commandLen);
            }
            commandStart = index+1;
        }
    }

    size_t remainderSize = protocolWriteIndex - commandStart;
    if (remainderSize == PROTOCOL_BUF_SIZE/2)
    {
        commandStart = 0;
    }
    else if(commandStart >= PROTOCOL_BUF_SIZE/2)
    {
        commandStart = 0;
    }

    protocolWriteIndex = remainderSize;
    protocolNewData = false;
}
