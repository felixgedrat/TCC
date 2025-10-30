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

void engzee_differentiation(struct Signal *input, struct Signal *diff_E);
void engzee_lourenco(Signal* unfiltered_ecg, Signal* MA3, EngzeeState* state);

#endif /* ENGZEE_H_ */
