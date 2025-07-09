/*
 * christov.h
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#ifndef CHRISTOV_H_
#define CHRISTOV_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mathematics.h"
#include "prefiltering.h"

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define MAX_LENGTH 1000
#define ms50 12
#define ms200 50
#define ms350 87
#define ms1200 300
#define max_qrs_size 320
#define max_section_size 25
/* USER CODE END PM */

void christov_differentiation(float *input, float *diff_C, int length);
void chistov_noise(float *diff_signal, float *diff_filtered_signal, uint16_t total_taps, int length);
void christov(uint16_t* mock_input, float* MA3, int length, int sample, int fs, int* QRS, int *len_detection, float *MM, float *RR, int *R_idx);

#endif /* CHRISTOV_H_ */
