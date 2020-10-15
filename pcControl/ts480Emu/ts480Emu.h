#ifndef PCCONTROL_TS480EMU_TS480EMU_H_
#define PCCONTROL_TS480EMU_TS480EMU_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define PROTOCOL_BUF_SIZE   100
extern uint8_t protocolBuffer[];
extern size_t protocolWriteIndex;
extern bool protocolNewData;

void SetupTS480EmuSerial();
void ProcessTS480EmuSerial();

void SendTS480EmuSerial(uint8_t *pBuffer, size_t len);

#endif
