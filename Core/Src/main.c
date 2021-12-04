/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
uint8_t mode = 1;
uint8_t dutyCycle = 100;
uint8_t wantedDutyCycle = 0;
uint8_t fadeMode = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void proccesDmaData(uint8_t* sign,uint16_t len);
void sendData(uint8_t* data,uint16_t len);
void pwmToLed(uint8_t* sign,uint16_t len);
void changeMode(uint8_t newMode);
void setDutyCycle(uint8_t D);
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

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* System interrupt init*/

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  USART2_RegisterCallback(proccesDmaData);
  LL_TIM_EnableIT_CC2(TIM2);
  LL_TIM_EnableCounter(TIM2);
  LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
  LL_TIM_EnableCounter(TIM2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
  while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_0)
  {
  }
  LL_RCC_HSI_Enable();

   /* Wait till HSI is ready */
  while(LL_RCC_HSI_IsReady() != 1)
  {

  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {

  }
  LL_Init1msTick(8000000);
  LL_SetSystemCoreClock(8000000);
}

/* USER CODE BEGIN 4 */
void proccesDmaData(uint8_t* sign,uint16_t len){
	uint8_t *tx_data;
	char str[len];
	for(int j=0;j<len;j++){
		str[j] = *(sign+j);
	}
	if(strstr(str,"$auto$")){
		changeMode(1);
	}
	else if(strstr(str,"$manual$")){
		changeMode(2);
	}
	else if(strstr(str,"$PWM")){
		pwmToLed(sign,len);
	}
	else{
		int len_data = asprintf(&tx_data, "\n\rInvalid command\n\rValid command:\n\r$auto$ - automatic mode\n\r$manual$ - manual mode\n\r$PWMxx$ - PWM settings in manual mode\n\rMode is %d - 1(automatic) 2(manual)\n\r",mode);
		sendData(tx_data,len_data);
		free(tx_data);
	}
}

void sendData(uint8_t* data,uint16_t len){
	USART2_PutBuffer(data, len);
}

void pwmToLed(uint8_t* sign,uint16_t len){
	uint8_t *tx_data;
	char str[len];
	for(int j=0;j<len;j++){
		str[j] = *(sign+j);
	}
	char breakset[] = "0123456789";
	if(strstr(str,"$PWM") && ((*(strpbrk(str, breakset)+1) == '$') || (*(strpbrk(str, breakset)+2) == '$') || (*(strpbrk(str, breakset)+3) == '$'))){
		wantedDutyCycle = atoi(strpbrk(str, breakset));
		int len_data = asprintf(&tx_data, "The brightness is set to: : %d %\n\r",wantedDutyCycle);
		sendData(tx_data,len_data);
		free(tx_data);
	}
	else{
		int len_data = asprintf(&tx_data, "End char '$' not found%\n\r");
		sendData(tx_data,len_data);
		free(tx_data);
	}
}

void changeMode(uint8_t newMode){
	uint8_t *tx_data;
	mode = newMode;
	if (newMode == 1){
		int len_data = asprintf(&tx_data, "Mode is set to:  automatic\n\r");
		sendData(tx_data,len_data);
		free(tx_data);
	}
	else if (newMode == 2){
		LL_mDelay(50);
		int len_data = asprintf(&tx_data, "Mode is set to:  manual\n\r");
		sendData(tx_data,len_data);
		free(tx_data);
	}
	else{
		int len_data = asprintf(&tx_data, "Mode is set to:  none\n\r");
		sendData(tx_data,len_data);
		free(tx_data);
	}
	return;
}

void changeLedPWM(){
	if(mode == 1){
		if(fadeMode == 0){
				dutyCycle -= 1;
				if(dutyCycle <= 0){
					fadeMode = 1;
				}
			}
			else if(fadeMode == 1){
				dutyCycle += 1;
				if (dutyCycle >= 100){
					fadeMode = 0;
				}
			}
	}
	else if(mode == 2){
		if(wantedDutyCycle < dutyCycle){
			dutyCycle -=1;
		}
		else if(wantedDutyCycle > dutyCycle){
			dutyCycle +=1;
		}
	}
	setDutyCycle(dutyCycle);
}

void setDutyCycle(uint8_t D){
	TIM2->CCR1 = ((TIM2->ARR) * D) / 100;
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
