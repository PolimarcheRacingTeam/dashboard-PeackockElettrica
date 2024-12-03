/*
 * shift_register.c
 *
 *  Created on: Jul 1, 2021
 *      Author: Antonio Lanciotti
 */


#include "shift_register.h"

#define BIT(x,n) (((x) >> (n)) & 1)

void SR_CLOCK_Pulse(){
	SR_CLOCK_GPIO_Port->BSRR = (uint32_t)SR_CLOCK_Pin;
	SR_CLOCK_GPIO_Port->BRR = (uint32_t)SR_CLOCK_Pin;
}

void SR_LATCH_Pulse(){
	SR_LATCH_GPIO_Port->BSRR = (uint32_t) SR_LATCH_Pin;
	SR_LATCH_GPIO_Port->BRR = (uint32_t) SR_LATCH_Pin;
}


void SR_init(){
	//setta a zero i pin data, clock e latch (sono 0 di default da GPIO ma per sicurezza)
	HAL_GPIO_WritePin(SR_CLOCK_GPIO_Port, SR_CLOCK_Pin, 0);
	HAL_GPIO_WritePin(SR_LATCH_GPIO_Port, SR_LATCH_Pin, 0);
	HAL_GPIO_WritePin(SR_DATA_GPIO_Port, SR_DATA_Pin, 0);
	SR_writeOnes(0);
}

void SR_writeByte(uint8_t data){
	//bit piu significativo va fuori per primo
	for (int i=7; i>=0; i--){
		HAL_GPIO_WritePin(SR_DATA_GPIO_Port, SR_DATA_Pin, BIT(data,i));
		SR_CLOCK_Pulse();
	}
	SR_LATCH_Pulse();
	SR_DATA_GPIO_Port->BRR = SR_DATA_Pin;
}

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
