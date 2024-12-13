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
  hcan.Init.Prescaler = 16;
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
	uint8_t buf[8];	//arrivano 8byte
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, buf) == HAL_OK) {
        process_can_message(&RxHeader, buf);}
}

void process_can_message(CAN_RxHeaderTypeDef *RxHeader, uint8_t *buf) {
    switch(RxHeader->StdId){
    case 0x012:	//brakeLight -> frenoPremuto
    	uint8_t bit_0 = buf[0] & 0x01;	//prende il LSB
    	freniData = bit_0;
    	break;
    case 0x030:	//velocita angolari ruote
    	//lettura in Little-Endian
		if(RxHeader->DLC == 4){
    	int16_t val1 = (int16_t)(buf[0] | (buf[1] << 8));  // Byte 0-1	velocita angolare delle due ruote
    	int16_t val2 = (int16_t)(buf[2] | (buf[3] << 8));  // Byte 2-3
    	//lettura in Big-Endian
    	//int16_t val1 = (int16_t)((buf[0] << 8) | buf[1]);  // Byte 0-1
    	//int16_t val2 = (int16_t)((buf[2] << 8) | buf[3]);  // Byte 2-3
    	vehicleSpeed = (uint16_t)((val1+val2)/2)*raggioRuota;
    	flags[1] = 1;
    	break;}
	case 0x021:
		if(RxHeader->DLC == 6){
		tempBatteries = (uint8_t)(buf[0]+buf[1]+buf[2]+buf[3]+buf[4]+buf[5])/6; //temperatura media Pacco Batteria
		flags[2] = 1;
		break;}
	case 0x023:
		if(RxHeader->DLC == 8){
		tempMot1 = (uint16_t)(buf[0]<<8|buf[1]);	//SX
		tempMot2 = (uint16_t)(buf[2]<<8|buf[3]);	//DX
		tempAvgMot = (uint16_t)((tempMot1+tempMot2)/2);
		tempInverter1 = (uint16_t)(buf[4]<<8|buf[5]);
		tempInverter2 = (uint16_t)(buf[6]<<8|buf[7]);
		tempAvgInverter = (uint16_t)((tempInverter1+tempInverter2)/2);
		flags[4] = 1;
		flags[5] = 1;
		flags[6] = 1;
		flags[7] = 1;
		break;}
	case 0x052:
		if(RxHeader->DLC =4){
		voltBattery = (uint16_t)((buf[0] | (buf[1]<<8)));
		flags[3] = 1;
		}
    }
}


/* USER CODE END 1 */
