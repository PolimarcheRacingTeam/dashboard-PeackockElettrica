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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "can.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
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
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define SR_DATA_Pin GPIO_PIN_5
#define SR_DATA_GPIO_Port GPIOB
#define SR_LATCH_Pin GPIO_PIN_6
#define SR_LATCH_GPIO_Port GPIOB
#define SR_CLOCK_Pin GPIO_PIN_7
#define SR_CLOCK_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */


#define Ndata 9 //numero dati totale da visualizzare su display
#define NFlagsInterrupt 1


extern volatile uint8_t flagErroreInCorso;
extern uint16_t ultimoErroreRicevuto;

#define raggioRuota 0.225	//in metri
extern uint16_t freniData;
extern uint16_t r2dData;
extern uint8_t RxData[8];
extern uint16_t mapData;
extern CAN_TxHeaderTypeDef r2dTxHeader, mapTxHeader;
extern CAN_FilterTypeDef can_filter;
extern CAN_RxHeaderTypeDef RxHeader;
extern uint32_t TxMailbox;
extern CAN_HandleTypeDef hcan;
extern UART_HandleTypeDef huart2;
extern uint8_t pageRefreshata;

extern uint8_t errorValue;
extern uint8_t flagError;
extern char errorName[20];

extern uint32_t millisFlagsInterrupt[NFlagsInterrupt];
extern volatile uint8_t flagsUsable[NFlagsInterrupt];

extern uint16_t vehicleSpeed;
extern uint16_t tempBatteries;
extern uint16_t voltBattery;
extern uint16_t tempAvgMot;
extern uint16_t tempAvgInverter;
extern uint16_t tempAvgFan;
extern uint16_t statoBatteria;
extern char names[Ndata][50];
extern uint8_t cmd_end[3];

extern uint8_t flags[Ndata];	//struttura che contiene i flag dei valori che si sono aggiornati
extern uint16_t* arrayData[Ndata];

extern volatile uint8_t flagOK;
extern uint8_t newData;
extern volatile uint8_t flagNewMap;

extern uint32_t lastMillis[Ndata];


/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
