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
	  HAL_CAN_Start(&hcan1);

	  if(HAL_CAN_ConfigFilter(&hcan1, &can_filter)!= HAL_OK){
		  Error_Handler();
	  }
	  if(HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING)!= HAL_OK){
		  Error_Handler();
	  }


	  //HAL_CAN_ConfigFilter(&hcan1, &can_filter);
	  //HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

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

CAN_HandleTypeDef hcan1;

/* CAN1 init function */
void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 9;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_3TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_4TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = ENABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(CAN1_TX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_SetPriority(CAN1_RX1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN1 GPIO Configuration
    PB8     ------> CAN1_RX
    PB9     ------> CAN1_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8|GPIO_PIN_9);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(CAN1_TX_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX0_IRQn);
    HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
		HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
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
			//HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
			break;
		case 0x024:	//tmpFans
			if(RxHeader->DLC == 4){
		        uint16_t tempCool1 = (uint16_t)(buf[0] << 8 | buf[1]);
		        uint16_t tempCool2 = (uint16_t)(buf[2] << 8 | buf[3]);
		        tempAvgFan = (uint16_t)((tempCool1 + tempCool2) / 2);
		        vars[5].flag=1;

			}
		case 0x021:	//tmpBatteries
			if(RxHeader->DLC == 6){
				tempAvgBat = (uint8_t)((buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5])/6); //temperatura media Pacco Batteria
				vars[4].flag=1;
			}
			break;
		case 0x023:	//tmpEngine and tmpInverter
			if(RxHeader->DLC == 8){
				/*
		        int tempMot1 = (uint16_t)(buf[1] << 8 | buf[0]);  // SX (Corretto)
		        int tempMot2 = (uint16_t)(buf[3] << 8 | buf[2]);  // DX (Corretto)
		        tempAvgMot = (uint16_t)((tempMot1 + tempMot2) / 2);
		        int tempInverter1 = (uint16_t)(buf[5] << 8 | buf[4]); // Corretto
		        int tempInverter2 = (uint16_t)(buf[7] << 8 | buf[6]); // Corretto
		        tempAvgInverter = (uint16_t)((tempInverter1 + tempInverter2) / 2);
		        */
				uint16_t tempMot1 = (uint16_t)(buf[0] << 8 | buf[1]);
				uint16_t tempMot2 = (uint16_t)(buf[2] << 8 | buf[3]);
		        tempAvgMot = (uint16_t)((tempMot1 + tempMot2) / 2);
		        uint16_t tempInverter1 = (uint16_t)(buf[4] << 8 | buf[5]);
		        uint16_t tempInverter2 = (uint16_t)(buf[6] << 8 | buf[7]);
		        tempAvgInverter = (uint16_t)((tempInverter1 + tempInverter2) / 2);
		        vars[2].flag=1;
		        vars[3].flag=1;
			}
			break;
		case 0x031:		//RPM x TEST
				if(RxHeader->DLC == 2){
					vehicleSpeed = (uint16_t)(buf[0] | (buf[1] << 8));
					vars[0].flag=1;
				}
				break;
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
				vehicleSpeed = (uint8_t)((val1+val2)/2);
				vars[0].flag=1;
			}
			break;

		case 0x026:	//SoC
			if(RxHeader->DLC == 1){
				battery_SOC = (uint16_t)(buf[0]);
				vars[1].flag=1;

				canBUS_RX++;
			}
			break;
			//ERRORI

		case 0x001:
			if (!flagErroreInCorso){
				flagErroreInCorso = 1;
				errorValue = 1;
				NEXTION_SendString("ErrorValue.txt",1, 11);
				ultimoErroreRicevuto = HAL_GetTick();
				HAL_TIM_Base_Start_IT(&htim2);
			}
			break;
		case 0x002:
			if (!flagErroreInCorso){
				flagErroreInCorso = 1;
				errorValue = 2;
				NEXTION_SendString("ErrorValue.txt",2, 11);
				ultimoErroreRicevuto = HAL_GetTick();
				HAL_TIM_Base_Start_IT(&htim2);
			}
			break;
		case 0x003:
			if (!flagErroreInCorso){
				flagErroreInCorso = 1;
				errorValue = 3;
				NEXTION_SendString("ErrorValue.txt",3, 11);
				ultimoErroreRicevuto = HAL_GetTick();
				HAL_TIM_Base_Start_IT(&htim2);
			}
			break;
		case 0x004:
			if (!flagErroreInCorso){
				flagErroreInCorso = 1;
				errorValue = 4;
				NEXTION_SendString("ErrorValue.txt",4, 11);
				ultimoErroreRicevuto = HAL_GetTick();
				HAL_TIM_Base_Start_IT(&htim2);
			}
			break;
		case 0x005:
			if (!flagErroreInCorso){
				flagErroreInCorso = 1;
				errorValue = 5;
				NEXTION_SendString("ErrorValue.txt",5, 11);
				ultimoErroreRicevuto = HAL_GetTick();
				HAL_TIM_Base_Start_IT(&htim2);
			}
			break;
		case 0x006:
			if (!flagErroreInCorso){
				flagErroreInCorso = 1;
				errorValue = 6;
				NEXTION_SendString("ErrorValue.txt",6, 11);
				ultimoErroreRicevuto = HAL_GetTick();
				HAL_TIM_Base_Start_IT(&htim2);
			}
			break;

    }
}
/* USER CODE END 1 */
