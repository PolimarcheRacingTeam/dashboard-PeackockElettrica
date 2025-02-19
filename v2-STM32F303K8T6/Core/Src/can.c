/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
#include "can.h"

/* USER CODE BEGIN 0 */
void CAN_setup(void){
	 //SETUP CAN
	  //filtro CAN --> Serve per far funzionare il CANBUS (obbligatorio) --> Isola messaggio che non hanno id/maschera che mi interessa
	  //tutti a 0 = non filtra niente
	  can_filter.FilterActivation = ENABLE;
	  can_filter.FilterBank = 0;
	  can_filter.FilterFIFOAssignment = CAN_RX_FIFO0;	//CAN_FILTER_FIFO0
	  can_filter.FilterIdHigh = 0;
	  can_filter.FilterIdLow = 0;
	  can_filter.FilterMaskIdHigh = 0; // Maschera per coprire l'intervallo da 0x012 a 0x053  sFilterConfig.FilterMaskIdLow = 0;
	  can_filter.FilterMaskIdLow = 0;
	  can_filter.FilterMode = CAN_FILTERMODE_IDMASK;
	  can_filter.FilterScale = CAN_FILTERSCALE_32BIT;
	  if(HAL_CAN_ConfigFilter(&hcan, &can_filter)!= HAL_OK){
		  Error_Handler();
	  }
	  HAL_CAN_Start(&hcan);
	  if(HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING)!= HAL_OK){
		  Error_Handler();
	  }

	  r2dTxHeader.DLC = 1;	//numero byte che deve mandare (in questo caso basterebbe un bit)
	  r2dTxHeader.StdId = 0x016; //id pacchetto
	  r2dTxHeader.IDE = CAN_ID_STD;
	  r2dTxHeader.RTR = CAN_RTR_DATA; //DATA-Standard

	  mapTxHeader.DLC = 1;		//Numero byte messaggio, da 1 a 8
	  mapTxHeader.StdId = 0x040; // Standard Identifier, va da 0 a 0x7FF (11bit)
	  mapTxHeader.IDE = CAN_ID_STD;	//Indirizzi standard (=0) e non extended
	  mapTxHeader.RTR = CAN_RTR_DATA; //DATA-Standard
}
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
  hcan.Init.Prescaler = 8;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_2TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
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
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData) == HAL_OK) {
        process_can_message(&RxHeader, RxData);
    }
    else{
    	Error_Handler();
    }
}

void process_can_message(CAN_RxHeaderTypeDef *RxHeader, uint8_t *buf) {
    switch(RxHeader->StdId){
		case 0x012:	//brakeLight -> frenoPremuto	--> in decimale
			freniData = buf[0] & 0x01;	//Estrai solo il bit più basso
			break;
		case 0x030:	//velocita angolari ruote
			//lettura in Little-Endian
			if(RxHeader->DLC == 4){
				int16_t val1 = (int16_t)(buf[0] | (buf[1] << 8));  // Byte 0-1	velocita angolare delle due ruote
				int16_t val2 = (int16_t)(buf[2] | (buf[3] << 8));  // Byte 2-3
				//lettura in Big-Endian
				//int16_t val1 = (int16_t)((buf[0] << 8) | buf[1]);  // Byte 0-1
				//int16_t val2 = (int16_t)((buf[2] << 8) | buf[3]);  // Byte 2-3
				vehicleSpeed = (uint8_t)((val1+val2)/2)*raggioRuota;
				vehicleSpeed = (uint8_t)((val1+val2)/2);
				flags[1] = 1;
				break;
			}
		case 0x021:
			if(RxHeader->DLC == 6){
				tempBatteries = (uint8_t)((buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5])/6); //temperatura media Pacco Batteria
				flags[2] = 1;
				break;
			}
		case 0x023:
			if(RxHeader->DLC == 8){
		        tempMot1 = (uint16_t)(buf[1] << 8 | buf[0]);  // SX (Corretto)
		        tempMot2 = (uint16_t)(buf[3] << 8 | buf[2]);  // DX (Corretto)
		        tempAvgMot = (uint16_t)((tempMot1 + tempMot2) / 2);
		        tempInverter1 = (uint16_t)(buf[5] << 8 | buf[4]); // Corretto
		        tempInverter2 = (uint16_t)(buf[7] << 8 | buf[6]); // Corretto
		        tempAvgInverter = (uint16_t)((tempInverter1 + tempInverter2) / 2);
				flags[4] = 1;
				flags[5] = 1;
				flags[6] = 1;
				flags[7] = 1;
				break;
			}
		case 0x052:
			if(RxHeader->DLC =4){
				voltBattery = (uint16_t)((buf[0] | (buf[1]<<8)));
				flags[3] = 1;
			}
    }
}


/* USER CODE END 1 */
