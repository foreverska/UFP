#include "ufp_config.h"

#if NUM_BANDS == 0
#error RADIO MUST HAVE AT LEAST ONE BAND
#endif

bandEntry radioBands[NUM_BANDS] = {
    {"20M", 14000000, 14350000},
    {"40M",  7000000,  7300000}
};
