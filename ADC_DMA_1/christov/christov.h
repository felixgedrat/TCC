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
#include "main.h"

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

void christov_differentiation(struct Signal *input, struct Signal *diff_C);
void christov_noise(struct Signal *diff_signal, struct Signal *diff_filtered_signal);
void christov(float* MA3, int sample, ChristovState* state);


#endif /* CHRISTOV_H_ */
