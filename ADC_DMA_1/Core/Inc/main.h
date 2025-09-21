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
#include "prefiltering.h"
#include "christov.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define BUF_LEN 2500
#define BUF_LEN_HALF 1250
#define BUF_LEN_HALF_CHRISTOV (1250-2)
#define PREFILTERING_HISTORY 7
#define MAX_SECTION 1000		// length of unfiltered section
#define THI_LIST_SIZE 320
#define M_SLOPE_SIZE 250
#define MAX_QRS 320
#define MAX_R_PEAKS 320
#define MAX_DETECTION_INIT 320
#define MAX_DETECTION_FINAL 500

// FILTER MACROS
#define MAX_FILTER_ORDER 20 // maximum allowed filter order
#define FILTER_B1_ORDER 4
#define FILTER_B2_ORDER 6
#define FILTER_B_NOISE_ORDER 9
#define TOTAL_TAPS (FILTER_B1_ORDER+1+FILTER_B2_ORDER+1+FILTER_B_NOISE_ORDER+1)
#define DIFFERENCE_CHRISTOV_STATE 2
#define DIFFERENCE_ENGZEE_STATE 4

// Struct for Engzee detection
typedef struct {
    float M;
    float MM[5];
    uint16_t MM_size;
//    int mm_count;

    uint32_t QRS[MAX_QRS];
    uint32_t qrs_index;

//    int thi_list[THI_LIST_SIZE];
    uint32_t thi;

    uint32_t thf;
    int counter;

    float newM5;

    int unfiltered_section[MAX_SECTION];
    int section_index;
    int maxi;

    float M_slope[M_SLOPE_SIZE];  // para fs = 250Hz

    uint32_t i_global;
    uint8_t fs;	// sampling frequency

    uint16_t r_peaks[MAX_R_PEAKS];
    uint16_t peaks_index;

    int len_engzee;
} EngzeeState;

// Struct for Christov detection
typedef struct {
    float M;
    float MM[5];
    uint16_t MM_size;

    float newM5;

    uint32_t QRS[MAX_QRS];
    uint32_t qrs_index;

    float F;

    float R;
    float RR[5];
    int rr_index;
    float Rm;

    float MFR;

    float M_slope[250];

    int fs;

    uint32_t i_global;
    int len_detection;

    float M_section[MAX_SECTION];
    uint32_t M_section_index;

    float F_section[ms350];
    uint16_t F_section_index;
} ChristovState;

// Struct for global detection and parameters
typedef struct {
	uint32_t buffer[BUF_LEN];
	uint32_t buffer_history[FILTER_B1_ORDER];

	int detections[MAX_DETECTION_FINAL];
	int len_detections;
} GlobalState;

// Struct for state signal processing
typedef struct Signal {
	float signal[BUF_LEN_HALF];
	float state[MAX_FILTER_ORDER];
	uint16_t len_signal;
	uint16_t len_state;
}Signal;

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
