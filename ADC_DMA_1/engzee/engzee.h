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
#define NEG_THRESHOLD 2
#define MAX_QRS_SIZE 320
#define MAX_SECTION_SIZE 25
/* USER CODE END PM */

// Struct for Engzee detection
typedef struct EngzeeState{
	// M-threshold parameters
	float M;								// M threshold
    float MM[5];							// M buffer, max size 5
    uint16_t MM_size;						// M buffer length
    float newM5;							// potential new element of M buffer
    float M_slope[M_SLOPE_SIZE];  // slope used for M parameter

    // Detection window parameters
    bool thi;								// signals detection
    bool thf;								// signals sudden change
    uint8_t counter;						// number of instants in window
    float unfiltered_section[ms4000];		// section of unfiltered ecg signal updated every time a new beat is detected
    uint16_t len_unfiltered_section;		// length of unfiltered section

    // Constants
    uint8_t fs;	// sampling frequency

    // R peaks detected in filtered signal
    uint32_t QRS[MAX_QRS];
    uint32_t len_QRS;

    // true R peaks (detected in unfiltered section)
    uint32_t r_peaks[MAX_R_PEAKS];
    uint16_t len_r_peaks;

    // Absolute instant
    uint32_t i_global;
} EngzeeState;

void engzee_differentiation(struct Signal *input, struct Signal *diff_E);
void engzee_lourenco(Signal* unfiltered_ecg, Signal* MA3, EngzeeState* state);

#endif /* ENGZEE_H_ */
