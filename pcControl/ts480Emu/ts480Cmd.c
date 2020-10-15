#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <pcControl/ts480Emu/ts480Cmd.h>
#include <pcControl/ts480Emu/ts480Emu.h>
#include <platform/platform.h>

#define FREQ_LEN    (11)
#define IF_READ_LEN (38)
#define VFO_SETLEN (FREQ_LEN + 3)
#define MD_RESP_LEN (4)
#define MR_RESP_LEN (50)

#define FA_CMD  (0x4641)
#define FB_CMD  (0x4642)
#define ID_CMD  (0x4944)
#define IF_CMD  (0x4946)
#define MD_CMD  (0x4D44)
#define MR_CMD  (0x4D52)
#define MW_CMD  (0x4D57)

#define READ_CMD            (len == 3 && pCmd[2] == ';')

static void SendSyntaxErr()
{
    uint8_t *pError = (uint8_t*) "?;";
    SendTS480EmuSerial(pError, 2);
}

static uint64_t StringToInt(uint8_t *pBuffer, size_t size)
{
    uint64_t num = 0;
    for (int i = 0; i < size; i++)
    {
        num *= 10;
        if (pBuffer[i] < '0' || pBuffer[i] > '9')
        {
            return (uint64_t) -1;
        }

        num += pBuffer[i] - '0';
    }

    return num;
}

static void IntToString(uint64_t val, uint8_t *pBuffer, size_t maxDigits)
{
    for (int i = maxDigits-1; i >= 0; i--)
    {
        pBuffer[i] = val%10 + '0';
        val = val/10;
    }
}

static uint64_t StringToFreq(uint8_t *pBuffer)
{
    return StringToInt(pBuffer, FREQ_LEN);
}

static void FreqToString(uint8_t side, uint8_t *pBuffer)
{
    uint64_t freq = GetFreq(side);
    IntToString(freq, pBuffer, FREQ_LEN);
}

static void vfoFreq(uint8_t *pCmd, size_t len)
{
    uint8_t side = A_SIDE;

    if (pCmd[1] == 'B')
    {
        side = B_SIDE;
    }

    if (READ_CMD)
    {
        uint8_t resp[VFO_SETLEN];

        resp[0] = 'F';
        resp[1] = pCmd[1];
        FreqToString(side, &resp[2]);
        resp[13] = ';';
        SendTS480EmuSerial(resp, FREQ_LEN + 3);
    }
    else
    {
        if (len != VFO_SETLEN)
        {
            SendSyntaxErr();
            return;
        }

        uint64_t freq = StringToFreq(&pCmd[2]);
        if (freq == (uint64_t) -1)
        {
            return;
        }

        SetFreq(freq, side);
    }
}

static void ifCmd(uint8_t *pCmd, size_t len)
{
    if (READ_CMD)
    {
        uint8_t resp[IF_READ_LEN];
        memset(resp, '0', IF_READ_LEN);
        resp[0] = 'I';
        resp[1] = 'F';
        FreqToString(A_SIDE, &resp[2]);
        resp[IF_READ_LEN-1] = ';';
        SendTS480EmuSerial(resp, IF_READ_LEN);
    }
    else
    {
        SendSyntaxErr();
    }
}

static void mdCmd(uint8_t *pCmd, size_t len)
{
    if (READ_CMD)
    {
        uint8_t resp[MD_RESP_LEN];
        resp[0] = 'M';
        resp[1] = 'D';
        resp[2] = GetMode() + '0';
        resp[3] = ';';
        SendTS480EmuSerial(resp, MD_RESP_LEN);
    }
    else
    {
        if (len != MD_RESP_LEN)
        {
            SendSyntaxErr();
        }
        platformModes newMode = (platformModes) (pCmd[2] - '0');
        SetMode(newMode);
    }
}

static void mrCmd(uint8_t *pCmd, size_t len)
{
    if (len == 7 && pCmd[6] == ';')
    {
        uint8_t resp[MR_RESP_LEN];
        ChannelType chanType = RxChannel;
        if (pCmd[2] == '1')
        {
            chanType = TxChannel;
        }
        int64_t chanNum = StringToInt(&pCmd[4], 2);
        if (chanNum == -1)
        {
            SendSyntaxErr();
            return;
        }

        MemChannelData readChannel = GetChannel(chanType, chanNum);
        resp[0] = 'M';
        resp[1] = 'R';
        resp[2] = '0';
        if (readChannel.type == TxChannel)
        {
            resp[2] = '1';
        }
        resp[3] = '0';
        IntToString(chanNum, &resp[4], 2);
        IntToString(readChannel.frequency, &resp[6], FREQ_LEN);
        memset(&resp[17], '0', 24);
        memcpy(&resp[41], readChannel.name, CHANNEL_NAME_SIZE);
        resp[49] = ';';
        SendTS480EmuSerial(resp, MR_RESP_LEN);
    }
    else
    {
        SendSyntaxErr();
    }
}

static void mwCmd(uint8_t *pCmd, size_t len)
{
    if (READ_CMD)
    {
        SendSyntaxErr();
    }
    else
    {
        MemChannelData newChannel;
        if (len != 50)
        {
            SendSyntaxErr();
            return;
        }
        int64_t chanNum = StringToInt(&pCmd[4], 2);
        if (chanNum == -1)
        {
            SendSyntaxErr();
            return;
        }

        newChannel.type = RxChannel;
        if (pCmd[2] == '1')
        {
            newChannel.type = TxChannel;
        }
        newChannel.frequency = StringToInt(&pCmd[7], FREQ_LEN);
        memcpy(newChannel.name, &pCmd[41], CHANNEL_NAME_SIZE);

        if (SetChannel(newChannel.type, chanNum, &newChannel) == false)
        {
            SendTS480EmuSerial("O;", 2);
        }
    }
}

void ProcessCommand(uint8_t *pCmd, size_t len)
{
    uint16_t cmd = pCmd[0]<<8 | pCmd[1];

    switch(cmd)
    {
    case FA_CMD:
    case FB_CMD:
        vfoFreq(pCmd, len);
        return;
    case IF_CMD:
        ifCmd(pCmd, len);
        return;
    case MD_CMD:
        mdCmd(pCmd, len);
        return;
    case MR_CMD:
        mrCmd(pCmd, len);
        return;
    case MW_CMD:
        mwCmd(pCmd, len);
        return;
    case ID_CMD:
        SendTS480EmuSerial("ID020;", 6);
        return;
    default:
        SendSyntaxErr();
    }

    return;
}
