/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define check_button1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
CAN_FilterTypeDef sFilterConfig;
CAN_TxHeaderTypeDef TxHeader, r2dTxHeader, mapTxHeader;
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
#define RAGGIORUOTA 5 //metri
//uint8_t freniRxData[1];
uint32_t TxMailbox;


int8_t nData = 15;
int32_t data[15];
int8_t datacheck = 0;
//battery_bar,speed_value,w_last_lap,estimated_laps,engine_mod,inverter_temp1,delta_time,engine_temp1,lap_time,lv_battery,readyToDrive.val,engine_tempLX2,engine_tempRX2,inverter_temp2,battery_temp2
char names[15][30]= {"battery_bar","speed_value","w_last_lap","estimated_laps","engine_mod","inverter_temp1","delta_time","engine_temp1","lap_time","lv_battery","readyToDrive","engine_tempLX2","engine_tempRX2","inverter_temp2","battery_temp2"};
int8_t txt[] = {1,2,3,4,5,6,7,8,9,11,12,13,14};
int8_t val[] = {0,1};

uint8_t currentPage = 0;

uint8_t cmd_end[3] = {0xFF,0xFF,0xFF}; //per inviare il comando

//inizializzazione variabili per ricezione CAN
uint8_t frenoPremuto = 0,tempBattery =0, speed=0;
float voltBattery=0;
uint16_t tempMot1 = 0, tempMot2 = 0, tempAvgMot = 0, tempInverter1=0,tempInverter2=0,tempAvgInverter=0;


//comunica con il nextion tramite messaggi tipo "oggetto.txt/val = {valore}" + 3volte comando 0xFF per confermare
void NEXTION_SendString (char* ID, char* string,char* tipo){ //tipo può essere txt o val
	char buff[50];
	int len = sprintf(buff,"%s.%s=\"%s\"",ID,tipo,string);
	HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,1000);
	HAL_UART_Transmit(&huart2,cmd_end,3,100); //invio comandi = esegue
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
	if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, RxData)== HAL_OK){
		// 'rxHeader' contiene le informazioni del messaggio CAN ricevuto
		        // 'rxData' contiene i dati del messaggio ricevuto
		switch(RxHeader.StdId){
		case 0x012: //id brakeLigh
			frenoPremuto = (RxData[0]>>0) & 0x01; //prendo il prime byte, il bit posizione 0, &0x01 mascehra tutti gli altri bit;
			break;
		case 0x021:
			tempBattery = (uint8_t)(RxData[0]+RxData[1]+RxData[2]+RxData[3]+RxData[4]+RxData[5])/6; //temperatura media Pacco Batteria
			break;
		case 0x053:
			memcpy(&voltBattery,RxData,sizeof(float));
			// pacco batteria-> voltBattery = (uint8_t)(RxData[0]+RxData[1]+RxData[2]+RxData[3]+RxData[4]+RxData[5])/6; //voltaggio media Pacco Batteria
			break;
		case 0x023:
			tempMot1 = (uint16_t)(RxData[0]<<8|RxData[1]);
			tempMot2 = (uint16_t)(RxData[2]<<8|RxData[3]);
			tempAvgMot = (uint16_t)((tempMot1+tempMot2)/2);
			tempInverter1 = (uint16_t)(RxData[4]<<8|RxData[5]);
			tempInverter2 = (uint16_t)(RxData[6]<<8|RxData[7]);
			tempAvgInverter = (uint16_t)((tempInverter1+tempInverter2)/2);
			break;
		case 0x030:
			speed = (uint8_t)((((RxData[0]<<8|RxData[1])+(RxData[2]<<8|RxData[3]))/4)*RAGGIORUOTA*3.6);
			break;





		}
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */


  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x012 << 5;		//id di base da controllare
  sFilterConfig.FilterIdLow = 0;	//= utilizzato
  sFilterConfig.FilterMaskIdHigh = 0x3F << 5; // Maschera per coprire l'intervallo da 0x012 a 0x053  sFilterConfig.FilterMaskIdLow = 0;
  sFilterConfig.FilterMaskIdLow = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK; // Modalità maschera
  sFilterConfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.FilterBank = 0;
  HAL_CAN_ConfigFilter(&hcan, &sFilterConfig);
  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

  r2dTxHeader.DLC = 1;	//numero byte che deve ricevere (in questo caso basterebbe un bit)
  r2dTxHeader.StdId = 0x016; //id pacchetto
  r2dTxHeader.IDE = CAN_ID_STD;
  r2dTxHeader.RTR = CAN_RTR_DATA; //DATA

  mapTxHeader.DLC = 1;		//Numero byte messaggio, da 1 a 8
  mapTxHeader.StdId = 0x040; // Standard Identifier, va da 0 a 0x7FF (11bit)
  mapTxHeader.IDE = CAN_ID_STD;	//Indirizzi standard e non extended
  mapTxHeader.RTR = CAN_RTR_DATA; //DATA

  HAL_Delay(1000);

  uint32_t currMillis = HAL_GetTick();

  //NEXTION_SendString("t0","Hello");
  //NEXTION_SendString("t1","World");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */



	  //R2D
	  //Lettura stato del GPIO Button, controllo se è alto o basso, 0 = Premuto
	  if(HAL_GPIO_ReadPin(B1_button_GPIO_Port, B1_button_Pin) == 0 && (HAL_GetTick()-currMillis)>1000){
		  //premuto
		  currMillis = HAL_GetTick();
		  if (frenoPremuto){
			  uint8_t r2dToDriveValore=1;
			  HAL_CAN_AddTxMessage(&hcan, &r2dTxHeader, &r2dToDriveValore, &TxMailbox);
			  data[10]=1;
		  }
	  }

	  if(HAL_GPIO_ReadPin(B2_button_GPIO_Port, B2_button_Pin) == 0 && (HAL_GetTick()-currMillis)>1000){
	  		  //premuto pulsante 2
		  currMillis = HAL_GetTick();
		  if (currentPage){
				currentPage =0;
				char buff[10];
				int len = sprintf(buff,"page main");
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,1000);
				HAL_UART_Transmit(&huart2,cmd_end,3,100);
	  		  } else{
	  			currentPage = 1;
	  			char buff[10];
				int len = sprintf(buff,"page temp");
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,1000);
				HAL_UART_Transmit(&huart2,cmd_end,3,100);
	  		  }
	  	  }


	  char tipo[3];
	  for(int i =0;i<nData;i++){
			  if (i==0 || i==10){
				  strcpy(tipo,"val");
			  } else{
				  strcpy(tipo,"txt");
			  }
			  NEXTION_SendString((char*)names[i],(char*)data[i],tipo);
		  }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN;
  hcan.Init.Prescaler = 8;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_7TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_7TQ;
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

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_Pin|SR_DATA_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SR_LATCH_Pin|SR_CLOCK_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_Pin SR_DATA_Pin */
  GPIO_InitStruct.Pin = LED_Pin|SR_DATA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SR_LATCH_Pin SR_CLOCK_Pin */
  GPIO_InitStruct.Pin = SR_LATCH_Pin|SR_CLOCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : B1_button_Pin B2_button_Pin B3_button_Pin */
  GPIO_InitStruct.Pin = B1_button_Pin|B2_button_Pin|B3_button_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
