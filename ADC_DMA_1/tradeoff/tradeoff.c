/*
 * tradeoff.c
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "mathematics.h"
#include "prefiltering.h"
#include "main.h"
#include "engzee.h"

//#define MAX_LEN 320
#define MAX_LEN 500
#define MAX_SEARCH_INTERVAL 200
#define DELTA 30	// DELTA = missed beats will be searched in the detections[i]+delta, detections[i+1]-delta section

/**
 * @brief Tradeoff
 * @input Christov and Engzee detections and sizes
 * @output final detections after tradeoff
 */
void tradeoff(EngzeeState* engzee_state, ChristovState* christov_state, FinalDetect* final_detect) {

	// Declare variables
	uint16_t i;																// Iterating index
	uint32_t lower_bound;													// Lower bound for searching
	uint32_t upper_bound;													// Upper bound for searching
	uint8_t missed_beat_len = 0;											// Length of missed beats in search interval
	uint32_t missed_beat[MAX_QRS];      									// Array of missed beats
	uint8_t k;																// Iterating index

	// Removes first detection by both algorithms
	for (i = 0; i < engzee_state->len_r_peaks - 1; i++) {
		engzee_state->r_peaks[i] = engzee_state->r_peaks[i + 1];
	}
	engzee_state->len_r_peaks--;

	for (i = 0; i < christov_state->len_QRS - 1; i++) {
		christov_state->QRS[i] = christov_state->QRS[i + 1];
	}
	christov_state->len_QRS--;


	// Initialize final detection array
	final_detect->len_detections = engzee_state->len_r_peaks; 				// Set initial length of final detection equal to Engzee
	for (i = 0; i < engzee_state->len_r_peaks; i++) {						// Copy Engzee detection array
	        final_detect->detections[i] = engzee_state->r_peaks[i];
	    }


    // Start doing combined analysis
    if (final_detect->len_detections > 2*DELTA) {
    	for (i = 0; i < engzee_state->len_r_peaks - 1; i++) {				// MUST be len_rpeaks to avoid inclusion of just added peaks
    		missed_beat_len = 0;
			lower_bound = final_detect->detections[i] + DELTA;         		// determines upper and lower bounds for searching missed
			upper_bound = final_detect->detections[i + 1] - DELTA;     		//      detections in Christov
				if (upper_bound - lower_bound >= 0) {
					findValuesInRange(christov_state->QRS, christov_state->len_QRS, lower_bound, upper_bound, &missed_beat_len, missed_beat);
					if (missed_beat_len > 0) {
						for (k = 0; k < missed_beat_len; k++) {
							final_detect->detections[(final_detect->len_detections)++] = missed_beat[k];    // add missed detections to array
					}
				}
			}
    	}
    } else {
        for (i = 0; i < christov_state->len_QRS; i++) {
            final_detect->detections[i] = christov_state->QRS[i];
        }
        final_detect->len_detections = christov_state->len_QRS;
    }

    qsort(final_detect->detections, final_detect->len_detections, sizeof(int), compareInts);
}
