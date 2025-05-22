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

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

volatile uint8_t flagErroreInCorso = 0;
uint16_t ultimoErroreRicevuto = 0;


volatile uint8_t flagOK = 0;
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
uint16_t tempBatteries;
uint16_t tempAvgMot;
uint16_t tempAvgInverter;
uint16_t statoBatteria;
uint16_t tempAvgFan;
uint8_t newData = 0;
uint8_t flagError = 0;
char errorName[20] = " ";
uint8_t errorValue = 0;
uint32_t millisFlagsInterrupt[NFlagsInterrupt];	//indica quando è stato chiamato quell'interrupt
volatile uint8_t flagsUsable[NFlagsInterrupt] = {1};

//nomi delle variabili degli elementi sul nextion (manca il r2d,caso particolare)
char names[Ndata][50]= {"SpeedValue.txt","TempBat.txt","TempEng.txt","TempInv.txt","TempFan.txt","MapValue.txt","BatteryValBar.val","R2DValue.pco","ErrorValue.txt"};
uint8_t cmd_end[3] = {0xFF,0xFF,0xFF}; //per inviare il comando

uint8_t flags[Ndata];
uint32_t lastMillis[Ndata];
uint16_t* arrayData[Ndata] = {&vehicleSpeed,&tempBatteries,&tempAvgMot,&tempAvgInverter,&tempAvgFan,&mapData,&statoBatteria,&r2dData,&flagError};

