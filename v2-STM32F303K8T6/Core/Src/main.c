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
#include "tim.h"
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

//daeliminare
int ind = 0;
uint16_t freniData = 1;
uint16_t r2dData = 0;
uint16_t mapData = 2;
CAN_TxHeaderTypeDef r2dTxHeader, mapTxHeader;
CAN_FilterTypeDef can_filter;
CAN_RxHeaderTypeDef RxHeader;
uint32_t TxMailbox;

uint16_t vehicleSpeed;
uint16_t tempBatteries;
uint16_t voltBattery = 0;	//lo considero intero (moltiplicato *10 quando arriva)
uint16_t tempMot1;
uint16_t tempMot2;
uint16_t tempAvgMot;
uint16_t tempInverter1;
uint16_t tempInverter2;
uint16_t tempAvgInverter;
uint16_t statoBatteria;
uint8_t pageRefreshata = 1;

//nomi delle variabili degli elementi sul nextion (manca il r2d,caso particolare)
char names[Ndata][50]= {"page","speed_value.txt","battery_temp2.txt","lv_battery.txt","engine_tempLX2.txt","engine_tempRX2.txt","engine_temp1.txt","inverter_temp1.txt","engine_mod.txt","battery_bar.val"};
uint16_t currentPageDisplay = 0;
uint8_t cmd_end[3] = {0xFF,0xFF,0xFF}; //per inviare il comando
char pageDisplayArray[][5] = {"main","temp"};

//daTogliere ->
uint8_t flagMapPopupActive=0; // per evitare di aggioranre i dati sottostanti al popup nella pagina "main"
uint8_t flagMapPopupHide = 0;

uint8_t flags[Ndata];
uint8_t active[Ndata] = {-1,1,0,1,0,0,1,0,1,1,1};	//indica se l'elemento è attivo. Attivo -> è presente nella pagina attuale.  Inizializzato con lo stato della pagina principale
uint32_t lastMillis[Ndata];
uint16_t* arrayData[Ndata] = {&currentPageDisplay,&vehicleSpeed,&tempBatteries,&voltBattery,&tempMot1,&tempMot2,&tempAvgMot,&tempAvgInverter,&mapData,&statoBatteria,&r2dData};
//arrayData = array di PUNTATORI delle variabili contenenti i dati
/*
arrayData[0] = currentPage del display
arrayData[1] = vehicleSpeed
arrayData[2] = temperatura batterie
arrayData[3] = voltaggio batterie
arrayData[4] = tempMotore 1 SINISTRA
arrayData[5] = tempMotore 2 DESTRA	//si aggiornano insieme
arrayData[6] = temperatura media motore
arrayData[7] = tempAvgInverter
arrayData[8] = mappa
arrayData[9] = Stato Batteria (in percentuale)
arrayData[10] = r2d
CASI PARTICOLARI POSIZONI = 9,10 ->  statoBatteria = intero | r2d = mostrare/nascondere
PAGINA 1 POSIZONI = 1,3,6,8,9,10	//page non appartiene a nessuna pagina
PAGINA 2 POSIZIONI = 2,4,5,7
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
	int len;

	char msg2[9] = " ";
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
  MX_TIM2_Init();
  MX_TIM3_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */
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


  int tempo=0;
  HAL_Delay(100);
  uint32_t currMillis = HAL_GetTick();
  char msg[40] = " ";
  HAL_TIM_Base_Start(&htim3);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


  while (1)
  {
	  //CODICE PER GENERARE DATI FITTIZI

	  //*arrayData[1] = rand()%200;			//speed
	  *arrayData[2] = rand()%100;				//temp batt
	  *arrayData[3] = rand()%15;				//lv batter
	  *arrayData[4] = rand()%100;			//temp sx engine
	  *arrayData[5] = rand()%100;			//temp dx engine
	  *arrayData[6] = rand()%100;			//temp engine
	  *arrayData[7] = rand()%100;			//temp inver
	  //*arrayData[8] = rand()%3 +1;					//mappa
	  *arrayData[9] = rand()%100;					//statoBatteria
	  //*arrayData[10] = rand()%2 ;					//r2d


	  //daTogliere
	  //*arrayData[1] = tempo;

	  if (tempo==50){
		  //checkMapValue();
		  tempo =0;
	  }

	  if(flags[0]==1){
			char msg[30] = " ";
			len = sprintf(msg,"page %s",pageDisplayArray[currentPageDisplay]);
			HAL_UART_Transmit(&huart2,(uint8_t*)msg,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
			flags[0]=0;
			HAL_Delay(70);
			pageRefreshata=1;
	  }

	  for(uint8_t i=1;i<Ndata;i++ ){
		  //daTogliere
		  flags[i]=1;
		  if (flags[i] == 1 || (active[i] == 1 && currMillis-lastMillis[i] > 2000)){	//al massimo ogni due secondi ogni valore si aggiorna
			  if (i==8){
				  continue;
			  }
			  //mandare al nextion
			  NEXTION_SendString(names[i], *arrayData[i], i);
			  flags[i] = 0;
			  lastMillis[i] = HAL_GetTick();
			  HAL_Delay(1);
		  }
	  }


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  tempo++;
	  HAL_Delay(200);  //tempo++;
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
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
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
	if (index == 9){	//battery bar value
		len = sprintf(buff,"%s=%d",elemento,valore);
		HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
		HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
		if(valore < 15){
			len = sprintf(buff,"battery_bar.pco=59392");
		} else if (valore> 50){
			len = sprintf(buff,"battery_bar.pco=1796");
		} else{
			len = sprintf(buff,"battery_bar.pco=65504");
		}
		HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
		HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
	} else if (index == 10){	//r2d
		HAL_CAN_AddTxMessage(&hcan, &r2dTxHeader, &r2dData, &TxMailbox);
		//len = sprintf(buff,"readyToDrive.val=%d",r2dData);
		//HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
		//HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
		if(r2dData==0){
			len = sprintf(buff,"vis rtd_red,1");
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
			len = sprintf(buff,"vis rtd_green,0");
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
		}
		else if (r2dData==1){
			len = sprintf(buff,"vis rtd_red,0");
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
			len = sprintf(buff,"vis rtd_green,1");
			HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
		}
	HAL_Delay(10);
	}
	else if (index==7){
		len = sprintf(buff,"%s=\"%d\"",elemento,valore);
		HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
		HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
		len = sprintf(buff,"inverter_temp2.txt=\"%d\"",valore);
		HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
		HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY);
	}
	else{
		if(index==3){	//voltage battery -> float ma intero
			len = sprintf(buff,"%s=\"%dV\"",elemento,(int)(valore));
		}
		else{//caso generale
			len = sprintf(buff,"%s=\"%d\"",elemento,valore);
		}
		HAL_UART_Transmit(&huart2,(uint8_t*)buff,len,HAL_MAX_DELAY);
		HAL_UART_Transmit(&huart2,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
	}
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
