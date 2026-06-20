/*
 * Scheduler.c
 *
 *  Created on: 19 giu 2026
 *      Author: zinga
 */

#include "Scheduler.h"
#include "Tasks.h" // File dove dichiarerai Task_ReadInputs(), Task_SendCAN(), ecc.

volatile uint32_t tickCounter = 0;

volatile uint8_t flag_task_read_inputs = 0;
volatile uint8_t flag_task_send_can = 0;
volatile uint8_t flag_task_update_disp = 0;

void SchedulerInit(void){
	// SysTick configurato a 1 millisecondo (HCLK / 1000)
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	tickCounter = 0;
	flag_task_read_inputs = 0;
	flag_task_send_can = 0;
	flag_task_update_disp = 0;

	TaskInit();
}

void SchedulerTimerInterruptCallBack(void){
	tickCounter++;

	// Valuta le flag in base agli intervalli definiti nell'header
	if(tickCounter % INTERVAL_READ_INPUTS == 0){
		flag_task_read_inputs = 1;
	}
	if(tickCounter % INTERVAL_SEND_CAN == 0){
		flag_task_send_can = 1;
	}
	if(tickCounter % INTERVAL_UPDATE_DISP == 0){
		flag_task_update_disp = 1;
	}

	// Reset del contatore:
	if(tickCounter >= 1000){
		tickCounter = 0;
	}
}

void SchedulerManagementFunction(void){

	// 1. Task di Acquisizione
	if(flag_task_read_inputs){
		flag_task_read_inputs = 0;
		Task_ReadInputs();
	}

	// 2. Task di Trasmissione
	if(flag_task_send_can){
		flag_task_send_can = 0;
		Task_SendCAN();
	}

	// 3. Task di Interfaccia
	if(flag_task_update_disp){
		flag_task_update_disp = 0;
		Task_UpdateDisplay();
	}

}
