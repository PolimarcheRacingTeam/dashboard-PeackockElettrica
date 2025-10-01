/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */
#include "can.h"

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SR_LATCH_Pin|SR_CLOCK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : r2dButton_Pin resetButton_Pin */
  GPIO_InitStruct.Pin = r2dButton_Pin|resetButton_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SR_DATA_Pin */
  GPIO_InitStruct.Pin = SR_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SR_DATA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SR_LATCH_Pin SR_CLOCK_Pin */
  GPIO_InitStruct.Pin = SR_LATCH_Pin|SR_CLOCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 2 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if (flagStartingOK == 1){
		/*		DEBUGGING
		if (GPIO_Pin == B1_Pin){
			HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
			HAL_CAN_AddTxMessage(&hcan1, &r2dTxHeader, &r2dData, &TxMailbox);

		}
		*/
		switch (GPIO_Pin){
			/*
			case B1_Pin:
				HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
				//DEBUG
				if(r2dData && vars[0].currentValue >200){
					newData++;
				}
				*/
			case r2dButton_Pin:
				if(flagsUsable[0]==1){
					if (HAL_GPIO_ReadPin(r2dButton_GPIO_Port, r2dButton_Pin) == GPIO_PIN_RESET){
						if(freniData && !r2dData){
							//HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
							r2dData = 1;
						} else if(freniData && r2dData){
							r2dData = 0;
						}
						vars[7].flag = 1;
						HAL_CAN_AddTxMessage(&hcan, &r2dTxHeader, &r2dData, &TxMailbox);
						flagsUsable[0] = 0;
						millisFlagsInterrupt[0] = HAL_GetTick();
					}
				}
				break;
			case resetButton_Pin:
				if (HAL_GPIO_ReadPin(resetButton_GPIO_Port, resetButton_Pin) == GPIO_PIN_RESET){
					system_reset();
				}
				break;
			}
	}
}
/* USER CODE END 2 */
