/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include <string.h>
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

}

/* USER CODE BEGIN 2 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if (flagOK==1){
		switch (GPIO_Pin){
			case r2dButton_Pin:
				if(flagsUsable[0]==1){
					if (HAL_GPIO_ReadPin(r2dButton_GPIO_Port, r2dButton_Pin) == GPIO_PIN_RESET){
						if(freniData && !r2dData){
							r2dData = 1;
							flags[7] = 1;
						} else if(freniData && r2dData){
							r2dData = 0;
							flags[7] = 1;
						}
						HAL_CAN_AddTxMessage(&hcan, &r2dTxHeader, &r2dData, &TxMailbox);
						flagsUsable[0] = 0;
						millisFlagsInterrupt[0] = HAL_GetTick();
					}
				}
				break;
			case resetButton_Pin:
				if (HAL_GPIO_ReadPin(resetButton_GPIO_Port, resetButton_Pin) == GPIO_PIN_RESET){
					system_reset();
					//newData++;
				}
				break;
			}
	}
}

	/*
void cambiaPagina(){
	if (pageRefreshata){
		pageRefreshata=0;
	if(currentPageDisplay == 0){
		currentPageDisplay = 1;
	} else if (currentPageDisplay == 1){
		currentPageDisplay = 0;
	}
	for(int i = 1;i<Ndata;i++){
		if (active[i]==1){
			active[1] = 0;
		} else if (active[i]==0){
			active[i] = 1;
		}
		if (active[1]==1){
			flags[i] = 1;
		}
	}
	flags[10]=1;
	flags[0] = 1;
	}
}*/

/* USER CODE END 2 */
