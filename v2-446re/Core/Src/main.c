/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "can.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "shift_register.h"
#include <string.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DISPLAY_COLOR_RED 63488
#define DISPLAY_COLOR_GREEN 1856
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

volatile uint8_t flagErroreInCorso = 0;
uint16_t ultimoErroreRicevuto = 0;
/*
* 1 - speed
* 2 - SOC
* 3 - tmpInverter
* 4 - tmpEngine
* 5 - tmpBatteries
* 6 - tmpFans
* 7 - Engine MAP
* 8 - R2D
* 9 - ErrorValue
*/

int canBUS_RX = 0;
int USART_TX = 0;

volatile uint8_t flagStartingOK = 0;
volatile uint8_t flagNewMap=0;
uint16_t freniData = 0;
uint16_t r2dData = 0;
uint16_t mapData = 0;
CAN_TxHeaderTypeDef r2dTxHeader, mapTxHeader;
CAN_FilterTypeDef can_filter;
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
uint32_t TxMailbox;

uint16_t vehicleSpeed = 0;
uint16_t tempAvgBat=25;
uint16_t tempAvgMot=25;
uint16_t tempAvgInverter=25;
uint16_t battery_SOC = 100;
uint16_t tempAvgFan=30;
uint8_t newData = 0;
uint8_t errorValue = 0;

uint32_t millisFlagsInterrupt[NFlagsInterrupt];	//indica quando è stato chiamato quell'interrupt | x pulsanti
volatile uint8_t flagsUsable[NFlagsInterrupt] = {1};

uint8_t cmd_end[3] = {0xFF,0xFF,0xFF}; //per inviare il comando

DisplayElement vars[NData] =  {
		{"SpeedValue.txt",&vehicleSpeed,1,0},
		{"valSOC.val",&battery_SOC,1,0},
		{"valTempInv.val",&tempAvgInverter,1,0},
		{"valTempEng.val",&tempAvgMot,1,0},
		{"valTempBat.val",&tempAvgBat,1,0},
		{"valTempFan.val",&tempAvgFan,1,0},	//5
		{"MapValue.txt",&mapData,0,0},
		{"R2DValue.pco",&r2dData,0,0},
		{"ErrorValue.txt",&errorValue,0,0}
};
//		*(vars[i].value)

