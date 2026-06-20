#ifndef SHIFT_REGISTER_H_
#define SHIFT_REGISTER_H_

#include "main.h"

// Definisci qui i parametri del tuo debouncing
#define SR_DEBOUNCE_SAMPLES 5
#define SR_READ_INTERVAL_MS 10 // Il task gira a 20ms, quindi questo può stare a 0 o 10

typedef struct {
    uint8_t currentValue;
    uint8_t lastRawValue;
    uint8_t debounceCounter;
    uint32_t lastReadTime;
    uint8_t isValid;
} SR_State_t;

void SR_Init(void);
void SR_ProcessPeriodic(void);
uint8_t SR_HasChanged(void);
int8_t SR_GetStablePosition(void);

#endif /* SHIFT_REGISTER_H_ */