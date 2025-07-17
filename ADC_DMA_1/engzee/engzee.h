/*
 * engzee.h
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#ifndef ENGZEE_H_
#define ENGZEE_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mathematics.h"
#include "prefiltering.h"
#include "christov.h"
#include "main.h"

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define ms200 50
#define ms1200 300
#define ms160 40
#define neg_threshold 2
#define max_qrs_size 320
#define max_section_size 25
/* USER CODE END PM */

void engzee_differentiation(float *input, float *diff_E);
void engzee_lourenco(uint16_t* mock_input, float* diff_E, int sample, EngzeeState* state);

#endif /* ENGZEE_H_ */
