/*
 * shift_register.h
 *
 *  Created on: Jul 1, 2021
 *      Author: Antonio Lanciotti
 */

#ifndef INC_SHIFT_REGISTER_H_
#define INC_SHIFT_REGISTER_H_

#include "stm32f3xx.h"
#include "main.h"

//NECESSARIO DEFINIRE DA CubeMX I PIN DELLO SHIFT REGISTER COME GPIO_OUTPUT CON LE SEGUENTI LABEL: SR_LATCH, SR_CLOCK, SR_DATA

#define SR_reset() SR_writeByte(0)

void SR_init();						//inizializza i tre pin a 0 e azzera lo shift register
void SR_writeByte(uint8_t data);	//invia un byte
void SR_writeOnes(uint8_t n);		//0<=n<=8, invia un byte della forma (8-n) zeri e n uni es. n=5 --> 00011111 = 2^5-1

#endif /* INC_SHIFT_REGISTER_H_ */
