/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
#include "can.h"

/* USER CODE BEGIN 0 */
CAN_HandleTypeDef hcan;

#include "tim.h"

/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 9;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_3TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN)
  {
  /* USER CODE BEGIN CAN_MspInit 0 */

  /* USER CODE END CAN_MspInit 0 */
    /* CAN clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN interrupt Init */
    HAL_NVIC_SetPriority(CAN_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN_RX0_IRQn);
  /* USER CODE BEGIN CAN_MspInit 1 */

  /* USER CODE END CAN_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN)
  {
  /* USER CODE BEGIN CAN_MspDeInit 0 */

  /* USER CODE END CAN_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN_RX0_IRQn);
  /* USER CODE BEGIN CAN_MspDeInit 1 */

  /* USER CODE END CAN_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
		//HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
        process_can_message(&RxHeader, RxData);
    }
    else{
    	Error_Handler();
    }
}
void process_can_message(CAN_RxHeaderTypeDef *RxHeader, uint8_t *buf) {
    switch(RxHeader->StdId){
    uint16_t tmpValue;
		case 0x012:	//brakeLight -> frenoPremuto	--> in decimale
			freniData = buf[0] & 0x01;	//Estrai solo il bit più basso
			//HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
			break;
		case 0x024:
			if(RxHeader->DLC == 4){
		        int tempFan1 = (uint16_t)(buf[0] << 8 | buf[1]);
		        int tempFan2 = (uint16_t)(buf[2] << 8 | buf[3]);
		        tmpValue = (uint16_t)((tempFan1 + tempFan2) / 2);
		        if(vars[5].lastValue!=tmpValue){
		        	vars[5].lastValue=vars[5].currentValue;
		        	vars[5].currentValue = tmpValue;
		        	vars[5].flag=1;
		        }
			}
			break;
		case 0x021:
			if(RxHeader->DLC == 6){
				tmpValue = (uint8_t)((buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5])/6); //temperatura media Pacco Batteria
		        if(vars[4].lastValue!=tmpValue){
		        	vars[4].lastValue=vars[4].currentValue;
		        	vars[4].currentValue = tmpValue;
		        	vars[4].flag=1;
		        }
			}
			break;
		case 0x023:
			if(RxHeader->DLC == 8){
				/*
		        int tempMot1 = (uint16_t)(buf[1] << 8 | buf[0]);  // SX (Corretto)
		        int tempMot2 = (uint16_t)(buf[3] << 8 | buf[2]);  // DX (Corretto)
		        tempAvgMot = (uint16_t)((tempMot1 + tempMot2) / 2);
		        int tempInverter1 = (uint16_t)(buf[5] << 8 | buf[4]); // Corretto
		        int tempInverter2 = (uint16_t)(buf[7] << 8 | buf[6]); // Corretto
		        tempAvgInverter = (uint16_t)((tempInverter1 + tempInverter2) / 2);
		        */
		        int tempMot1 = (uint16_t)(buf[0] << 8 | buf[1]);
		        int tempMot2 = (uint16_t)(buf[2] << 8 | buf[3]);
		        tmpValue = (uint16_t)((tempMot1 + tempMot2) / 2);
		        if(vars[3].lastValue!=tmpValue){
		        	vars[3].lastValue=vars[3].currentValue;
		        	vars[3].currentValue = tmpValue;
		        	vars[3].flag=1;
		        }
		        int tempInverter1 = (uint16_t)(buf[4] << 8 | buf[5]);
		        int tempInverter2 = (uint16_t)(buf[6] << 8 | buf[7]);
		        tmpValue = (uint16_t)((tempInverter1 + tempInverter2) / 2);
		        if(vars[2].lastValue!=tmpValue){
		        	vars[2].lastValue=vars[2].currentValue;
		        	vars[2].currentValue = tmpValue;
		        	vars[2].flag=1;
		        }

			}
			break;

		case 0x031:		//RPM x TEST
				if(RxHeader->DLC == 2){
					tmpValue = (uint16_t)(buf[0] | (buf[1] << 8));
			        if(vars[0].lastValue!=tmpValue){
			        	vars[0].lastValue=vars[0].currentValue;
			        	vars[0].currentValue = tmpValue;
			        	vars[0].flag=1;
						//NEXTION_SendString(vars[0].element,*vars[0].value,0);
			        }
				}
				break;

				/*
		case 0x030:	//velocita angolari ruote
			//lettura in Little-Endian
			//HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
			break;
			if(RxHeader->DLC == 4){
				int16_t val1 = (int16_t)(buf[0] | (buf[1] << 8));  // Byte 0-1	velocita angolare delle due ruote
				int16_t val2 = (int16_t)(buf[2] | (buf[3] << 8));  // Byte 2-3
				//lettura in Big-Endian
				//int16_t val1 = (int16_t)((buf[0] << 8) | buf[1]);  // Byte 0-1
				//int16_t val2 = (int16_t)((buf[2] << 8) | buf[3]);  // Byte 2-3
				//vehicleSpeed = (uint8_t)((val1+val2)/2)*raggioRuota;
				tmpValue = (uint16_t)((val1+val2)/2);
		        if(vars[0].lastValue!=tmpValue){
		        	vars[0].lastValue=vars[0].currentValue;
		        	vars[0].currentValue = tmpValue;
		        	vars[0].flag=1;
		        }
			}
			break;
*/
		case 0x026:	//stato percentuale batteria
			if(RxHeader->DLC == 1){
				tmpValue = (uint16_t)(buf[0]);
		        if(vars[1].lastValue!=tmpValue){
		        	vars[1].lastValue=vars[1].currentValue;
		        	vars[1].currentValue = tmpValue;
		        	vars[1].flag=1;
		        }


			}
			break;
			//ERRORI

		case 0x001:
			if (!flagErroreInCorso){
				flagErroreInCorso = 1;
				errorValue = 1;
				//NEXTION_SendString("ErrorValue.txt",1, 11);
				showError(errorValue);
				ultimoErroreRicevuto = HAL_GetTick();
				HAL_TIM_Base_Start_IT(&htim2);
			}
			break;
		case 0x002:
			if (!flagErroreInCorso){
				flagErroreInCorso = 1;
				errorValue = 2;
				//NEXTION_SendString("ErrorValue.txt",2, 11);
				showError(errorValue);
				ultimoErroreRicevuto = HAL_GetTick();
				HAL_TIM_Base_Start_IT(&htim2);
			}
			break;
		case 0x003:
			if (!flagErroreInCorso){
				flagErroreInCorso = 1;
				errorValue = 3;
				//NEXTION_SendString("ErrorValue.txt",3, 11);
				showError(errorValue);
				ultimoErroreRicevuto = HAL_GetTick();
				HAL_TIM_Base_Start_IT(&htim2);
			}
			break;
		case 0x004:
			if (!flagErroreInCorso){
				flagErroreInCorso = 1;
				errorValue = 4;
				//NEXTION_SendString("ErrorValue.txt",4, 11);
				showError(errorValue);
				ultimoErroreRicevuto = HAL_GetTick();
				HAL_TIM_Base_Start_IT(&htim2);
			}
			break;
		case 0x005:
			if (!flagErroreInCorso){
				flagErroreInCorso = 1;
				errorValue = 5;
				//NEXTION_SendString("ErrorValue.txt",5, 11);
				showError(errorValue);ultimoErroreRicevuto = HAL_GetTick();
				HAL_TIM_Base_Start_IT(&htim2);
			}
			break;
		case 0x006:
			if (!flagErroreInCorso){
				flagErroreInCorso = 1;
				errorValue = 6;
				//NEXTION_SendString("ErrorValue.txt",6, 11);
				showError(errorValue);ultimoErroreRicevuto = HAL_GetTick();
				HAL_TIM_Base_Start_IT(&htim2);
			}
			break;
    }
}
/* USER CODE END 1 */
