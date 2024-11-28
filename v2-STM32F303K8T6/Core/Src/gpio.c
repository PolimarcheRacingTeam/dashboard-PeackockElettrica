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
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = r2dButton_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(r2dButton_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = nextPageButton_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(nextPageButton_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin */
  GPIO_InitStruct.Pin = button2_Pin|button3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 2 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	if(GPIO_Pin == r2dButton_Pin){
		if (HAL_GPIO_ReadPin(r2dButton_GPIO_Port, r2dButton_Pin) == GPIO_PIN_SET){
			flagU11=1;
			//pulsante premuto<<
			if(freniData && !r2dData){
				r2dData = 1;
				flags[10] = 1;
			} else if(freniData && r2dData){
				r2dData = 0;
				flags[10] = 1;
			}
		} else{
			//scrivere qua se si vuole tenere il r2d accessibile solo se mentre si preme il tasto
		}
		chiamata++;
		*arrayData[1]=chiamata;
		flags[1]=1;
	} else if (GPIO_Pin == nextPageButton_Pin){
		if (HAL_GPIO_ReadPin(nextPageButton_GPIO_Port, nextPageButton_Pin) == GPIO_PIN_SET){
			flagU12 = 1;
			if(currentPageDisplay == 0){
				currentPageDisplay = 1;
			} else if (currentPageDisplay == 1){
				currentPageDisplay = 0;
			}
			for(int i = 1;i<Ndata;i++){
				active[i] = !active[i];
				if (active[1]==1){
					flags[i] = 1;
				}
			}
			flags[0] = 1;
			}
		chiamata++;
		*arrayData[1]=104;
		flags[1]=1;
		//comando per visualizzare pagina successiva
	} else if (GPIO_Pin == button2_Pin){
		//gestione pulsante 2
	} else if (GPIO_Pin == button3_Pin){
		//gestione pulsante 3
	}
}
/* USER CODE END 2 */
