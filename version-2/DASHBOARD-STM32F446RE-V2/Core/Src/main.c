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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "shift_register.h"
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

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
uint8_t errorValue = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define MAX_COMMANDS 30
#define MAX_COMMAND_LEN 80

#define COLOR_RED    63488
#define COLOR_GREEN  1856

DisplayVar vars[NData] = {
    { "SpeedValue",0, 0,1,0},
	{ "valSOC",0, 0,1,0},	//also BatteryValBar	(type1)
	{ "valTempInv",0, 0,1,0},
	{ "valTempEng",0, 0,1,0},
	{ "valTempBat",0, 0,1,0},
	{ "valTempFan",0, 0,1,0},
	{ "MapValue",0, 0,1,0},
	{ "R2DValue",0, 0,1,0}
};

uint8_t flagOk = 0;
uint8_t cmd_end[3] = {0xFF,0xFF,0xFF};

uint16_t freniData = 0;
uint16_t r2dData = 0;
uint16_t mapData = 0;

volatile uint8_t flagStartingOK = 0;
volatile uint8_t flagNewMap=0;
CAN_TxHeaderTypeDef r2dTxHeader, mapTxHeader;
CAN_FilterTypeDef can_filter;
CAN_RxHeaderTypeDef RxHeader;
uint8_t RxData[8];
uint32_t TxMailbox;

uint8_t newData = 0;
uint32_t millisFlagsInterrupt[NFlagsInterrupt];	//indica quando è stato chiamato quell'interrupt | x pulsanti
volatile uint8_t flagsUsable[NFlagsInterrupt] = {1};

typedef struct {
    char buffer[MAX_COMMAND_LEN];
    uint8_t len;
    uint8_t valid;
} Command;

Command commandQueue[MAX_COMMANDS];
volatile uint8_t queueHead = 0;
volatile uint8_t queueTail = 0;
volatile uint8_t uartBusy = 0;


void EnqueueNextionCommand(const char* format, ...) {
    char temp[MAX_COMMAND_LEN - 3];
    va_list args;
    va_start(args, format);
    vsnprintf(temp, sizeof(temp), format, args);
    va_end(args);

    Command cmd;
    memset(&cmd, 0, sizeof(Command));	//reset
    cmd.len = strlen(temp) + 3;
    memcpy(cmd.buffer, temp, strlen(temp));
    cmd.buffer[cmd.len - 3] = 0xFF;
    cmd.buffer[cmd.len - 2] = 0xFF;
    cmd.buffer[cmd.len - 1] = 0xFF;

    EnqueueCommandRaw(&cmd);
}

void EnqueueCommandRaw(Command* cmd) {
    uint8_t nextTail = (queueTail + 1) % MAX_COMMANDS;
    if (nextTail == queueHead) return; // Queue full

    commandQueue[queueTail] = *cmd;
    queueTail = nextTail;

    TrySendNextCommand();
}

void TrySendNextCommand() {
    if (uartBusy || queueHead == queueTail) return;
    uartBusy = 1;
    HAL_UART_Transmit_DMA(&huart1, (uint8_t*)commandQueue[queueHead].buffer, commandQueue[queueHead].len);
}


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        queueHead = (queueHead + 1) % MAX_COMMANDS;
        uartBusy = 0;
        TrySendNextCommand();  // Avvia il prossimo comando
    }
}


/*
void SetSpeedValue(int value) {
    char temp[64];
    snprintf(temp, sizeof(temp), "SpeedValue.txt=\"%d\"", value);		//per evitare overflow

    // Cerca se l'ultimo comando nella coda è un comando SpeedValue
    uint8_t i = queueTail == 0 ? MAX_COMMANDS - 1 : queueTail - 1;
    if (strstr(commandQueue[i].buffer, "SpeedValue.txt")) {
        snprintf(commandQueue[i].buffer, MAX_COMMAND_LEN - 3, "%s", temp);
        commandQueue[i].len = strlen(temp) + 3;
        commandQueue[i].buffer[commandQueue[i].len - 3] = 0xFF;
        commandQueue[i].buffer[commandQueue[i].len - 2] = 0xFF;
        commandQueue[i].buffer[commandQueue[i].len - 1] = 0xFF;
    } else {
        EnqueueNextionCommand("SpeedValue.txt=\"%d\"", value);
    }
}

*/