uint8_t tempo = 0;
//arrayData = array di PUNTATORI delle variabili contenenti i dati
/*
arrayData[0] = vehicleSpeed
arrayData[1] = temperatura batterie
arrayData[2] = temperatura media motore
arrayData[3] = tempAvgInverter
arrayData[4] = mappa
arrayData[5] = Stato Batteria (in percentuale)
arrayData[6] = r2d
arrayData[7] = flag_errorID
 * */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_NVIC_Init(void);
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
	  char msg[40] = " ";
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
  MX_USART2_UART_Init();
  HAL_Delay(500);
  len = sprintf(msg, "page logoStart");
  HAL_UART_Transmit(&huart2,(uint8_t*)msg,len,HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
  HAL_Delay(2000);	//delay
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
  CAN_setup();	//avvia il CAN + filtro
  HAL_Delay(100);

  len = sprintf(msg, "page logoStart");
  HAL_UART_Transmit(&huart2,(uint8_t*)msg,len,HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
  uint32_t currMillis = HAL_GetTick();
  flagOK = 1;
  len = sprintf(msg, "page main");
  HAL_UART_Transmit(&huart2,(uint8_t*)msg,len,HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
  len = sprintf(msg, "vis ErrorBar1,0");
  HAL_UART_Transmit(&huart2,(uint8_t*)msg,len,HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
  len = sprintf(msg, "vis ErrorBar2,0");
  HAL_UART_Transmit(&huart2,(uint8_t*)msg,len,HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {	   currMillis = HAL_GetTick();


  	  if(tempo == 10){/*
  		  //CODICE PER GENERARE DATI FITTIZI
  		  *arrayData[0] = rand()%200;			//speed
  		  *arrayData[1] = rand()%100;				//temp batt
  		  *arrayData[2] = rand()%100;			//temp engine
  		  *arrayData[3] = rand()%100;			//temp inver
  		  *arrayData[4] = rand()%50;	  				//temp fan
  		  //*arrayData[5] = rand()%3 +1;		//mappa
  		  *arrayData[6] = rand()%101 ;		//statoBatteria (SoC %)
  		  *arrayData[7] = rand()%2;			//r2d
  		  *arrayData[8] = rand()%2;			//flagErrore (0-1)
  		  //newData++;
  		   *
  		   * for(uint8_t i = 0; i<Ndata;i++){
  			  flags[i]=1;
  		  }
  		  */
  		  tempo = 0;


  	  }

  	  if(!flagNewMap){
  		  for(uint8_t i=0;i<Ndata;i++ ){
  			  if (flags[i] == 1 || currMillis-lastMillis[i] > 2000){	//al massimo ogni due secondi ogni valore si aggiorna (display)
  				  //mandare al nextion
  				  if (i==5){
  					 MapValue();
  				  }
  				  flags[i] = 0;
  				  NEXTION_SendString(names[i], *arrayData[i], i);
  				  lastMillis[i] = HAL_GetTick();
  				  HAL_Delay(1);
  			  }
  		  }
  	  }

	  for (uint8_t i=0;i<NFlagsInterrupt;i++){						//aiuta ad evitare doppio click
		  if(!flagsUsable[i]){
			  if(currMillis - millisFlagsInterrupt[i] > 700){
					 flagsUsable[i] = 1;
			  }
		  }
	  }

	  /*
	  if (vehicleSpeed == 252){
		  newData++;
		  vehicleSpeed++;
	  }*/

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  tempo++;
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* EXTI9_5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
  /* EXTI4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);
  /* CAN_RX0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(CAN_RX0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(CAN_RX0_IRQn);
}

/* USER CODE BEGIN 4 */
//comunica con il nextion, inva messaggi tipo "oggetto.txt/val = {valore}" + 3volte comando 0xFF per eseguire
void NEXTION_SendString (char* elemento,int valore,int index){ //tipo può essere txt o val
	char buff[50];
	int len;
	switch (index){
		case 1:	//temp Batteries
			if(valore > sogliaTempBatteria){
				len = sprintf(buff,"TempBat.bco=63488");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
				len = sprintf(buff,"t8.bco=63488");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			}
			else{
				len = sprintf(buff,"%s=\"%d\"",elemento,valore);
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
				len = sprintf(buff,"TempBat.bco=0");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
				len = sprintf(buff,"t8.bco=0");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			}
			break;
		case 2:	//temp Engine
			if(valore > sogliaTempMotori){
				len = sprintf(buff,"TempEng.bco=63488");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
				len = sprintf(buff,"t7.bco=63488");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			}
			else{
				len = sprintf(buff,"%s=\"%d\"",elemento,valore);
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
				len = sprintf(buff,"TempEng.bco=0");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
				len = sprintf(buff,"t7.bco=0");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			}
			break;
		case 3:	//temp Inver
			if(valore > sogliaTempInverter){
				len = sprintf(buff,"TempInv.bco=63488");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
				len = sprintf(buff,"t6.bco=63488");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			}
			else{
				len = sprintf(buff,"%s=\"%d\"",elemento,valore);
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
				len = sprintf(buff,"TempInv.bco=0");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
				len = sprintf(buff,"t6.bco=0");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			}
			break;
		case 4:	//temp Fan
			if(valore > sogliaTempFan){
				len = sprintf(buff,"TempFan.bco=63488");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
				len = sprintf(buff,"t9.bco=63488");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			}
			else{
				len = sprintf(buff,"%s=\"%d\"",elemento,valore);
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
				len = sprintf(buff,"TempFan.bco=0");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
				len = sprintf(buff,"t9.bco=0");	//rosso
				HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
				HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			}
			break;
		case 6://battery bar value + %
			len = sprintf(buff,"%s=%d",elemento,valore);
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
			if(valore < 20){
				len = sprintf(buff,"BatteryValBar.pco=47104");	//rosso 		-> 0-19 %
			} else if (valore >=20 && valore <=35){
				len = sprintf(buff,"BatteryValBar.pco=64512");		//arancio		-> 20-35 %
			} else if (valore> 35 && valore <= 55){
				len = sprintf(buff,"BatteryValBar.pco=65504");		//giallo 		-> 36-55 %
			}else{
				len = sprintf(buff,"BatteryValBar.pco=1856");	//verde					-> 56-100 %
			}
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
			len = sprintf(buff,"BatteryValTex.txt=\"%d\"",valore);
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			break;
		case 7:	//r2d
			switch (r2dData){
				case 0:
					len = sprintf(buff,"%s=47104",elemento);
					break;
				case 1:
					len = sprintf(buff,"%s=1856",elemento);
					break;
			}
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			break;
		case 8:						//flagErrore = 1
			//caso errore
			if(!flagErroreInCorso){
				NEXTION_SendString("ErrorValue.txt",0, 12);
			}
			break;
		case 11:		//accendiErrore
			len = sprintf(buff,"vis ErrorValue,1",elemento);
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			len = sprintf(buff,"vis t0,1");
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			len = sprintf(buff,"ErrorValue.txt=\"%d\"",errorValue);
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			len = sprintf(buff,"vis ErrorBar1,1");
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			len = sprintf(buff,"vis ErrorBar2,1");
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			break;
		case 12:		//spegniErrore
			len = sprintf(buff,"vis ErrorValue,0",elemento);
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			len = sprintf(buff,"vis ErrorBar1,0");
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			len = sprintf(buff,"vis ErrorBar2,0");
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			len = sprintf(buff,"vis t0,0");
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			break;
		default:
			//caso generale
			len = sprintf(buff,"%s=\"%d\"",elemento,valore);
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			break;
		}
}

void MapValue(){
	if (flagOK){
		char msg[35] = " ";
		int len;
		if(checkMapValue()){
			len = sprintf(msg,"page MapPopUp");
			HAL_UART_Transmit(&huart2, &msg, len, HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			len = sprintf(msg,"mapValue.txt=\"%d\"",*arrayData[5]);
			HAL_UART_Transmit(&huart2, &msg, len, HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
			HAL_TIM_Base_Start_IT(&htim3); //avvia timer in mode one-pulse
		}
	}
}

void system_reset() {
	__disable_irq(); // Disabilita tutti gli interrupt
	NVIC_SystemReset();	//resetta il microcontrollore, senza togliere l'alimentazione dai pin
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
