/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
#define nextPageButton_Pin GPIO_PIN_5
#define nextPageButton_GPIO_Port GPIOA
#define nextPageButton_EXTI_IRQn EXTI9_5_IRQn
#define button2_Pin GPIO_PIN_6
#define button2_GPIO_Port GPIOA
#define button2_EXTI_IRQn EXTI9_5_IRQn
#define button3_Pin GPIO_PIN_7
#define button3_GPIO_Port GPIOA
#define button3_EXTI_IRQn EXTI9_5_IRQn
#define SR_DATA_Pin GPIO_PIN_15
#define SR_DATA_GPIO_Port GPIOA
#define SR_LATCH_Pin GPIO_PIN_3
#define SR_LATCH_GPIO_Port GPIOB
#define SR_CLOCK_Pin GPIO_PIN_4
#define SR_CLOCK_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
extern int chiamata;

#define Ndata 11 //numero dati totale da visualizzare su display
#define raggioRuota 0.225	//in metri
extern uint16_t freniData;
extern uint16_t r2dData;
extern uint16_t mapData;
extern CAN_TxHeaderTypeDef r2dTxHeader, mapTxHeader;
extern CAN_FilterTypeDef can_filter;
extern CAN_RxHeaderTypeDef RxHeader;
extern uint32_t TxMailbox;
extern CAN_HandleTypeDef hcan;
extern UART_HandleTypeDef huart2;

extern uint16_t vehicleSpeed;
extern uint16_t tempBatteries;
extern uint16_t voltBattery;
extern uint16_t tempMot1;
extern uint16_t tempMot2;
extern uint16_t tempAvgMot;
extern uint16_t tempInverter1;
extern uint16_t tempInverter2;
extern uint16_t tempAvgInverter;
extern char names[Ndata][50];
extern uint16_t currentPageDisplay;
extern uint8_t cmd_end[3];
extern char pageDisplayArray[][5];

extern uint8_t flags[Ndata];	//struttura che contiene i flag dei valori che si sono aggiornati
extern uint8_t active[Ndata];
extern uint16_t* arrayData[Ndata];
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