void SetDisplayVar(uint8_t index, int value) {
    if (index >= sizeof(vars)/sizeof(vars[0])) return;
    if(index == 0 || index == 6){
        EnqueueNextionCommand("%s.txt=\"%d\"", vars[index].varName, value);
    } else{
        EnqueueNextionCommand("%s.val=%d", vars[index].varName, value);
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
  MX_DMA_Init();
  MX_CAN1_Init();
  MX_USART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  char msg[60] = " ";
  int len;
  HAL_Delay(100);
  HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);
  HAL_Delay(200);
  len = sprintf(msg, "page logoStart");
  HAL_UART_Transmit(&huart1,(uint8_t*)msg,len,HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart1,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
  HAL_Delay(1500);	//delay
  HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);

  uint32_t currMillis = HAL_GetTick();
  flagStartingOK = 1;
  len = sprintf(msg, "page main");
  HAL_UART_Transmit(&huart1,(uint8_t*)msg,len,HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart1,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
  len = sprintf(msg, "vis ErrorBar1,0");
  HAL_UART_Transmit(&huart1,(uint8_t*)msg,len,HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart1,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue
  len = sprintf(msg, "vis ErrorBar2,0");
  HAL_UART_Transmit(&huart1,(uint8_t*)msg,len,HAL_MAX_DELAY);
  HAL_UART_Transmit(&huart1,cmd_end,3,HAL_MAX_DELAY); //invio comandi = esegue

  //HAL_TIM_Base_Start_IT(&htim6);

  CAN_setup();	//avvia il CAN + filtro
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  EnqueueNextionCommand("signal.val=1");  // Altro comando

	  for(uint8_t i = 0;i<NData;i++){
		  if(vars[i].flag == 1 || HAL_GetTick() - vars[i].lastMillis > 1000){
			  vars[i].flag=0;
			  vars[i].lastMillis = HAL_GetTick();
			  if(i==7){
				  r2dData ? EnqueueNextionCommand("R2DValue.pco=%d",COLOR_GREEN) : EnqueueNextionCommand("R2DValue.pco=%d",COLOR_RED);
				 break;
			  }
			  SetDisplayVar(i, vars[i].currentValue);
			  if(i==6){
				  MapValueCheck();
			  }
			  HAL_Delay(3);
		  }
	  }

	  for (uint8_t i=0;i<NFlagsInterrupt;i++){						//aiuta ad evitare doppio click
		  if(!flagsUsable[i]){
			  if(HAL_GetTick() - millisFlagsInterrupt[i] > 750){
					 flagsUsable[i] = 1;
			  }
		  }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void showError(int valore){
	EnqueueNextionCommand("vis t0,1");
	EnqueueNextionCommand("vis ErrorValue,1");
	EnqueueNextionCommand("vis ErrorBar1,1");
	EnqueueNextionCommand("vis ErrorBar2,1");
	EnqueueNextionCommand("ErrorValue.txt=\"%d\"",errorValue);
}

void hideError(){
	EnqueueNextionCommand("vis t0,0");
	EnqueueNextionCommand("vis ErrorValue,0");
	EnqueueNextionCommand("vis ErrorBar1,0");
	EnqueueNextionCommand("vis ErrorBar2,0");
}

void MapValueCheck(){
	if (flagStartingOK == 1){
		char msg[35] = " ";
		int len;
		if(!flagNewMap && checkMapValue()){
			flagNewMap=1;
			EnqueueNextionCommand("page MapPopUp");
			EnqueueNextionCommand("mapValue.txt=\"%d\"",vars[6].currentValue);
			HAL_TIM_Base_Start_IT(&htim3); //avvia timer in mode one-pulse
		}
	}
}
void system_reset() {
	EnqueueNextionCommand("rest");
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
#ifdef USE_FULL_ASSERT
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
