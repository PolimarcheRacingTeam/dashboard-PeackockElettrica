#ifndef INC_SHIFT_REGISTER_H_
#define INC_SHIFT_REGISTER_H_

#include "stm32f4xx.h"
#include "main.h"
#include "tim.h"

#define SR_reset() SR_writeByte(0)

void SR_init();						//inizializza i tre pin a 0 e azzera lo shift register
//void SR_writeByte(uint8_t data);	//invia un byte
//void SR_writeOnes(uint8_t n);		//0<=n<=8, invia un byte della forma (8-n) zeri e n uni es. n=5 --> 00011111 = 2^5-1

#endif /* INC_SHIFT_REGISTER_H_ */
