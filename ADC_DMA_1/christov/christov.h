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
#define BUF_LEN_HALF 1250
#define MAX_FILTER_ORDER 20 // maximum allowed filter order
#define MAX_SECTION 1000		// length of unfiltered section
#define MAX_QRS 320
/* USER CODE END PM */


// Struct for state signal processing
typedef struct Signal{
	float signal[BUF_LEN_HALF];
	float state[MAX_FILTER_ORDER];
	uint16_t len_signal;
	uint16_t len_state;
}Signal;

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
//struct Signal;
//struct ChristovState;
void christov_differentiation( Signal *input,  Signal *diff_C);
void christov_noise( Signal *diff_signal,  Signal *diff_filtered_signal);
void christov(Signal* MA3,  ChristovState* state);


#endif /* CHRISTOV_H_ */
