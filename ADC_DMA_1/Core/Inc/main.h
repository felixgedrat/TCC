/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define BUF_LEN 2500
#define BUF_LEN_HALF 1250
#define MAX_SECTION 25		// length of unfiltered section
#define THI_LIST_SIZE 320
#define M_SLOPE_SIZE 250
#define MAX_QRS 320
#define MAX_R_PEAKS 320

typedef struct {
    float M;
    float MM[5];
//    int mm_count;

    int QRS[MAX_QRS];
    int qrs_index;

    int thi_list[THI_LIST_SIZE];
    int thi;

    int thf;
    int counter;

    float newM5;

    int unfiltered_section[MAX_SECTION];
    int section_index;
    int maxi;

    float M_slope[M_SLOPE_SIZE];  // para fs = 250Hz

    int i_global;
    uint8_t fs;	// sampling frequency

    int r_peaks[MAX_R_PEAKS];
    int* peaks_index;

    int len_engzee;
} EngzeeState;

typedef struct {
    float M;
    float MM[5];
    int mm_count;

    float newM5;

    int QRS[MAX_QRS];
    int qrs_index;

    float F;

    int R;
    int RR[5];
    int rr_index;
    int Rm;

    float MFR;

    float M_slope[250];

    int fs;

    int len_detection;
} ChristovState;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define PB_Pin GPIO_PIN_0
#define PB_GPIO_Port GPIOA
#define SYNTH_IN_Pin GPIO_PIN_12
#define SYNTH_IN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