uint32_t lastSignal = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	  char msg[60] = " ";
	  int len;
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
  MX_CAN1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(100);
  HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
  HAL_Delay(100);
  len = sprintf(msg, "page logoStart");
  HAL_UART_Transmit(&huart1,(uint8_t*)msg,len,HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart1,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
  HAL_Delay(1200);	//delay - animation
  HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);

  uint32_t currMillis = HAL_GetTick();
  len = sprintf(msg, "page main");
  HAL_UART_Transmit(&huart1,(uint8_t*)msg,len,HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart1,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue

  CAN_setup();	//avvia il CAN + filtro

  flagStartingOK = 1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  currMillis = HAL_GetTick();

	  if(!flagNewMap){
		  for(uint8_t i=0;i<NData;i++ ){
			  if (vars[i].flag == 1 || vars[i].lastSent-currMillis>2000){
				  //to send to the nextion
				  vars[i].flag = 0;
				  vars[i].lastSent=currMillis;
				  NEXTION_SendString(vars[i].element,*vars[i].value, i);
				  HAL_Delay(1);
			  }
			  if(i==6){
				  MapValue();
			  }
		  }
	  }

  for (uint8_t i=0;i<NFlagsInterrupt;i++){						//aiuta ad evitare doppio click
	  if(!flagsUsable[i]){
		  if(HAL_GetTick() - millisFlagsInterrupt[i] > 750){
				 flagsUsable[i] = 1;
		  }
	  }
  }

  currMillis = HAL_GetTick();
  if(currMillis-lastSignal>1200){
  	  len = sprintf(msg, "signal.val=1");
  	  msg[len]=0xFF;msg[len+1]=0xFF;msg[len+2]=0xFF;
  	  HAL_UART_Transmit(&huart1,(uint8_t*)msg,len+3,HAL_MAX_DELAY);
  	  lastSignal = currMillis;
  }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//comunica con il nextion, inva messaggi tipo "oggetto.txt/val = {valore}" + 3volte comando 0xFF per eseguire
void NEXTION_SendString (char* elemento,int valore,int index){ //tipo può essere txt o val
	char buff[50];
	int len;
	switch (index){
		case 0:
		case 6:
			len = sprintf(buff,"%s=\"%d\"",elemento,valore);
			buff[len]=0xFF;buff[len+1]=0xFF;buff[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)buff,len+3,HAL_MAX_DELAY);
			break;
		case 7:	//r2d
			if(valore==1){
				len = sprintf(buff,"%s=%d",elemento,DISPLAY_COLOR_GREEN);
			} else if (valore==0){
				len = sprintf(buff,"%s=%d",elemento,DISPLAY_COLOR_RED);
			}
			buff[len]=0xFF;buff[len+1]=0xFF;buff[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)buff,len+3,HAL_MAX_DELAY);
			break;
		case 8:						//flagErrore = 1
			//caso errore
			if(!flagErroreInCorso){
				NEXTION_SendString("ErrorValue.txt",0, 12);
			}
			break;
		case 11:		//accendiErrore
			len = sprintf(buff,"vis ErrorValue,1",elemento);
			buff[len]=0xFF;buff[len+1]=0xFF;buff[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)buff,len+3,HAL_MAX_DELAY);
			len = sprintf(buff,"vis t0,1");
			buff[len]=0xFF;buff[len+1]=0xFF;buff[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)buff,len+3,HAL_MAX_DELAY);
			len = sprintf(buff,"ErrorValue.txt=\"%d\"",errorValue);
			buff[len]=0xFF;buff[len+1]=0xFF;buff[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)buff,len+3,HAL_MAX_DELAY);
			len = sprintf(buff,"vis ErrorBar1,1");
			buff[len]=0xFF;buff[len+1]=0xFF;buff[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)buff,len+3,HAL_MAX_DELAY);
			len = sprintf(buff,"vis ErrorBar2,1");
			buff[len]=0xFF;buff[len+1]=0xFF;buff[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)buff,len+3,HAL_MAX_DELAY);
			break;
		case 12:		//spegniErrore
			len = sprintf(buff,"vis ErrorValue,0",elemento);
			buff[len]=0xFF;buff[len+1]=0xFF;buff[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)buff,len+3,HAL_MAX_DELAY);
			len = sprintf(buff,"vis ErrorBar1,0");
			buff[len]=0xFF;buff[len+1]=0xFF;buff[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)buff,len+3,HAL_MAX_DELAY);
			len = sprintf(buff,"vis ErrorBar2,0");
			buff[len]=0xFF;buff[len+1]=0xFF;buff[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)buff,len+3,HAL_MAX_DELAY);
			len = sprintf(buff,"vis t0,0");
			buff[len]=0xFF;buff[len+1]=0xFF;buff[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)buff,len+3,HAL_MAX_DELAY);
			break;
		default:
			//general Case -> int value
			len = sprintf(buff,"%s=%d",elemento,valore);
			buff[len]=0xFF;buff[len+1]=0xFF;buff[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)buff,len+3,HAL_MAX_DELAY);
			break;
		}
}

void MapValue(){
	if (flagStartingOK == 1){
		char msg[35] = " ";
		int len;
		if(!flagNewMap && checkMapValue()){
			flagNewMap=1;
			len = sprintf(msg,"page MapPopUp");
			msg[len]=0xFF;msg[len+1]=0xFF;msg[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)msg,len+3,HAL_MAX_DELAY);
			len = sprintf(msg,"mapValue.txt=\"%d\"",mapData);
			msg[len]=0xFF;msg[len+1]=0xFF;msg[len+2]=0xFF;
			HAL_UART_Transmit(&huart1,(uint8_t*)msg,len+3,HAL_MAX_DELAY);
			HAL_TIM_Base_Start_IT(&htim3); //avvia timer in mode one-pulse
		}
	}
}

void system_reset() {
	char msg[35] = " ";
	int len = sprintf(msg,"rest");
	msg[len]=0xFF;msg[len+1]=0xFF;msg[len+2]=0xFF;
	HAL_UART_Transmit_IT(&huart1,(uint8_t*)msg,len+3);
	__disable_irq(); //disable all interrupts
	NVIC_SystemReset();	//reset the microcontroller, continue to power pins
}

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
