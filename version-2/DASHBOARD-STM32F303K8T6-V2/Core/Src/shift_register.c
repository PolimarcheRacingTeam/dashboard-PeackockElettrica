
#include "shift_register.h"
#include "can.h"


#define BIT(x,n) (((x) >> (n)) & 1)

// Funzione per leggere il registro shift
uint8_t readShiftRegister() {
    uint8_t data = 0;

    // Carica i dati paralleli nei registri interni
    HAL_GPIO_WritePin(SR_LATCH_GPIO_Port, SR_LATCH_Pin, GPIO_PIN_RESET);
    HAL_Delay(1); // breve delay per sincronizzazione
    HAL_GPIO_WritePin(SR_LATCH_GPIO_Port, SR_LATCH_Pin, GPIO_PIN_SET);

    // Leggi i dati seriali
    for (int i = 0; i < 8; i++) {
        data <<= 1; // Sposta il bit corrente

        HAL_GPIO_WritePin(SR_CLOCK_GPIO_Port, SR_CLOCK_Pin, GPIO_PIN_SET);
        if (HAL_GPIO_ReadPin(SR_DATA_GPIO_Port, SR_DATA_Pin) == GPIO_PIN_SET) {	//uguale a 1
            data |= 1; // Imposta il bit se il pin è HIGH
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

//controlla il valore della mappa impostato sul "manettino" e se è diversa lo invia via can e mostra il popup sul display
int checkMapValue(void){
	//uint8_t newData = determineSituation(readShiftRegister());
	//uint16_t newData = *arrayData[8];
	if (!flagNewMap && vars[6].currentValue!=newData){				//no pop up in corso
		if(newData==-1){
			//errore nella lettura
			flagErroreInCorso=1;
			errorValue = 131;
			NEXTION_SendString("ErrorValue.txt",0, 11);
			ultimoErroreRicevuto = HAL_GetTick();
			HAL_TIM_Base_Start_IT(&htim2);
		}
		else{
			vars[6].currentValue = newData;
			vars[6].flag = 1;
			HAL_CAN_AddTxMessage(&hcan, &mapTxHeader, &mapData, &TxMailbox);
			return 1;
		}
	}
	return 0;
}






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
