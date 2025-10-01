/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f3xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define r2dButton_Pin GPIO_PIN_4
#define r2dButton_GPIO_Port GPIOA
#define r2dButton_EXTI_IRQn EXTI4_IRQn
#define resetButton_Pin GPIO_PIN_5
#define resetButton_GPIO_Port GPIOA
#define resetButton_EXTI_IRQn EXTI9_5_IRQn
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SR_DATA_Pin GPIO_PIN_15
#define SR_DATA_GPIO_Port GPIOA
#define SR_LATCH_Pin GPIO_PIN_3
#define SR_LATCH_GPIO_Port GPIOB
#define SR_CLOCK_Pin GPIO_PIN_4
#define SR_CLOCK_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

#define NData 8
typedef struct {
    const char* varName;
   // uint8_t type;	//0 = txt; 1 = val; 2= bco
    uint16_t currentValue;
    uint16_t lastValue;
    volatile uint8_t flag;		//refresh the value if the new value is different from the last one
    uint16_t lastMillis;
} DisplayVar;

extern DisplayVar vars[NData];
extern uint16_t freniData;

extern uint8_t RxData[8];
extern uint16_t freniData;
extern uint16_t r2dData;
extern uint16_t mapData;

extern CAN_TxHeaderTypeDef r2dTxHeader, mapTxHeader;
extern CAN_FilterTypeDef can_filter;
extern CAN_RxHeaderTypeDef RxHeader;
extern uint32_t TxMailbox;
extern CAN_HandleTypeDef hcan;

extern volatile uint8_t flagErroreInCorso;
extern uint16_t ultimoErroreRicevuto;
extern uint8_t errorValue;
extern uint8_t flagError;
extern char errorName[20];

extern volatile uint8_t flagStartingOK;
extern uint8_t newData;
extern volatile uint8_t flagNewMap;


#define NFlagsInterrupt 1

extern uint32_t millisFlagsInterrupt[NFlagsInterrupt];
extern volatile uint8_t flagsUsable[NFlagsInterrupt];

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
