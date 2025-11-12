/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
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
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "christov.h"
#include "engzee.h"
#include "tradeoff.h"
#include "filter.h"
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
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
uint16_t buffer[BUF_LEN];

// Flags for processing circular buffer
uint8_t fill = 0; //1: processing 1st half; 2: processing 2nd half
bool firstHalfFull;
bool secondHalfFull;
bool lastBuffer;
bool finishedSampling;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */
int _write(int file, char *ptr, int len);
void notEnoughTimeError(void);
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
	GPIO_PinState PB12bitstatus = GPIO_PIN_RESET;
	// Filter coefficients
	float b1_filter[5] = {FLOAT_1div5, FLOAT_1div5, FLOAT_1div5, FLOAT_1div5, FLOAT_1div5 };
	float b2_filter[7] = {FLOAT_1div7, FLOAT_1div7, FLOAT_1div7, FLOAT_1div7, FLOAT_1div7,
																FLOAT_1div7, FLOAT_1div7 };
	float b_noise[10]  = {FLOAT_1div10, FLOAT_1div10, FLOAT_1div10, FLOAT_1div10,
									FLOAT_1div10, FLOAT_1div10, FLOAT_1div10, FLOAT_1div10,
																FLOAT_1div10, FLOAT_1div10 };

	// Set flags
	firstHalfFull = false;			// First Half of buffer is full and ready to be used
	secondHalfFull = false;			// Second Half of buffer is full and ready to be used
	lastBuffer = false;				// Last buffer will be processed (no more ECG is being sent)
	finishedSampling = false;		// Finish detection
	fill = 0;						// Indicates which half is being processed

	/* ---------------------------INITIALIZE STRUCTS ----------------------------------------*/
	// Structs for signals
	Signal unfiltered_ecg;		// Unfiltered ECG sampled by ADC (casted to float)
	Signal filtered_ecg_mid;	// ECG signal after first filter
	Signal filtered_ecg_C;		// Two different structs for filtered ECG section
	Signal filtered_ecg_E;		//     since their states are of different lengths
	Signal diff_C;				// Differentiated signal (Christov)
	Signal diff_E;				// Differentiated signal (Engzee)
	Signal diff_filtered_C;		// Signal after final filtering (Christov)
	Signal diff_filtered_E;		// Signal after final filtering (Engzee)

	// Zeroes all fields
	memset(&unfiltered_ecg,0,sizeof(unfiltered_ecg));
	memset(&filtered_ecg_mid,0,sizeof(filtered_ecg_mid));
	memset(&filtered_ecg_C, 0, sizeof(filtered_ecg_C));
	memset(&filtered_ecg_E, 0, sizeof(filtered_ecg_E));
	memset(&diff_C, 0, sizeof(diff_C));
	memset(&diff_E, 0, sizeof(diff_E));
	memset(&diff_filtered_C, 0, sizeof(diff_filtered_C));
	memset(&diff_filtered_E, 0, sizeof(diff_filtered_E));

	// Implements filter state and signal sizes
	unfiltered_ecg.len_state = 		FILTER_B1_ORDER;
	filtered_ecg_mid.len_state = 	FILTER_B2_ORDER;
	filtered_ecg_C.len_state = 		DIFFERENCE_CHRISTOV_STATE;
	filtered_ecg_E.len_state = 		DIFFERENCE_ENGZEE_STATE;
	diff_C.len_state = 				FILTER_B_NOISE_ORDER;
	diff_E.len_state = 				FILTER_B_NOISE_ORDER;

	unfiltered_ecg.len_signal = 	BUF_LEN_HALF;
	filtered_ecg_mid.len_signal= 	BUF_LEN_HALF;
	filtered_ecg_C.len_signal = 	BUF_LEN_HALF;
	filtered_ecg_E.len_signal = 	BUF_LEN_HALF;
	diff_C.len_signal = 			BUF_LEN_HALF;
	diff_E.len_signal = 			BUF_LEN_HALF;
	diff_filtered_C.len_signal= 	BUF_LEN_HALF;
	diff_filtered_E.len_signal= 	BUF_LEN_HALF;

	// Structs for algorithm states
	EngzeeState engzee_state;
	ChristovState christov_state;
	FinalDetect final_detect;
	memset(&engzee_state, 0, sizeof(EngzeeState)); 		// zera todos os campos
	memset(&christov_state, 0, sizeof(ChristovState)); 	// zera todos os campos
	memset(&final_detect, 0, sizeof(FinalDetect)); 		// zera todos os campos
	float increment = 0.0016064257028112205;
	for (int j = 0; j < ms1200 - ms200; ++j) {
			engzee_state.M_slope[j] = 1.0 - j * increment;
			christov_state.M_slope[j] = engzee_state.M_slope[j];
		}
	engzee_state.fs = 250;
	christov_state.fs = 250;


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
  MX_ADC1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start(&htim2);
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);	// Turns LED OFF

	// Wait for pin to be set to start detection
	while(PB12bitstatus != GPIO_PIN_SET){
			PB12bitstatus = HAL_GPIO_ReadPin(SYNTH_IN_GPIO_Port,SYNTH_IN_Pin);
		}
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)buffer, BUF_LEN);
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);	// Turns LED ON when detection starts
	while(firstHalfFull == false);
	fill = 1;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (finishedSampling == false) {

		if (fill == 1) {
			firstHalfFull = false;
			// Convert first half of array to float type
			array_conversion(&buffer[0], unfiltered_ecg.signal, BUF_LEN_HALF);
		} else if (fill == 2) {
			secondHalfFull = false;
			// Convert second half of array to float type
			array_conversion(&buffer[BUF_LEN_HALF], unfiltered_ecg.signal, BUF_LEN_HALF);
		}
		// ------------------------------------------------------------------//
		// --------------------------FILTERING-------------------------------//

		// Filter using b1 filter
		statefloatfilter(&unfiltered_ecg, &filtered_ecg_mid, b1_filter);

		// Filter using b2 filter for Christov
		statefloatfilter(&filtered_ecg_mid, &filtered_ecg_C, b2_filter);

		// Filter using b2 filter for Engzee
		statefloatfilter(&filtered_ecg_mid, &filtered_ecg_E, b2_filter);
		// ------------------------------------------------------------------//
		// -----------------------DIFFERENTIATION----------------------------//
		// Differentiate according to Christov
		christov_differentiation(&filtered_ecg_C, &diff_C);

		// Differentiate according to Engzee
		engzee_differentiation(&filtered_ecg_E, &diff_E);
		// ------------------------------------------------------------------//
		// -----------------------NOISE FILTERING----------------------------//
		// Filters noise of both differentiated signals
		statefloatfilter(&diff_C, &diff_filtered_C, b_noise);
		statefloatfilter(&diff_E, &diff_filtered_E, b_noise);
		// ------------------------------------------------------------------//
		// -----------------------BEAT DETECTION-----------------------------//
		engzee_lourenco(&unfiltered_ecg, &diff_filtered_E, &engzee_state);
		christov(&diff_filtered_C, &christov_state);

		// Waits for more data
		while((firstHalfFull || secondHalfFull || lastBuffer) == false);

		// Checks if detection is done
		PB12bitstatus = HAL_GPIO_ReadPin(SYNTH_IN_GPIO_Port,SYNTH_IN_Pin);		// If SYNTH_IN_Pin = GPIO_PIN_RESET means no more data will be sent
		if ((PB12bitstatus != GPIO_PIN_SET) && !lastBuffer) {lastBuffer = true;}					// 	(but one more half buffer will still be processed to ensure nothing was missed)
		else if (lastBuffer == true) {finishedSampling = true;}

		if (!finishedSampling) {
			if ((fill == 1 && firstHalfFull) ||
				(fill == 2 && secondHalfFull)) {
				notEnoughTimeError();
			} else if (fill == 1 && secondHalfFull) {
				fill = 2;
			} else if (fill == 2 && firstHalfFull) {
				fill = 1;
			}
		}

		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		/* USER CODE END 3 */
	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	// Combines detections from both algorithms
	tradeoff(&engzee_state,&christov_state, &final_detect);
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);	// Turns LED off when done sampling
	HAL_ADC_Stop_DMA(&hadc1);
	return 0;
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 100-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB_Pin */
  GPIO_InitStruct.Pin = PB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(PB_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SYNTH_IN_Pin */
  GPIO_InitStruct.Pin = SYNTH_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(SYNTH_IN_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
//	t_1 = DWT->CYCCNT;
//	buffer_1250_0 = buffer[1249];
//	buffer_1251_0 = buffer[1250];
	firstHalfFull = true;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
//	t_2 = DWT->CYCCNT;
//	buffer_1250_1 = buffer[1249];
//	buffer_1251_1 = buffer[1250];
	secondHalfFull = true;
}

int _write(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		ITM_SendChar(ptr[DataIdx]);  // Envia caractere por ITM
	}
	return len;
}

/**
  * @brief  This function is executed in case the circular buffer is starting to be overwritten but processing is not yet over.
  * @retval None
  */
void notEnoughTimeError(void){
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);	// Turns LED off when done sampling
	HAL_Delay(200);
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);	// Turns LED off when done sampling
	HAL_Delay(200);
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
