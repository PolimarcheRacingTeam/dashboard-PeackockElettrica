
#include "shift_register.h"
#include "can.h"

#define BIT(x,n) (((x) >> (n)) & 1)

//globale nel file
static SR_State_t sr_state = {
    .currentValue = 0,
    .lastRawValue = 0xFF,
    .debounceCounter = 0,
    .lastReadTime = 0,
    .isValid = 0
};

// Inizializzazione
void SR_Init(void) {
    HAL_GPIO_WritePin(SR_CLOCK_GPIO_Port, SR_CLOCK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SR_LATCH_GPIO_Port, SR_LATCH_Pin, GPIO_PIN_SET);
    sr_state.currentValue = 0;
    sr_state.lastRawValue = 0xFF;
    sr_state.debounceCounter = 0;
    sr_state.lastReadTime = 0;
    sr_state.isValid = 0;
}

// Lettura grezza
uint8_t SR_ReadRaw(void) {
    uint8_t data = 0;
    
    // Pulse sul latch per caricare i dati paralleli (PL pin)
    HAL_GPIO_WritePin(SR_LATCH_GPIO_Port, SR_LATCH_Pin, GPIO_PIN_RESET);
    
    // Breve attesa senza bloccare
    for (volatile int i = 0; i < 72; i++) __NOP(); // circa ~1us -> 72MHz
    
    HAL_GPIO_WritePin(SR_LATCH_GPIO_Port, SR_LATCH_Pin, GPIO_PIN_SET);
    
    // Leggi i dati seriali 
    for (int i = 0; i < 8; i++) {
        data <<= 1;
        
        // Clock alto per leggere il bit
        HAL_GPIO_WritePin(SR_CLOCK_GPIO_Port, SR_CLOCK_Pin, GPIO_PIN_SET);
        
        if (HAL_GPIO_ReadPin(SR_DATA_GPIO_Port, SR_DATA_Pin) == GPIO_PIN_SET) {
            data |= 1;
        }
        
        HAL_GPIO_WritePin(SR_CLOCK_GPIO_Port, SR_CLOCK_Pin, GPIO_PIN_RESET);
    }
    
    return data;
}

int determineSituation(uint8_t data) {
    switch (data) {
        case 0b00000001: return 1; // Situazione 1
        case 0b00000010: return 2; // Situazione 2
        case 0b00000100: return 3; // Situazione 3
        case 0b00001000: return 4; // Situazione 4
        case 0b00010000: return 5; // Situazione 5
        case 0b00100000: return 6; // Situazione 6
        case 0b01000000: return 7; // Situazione 7
        case 0b10000000: return 8; // Situazione 8
        case 0b00000000: return 0;
        default: return -1; // Nessuna situazione valida
    }
}

// Conta i bit attivi 
static uint8_t countBits(uint8_t value) {
    uint8_t count = 0;
    while (value) {
        count += value & 1;
        value >>= 1;
    }
    return count;
}

// Byte -> Posizione
static int8_t byteToPosition(uint8_t data) {
    uint8_t bitCount = countBits(data);
    
    if (bitCount == 0) {
        return 0; 
    }

    //al max 1 per valore per posizione
    if (bitCount != 1) {
        return -1; 
    }
    
    // Trova quale bit è attivo
    for (int i = 0; i < 8; i++) {
        if (data & (1 << i)) {
            return i + 1; // Posizioni 1-8
        }
    }
    
    return -1; //errore
}

//Periodico per debouncing
void SR_ProcessPeriodic(void) {
    uint32_t currentTime = HAL_GetTick();
    
    if ((currentTime - sr_state.lastReadTime) < SR_READ_INTERVAL_MS) {
        return;
    }
    
    sr_state.lastReadTime = currentTime;
    
    uint8_t rawValue = SR_ReadRaw();
    
    if (rawValue == sr_state.lastRawValue) {
        if (sr_state.debounceCounter < SR_DEBOUNCE_SAMPLES) {
            sr_state.debounceCounter++;
        }
        
        if (sr_state.debounceCounter >= SR_DEBOUNCE_SAMPLES) {
            uint8_t newValue = rawValue;
            
            if (newValue != sr_state.currentValue) {
                sr_state.currentValue = newValue;
                sr_state.isValid = 1;
            }
        }
    } else {
        sr_state.lastRawValue = rawValue;
        sr_state.debounceCounter = 1;
    }
}

int8_t SR_GetStablePosition(void) {
    return byteToPosition(sr_state.currentValue);
}

uint8_t SR_HasChanged(void) {
    if (sr_state.isValid) {
        sr_state.isValid = 0; // Reset del flag
        return 1;
    }
    return 0;
}

int8_t SR_GetCurrentPosition(void) {
    return byteToPosition(sr_state.currentValue);
}


int checkMapValue(void){
    SR_ProcessPeriodic();
    
    if (SR_HasChanged()) {
        int8_t newPosition = SR_GetStablePosition();
        
        if (newPosition == -1) {
            flagErroreInCorso = 1;
			showError(131);
            ultimoErroreRicevuto = HAL_GetTick();
            HAL_TIM_Base_Start_IT(&htim2);
            return 0;
        }
        
        if (!flagNewMap && vars[6].currentValue != newPosition) {
            vars[6].currentValue = newPosition;
            vars[6].flag = 1;
            
            // Invia via CAN
            uint8_t mapData = (uint8_t)newPosition;
            HAL_CAN_AddTxMessage(&hcan, &mapTxHeader, &mapData, &TxMailbox);
            return 1;
        }
    }
    return 0;
}



/* ========================================================================
Vecchia implementazione 
 * ======================================================================== */

/*
void SR_CLOCK_Pulse(){
	SR_CLOCK_GPIO_Port->BSRR = (uint32_t)SR_CLOCK_Pin;
	SR_CLOCK_GPIO_Port->BRR = (uint32_t)SR_CLOCK_Pin;
}

void SR_LATCH_Pulse(){
	SR_LATCH_GPIO_Port->BSRR = (uint32_t) SR_LATCH_Pin;
	SR_LATCH_GPIO_Port->BRR = (uint32_t) SR_LATCH_Pin;
}


void SR_init(){
	HAL_GPIO_WritePin(SR_CLOCK_GPIO_Port, SR_CLOCK_Pin, 0);
	HAL_GPIO_WritePin(SR_LATCH_GPIO_Port, SR_LATCH_Pin, 0);
	HAL_GPIO_WritePin(SR_DATA_GPIO_Port, SR_DATA_Pin, 0);
}

void SR_writeByte(uint8_t data){
	//bit piu significativo va fuori per primo
	for (int i=7; i>=0; i--){
		HAL_GPIO_WritePin(SR_DATA_GPIO_Port, SR_DATA_Pin, BIT(data,i));
		SR_CLOCK_Pulse();
	}
	SR_LATCH_Pulse();
	SR_DATA_GPIO_Port->BRR = SR_DATA_Pin;
}*/
/*
void SR_writeOnes(uint8_t n){
	if (n>8) n=8;	//limita ad 8 il parametro
	int i;
	SR_DATA_GPIO_Port->BRR = SR_DATA_Pin;	//setta a zero il data
	for (i=0; i<(8-n); i++) SR_CLOCK_Pulse();	//8-n shift
	SR_DATA_GPIO_Port->BSRR = SR_DATA_Pin;	//setta a uno il data
	for (i=0; i<n; i++) SR_CLOCK_Pulse();	//n shift
	SR_LATCH_Pulse();
	SR_DATA_GPIO_Port->BRR = SR_DATA_Pin;	//resetta a zero data
}
*/
