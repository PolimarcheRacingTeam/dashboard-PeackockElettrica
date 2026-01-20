#ifndef INC_SHIFT_REGISTER_H_
#define INC_SHIFT_REGISTER_H_

#include "stm32f3xx.h"
#include "main.h"
#include "tim.h"
#include <stdint.h>

#define SR_DEBOUNCE_SAMPLES     5      // Numero di letture consecutive uguali richieste
#define SR_READ_INTERVAL_MS     10     // Intervallo tra letture (ms)
#define SR_SETUP_TIME_US        1      // Tempo di setup del latch (microsecondi)
#define SR_ERROR_INVALID_STATE  255    // Valore di errore per stato invalido

typedef struct {
    uint8_t currentValue;              // Valore attualmente stabile
    uint8_t lastRawValue;              // Ultima lettura grezza
    uint8_t debounceCounter;           // Contatore per debouncing
    uint32_t lastReadTime;             // Timestamp ultima lettura
    uint8_t isValid;                   // Flag validità lettura
} SR_State_t;

void SR_Init(void);
uint8_t SR_ReadRaw(void);
int8_t SR_GetStablePosition(void);
void SR_ProcessPeriodic(void);
uint8_t SR_HasChanged(void);
int8_t SR_GetCurrentPosition(void);

#endif 
