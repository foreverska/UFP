#ifndef DISPLAY_DISPLAY_H_
#define DISPLAY_DISPLAY_H_

void SetupDisplay();
void ProcessDisplay();

void UpdateDisplayPow(uint8_t pow);
void UpdateDisplayFreq(uint64_t freqHz);
void UpdateDisplayAB(uint8_t side);
void UpdateDispalyVcc(int_fast16_t mv);

#endif
