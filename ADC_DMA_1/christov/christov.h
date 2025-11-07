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
#define ms4000 1000
#define ms10000 2500
#define max_qrs_size 320
#define max_section_size 25
#define BUF_LEN_HALF 1250
#define MAX_FILTER_ORDER 20 // maximum allowed filter order
#define MAX_QRS 1000
#define M_SLOPE_SIZE 250
/* USER CODE END PM */


/* private types -------------------------------------------------------------*/
// Struct for state signal processing
typedef struct Signal{
	float signal[BUF_LEN_HALF];		// signal
	float state[MAX_FILTER_ORDER];	// current state of signal
	uint16_t len_signal;			// length of signal
	uint16_t len_state;				// length of state
}Signal;

// Struct for Christov detection
typedef struct ChristovState{
	// M-threshold parameters
    float M;						// current M parameter
    float MM[5];					// M buffer, max size 5
    uint16_t MM_size;				// size of M buffer
    float newM5;					// new value to be added to buffer
    float M_section[ms10000];		// stores signal values since last detection
    uint32_t len_M_section;			// size of M section
    float M_slope[M_SLOPE_SIZE];	// slope used for M parameter

    // F-threshold
    float F;						// F threshold
    float F_section[ms350];			// buffer which stores last 350 ms of signal
    uint16_t len_F_section;			// length of F section

    // R-threshold parameters
    float R;						// Current R parameter
    float RR[5];					// R buffer, max size 5
    uint8_t RR_size;				// size of R buffer
    uint32_t Rm;					// mean integer of RR buffer

    // MFR threshold
    float MFR;

    // Constants
    uint8_t fs;							// sampling frequency

    // Detections
    uint32_t QRS[MAX_QRS];			// detection array
    uint32_t len_QRS;				// length of detection array

    // Absolute instant
    uint32_t i_global;
} ChristovState;

/* PFP -------------------------------------------------------------*/
void christov_differentiation( Signal *input,  Signal *diff_C);
void christov_noise( Signal *diff_signal,  Signal *diff_filtered_signal);
void christov(Signal* MA3,  ChristovState* state);


#endif /* CHRISTOV_H_ */
