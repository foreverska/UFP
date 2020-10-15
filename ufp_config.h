#ifndef UFP_CONFIG_H_
#define UFP_CONFIG_H_

#include <stdint.h>

#define CYCLE_MS    (40)
#define DEF_TUN_POW (10)
#define MIN_POW     (0)
#define MAX_POW     (5)
#define MAX_DIGIT   (8)
#define MIN_FREQ    (14000000)
#define MAX_FREQ    (14350000)

#define NUM_BANDS   2

typedef struct {
    char *pName;
    uint64_t minFreq;
    uint64_t maxFreq;
} bandEntry;

extern bandEntry radioBands[];

#endif
