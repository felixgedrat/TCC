/*
 * christov.c
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
#include "christov.h"
#include "main.h"

/**
 * @brief Christov Differentiation
 * @input values ​​after filtering, sample size
 * @output differentiated values ​​- Christov
 */
void christov_differentiation(struct Signal *input, struct Signal *diff_C) {
	// Create and fill array of input signal with state
	uint16_t len_signal_state = input->len_signal + input->len_state;
	float signal_with_state[len_signal_state];
	uint16_t i=0;
	for (i=0;i<input->len_state;i++){
		signal_with_state[i] = input->state[i];
	}
	for (i=input->len_state;i<len_signal_state;i++){
		signal_with_state[i] = input->signal[i-input->len_state];
	}
	// Evaluate the difference array
	uint16_t count = 0;
	for (i = 1; i < len_signal_state - 1; i++) {
		diff_C->signal[count++] = fabs(signal_with_state[i + 1] - signal_with_state[i - 1]);
	}
	// Update input's state
	for (i=0;i<input->len_state;i++){
		input->state[i]=signal_with_state[len_signal_state-input->len_state+i];
	}
}


/**
 * @brief Christov Detection
 * @input Filtered input and current state of detection
 * @output Spikes detected - Christov
 */
void christov(Signal* MA3, ChristovState* state){
	uint32_t local_i;
	uint32_t last_QRS;
	float max_latest;
	float max_earliest;
	last_QRS = state->QRS[state->len_QRS - 1];

	// --------- Zero out filter delay --------- //
	if (state->i_global == 0) {
		memset(MA3->signal,0,TOTAL_TAPS*sizeof(float));
	}

	// ------------ Detection loop ------------ //
	for (local_i = 0; local_i < BUF_LEN_HALF; local_i++) {
		//////////////////////////////////////////////////
		// M threshold
		if (state->i_global < 5 * state->fs) {
			state->M = 0.6 * max(MA3->signal, local_i);
			state->MM_size = append5(state->MM,state->MM_size,state->M);
		}
		else if (state->len_QRS && state->i_global < last_QRS + ms200) {
			state->newM5 = 0.6*max(state->M_section,state->len_M_section);
			if (state->newM5 > 1.5 * state->MM[state->MM_size-1]) {
				state->newM5 = 1.1 * state->MM[state->MM_size-1];
			}
		}
		else if (state->len_QRS && state->i_global == last_QRS + ms200) {
			if (state->newM5 == 0) {
				state->newM5 = state->MM[state->MM_size-1];
			}
			state->MM_size = append5(state->MM,state->MM_size,state->newM5);
			state->M = mean(state->MM,state->MM_size);
		}
		else if (state->len_QRS && (state->i_global > last_QRS + ms200) && (state->i_global < last_QRS + ms1200)) {
			state->M = mean(state->MM,state->MM_size) * state->M_slope[state->i_global - last_QRS + ms200];
		}
		else if (state->len_QRS && last_QRS + ms1200) {
			state->M = 0.6 * (mean(state->MM,state->MM_size));
		}

		if (state->len_QRS) {
			state->M_section[state->len_M_section++] = MA3->signal[local_i];
		}

		//////////////////////////////////////////////////
		// F threshold

		if ((state->i_global) > ms350) {
			max_earliest = maxStartEnd(state->F_section,0,ms50);
			max_latest = maxStartEnd(state->F_section,ms350-ms50,ms350);
			state->F = state->F + ((max_latest - max_earliest) / 150.0);
		}

		//////////////////////////////////////////////////
		// R threshold

		if (state->len_QRS && state->i_global < last_QRS + (int)(2.0 / 3.0 * ((float)state->Rm))) {
			state->R = 0;
		} else if (state->len_QRS && state->i_global > last_QRS+ (int)(2.0 / 3.0 * ((float)state->Rm)) && state->i_global < last_QRS + state->Rm) {
			state->R = (state->M - mean(state->MM,state->MM_size)) / 1.4;
		}

		//////////////////////////////////////////////////
		// MFR threshold

		state->MFR = state->M + state->F + state->R;

		//////////////////////////////////////////////////
		// Detection

		// First detection
		if (!(state->len_QRS) && MA3->signal[local_i] > state->MFR) {
			state->QRS[state->len_QRS++] = state->i_global;
			last_QRS = state->i_global;
			state->len_M_section = 0;

		// Other detections
		} else if (state->len_QRS && state->i_global > last_QRS + ms200 && MA3->signal[local_i] > state->MFR) {
			state->QRS[state->len_QRS++] = state->i_global;	// Stores detection
			last_QRS = state->i_global;						// updates last QRS
			state->len_M_section = 0;						// zeroes out M section
			if (state->len_QRS > 2) {
				uint32_t RR_add = state->QRS[state->len_QRS-1] - state->QRS[state->len_QRS-2];
				state->rr_index = append5(state->RR,state->rr_index,RR_add);
				state->Rm = (uint32_t)mean(state->RR,state->rr_index);
			}

		}
		state->len_F_section = append_ms350(state->F_section,state->len_F_section,MA3->signal[local_i]);
		state->i_global++;
	}

	// the lines below do the functionality of a pop(0) operation in Python
	for (uint32_t l = 0; l < state->len_QRS; l++) {
		state->QRS[l] = state->QRS[l + 1];
	}
	state->len_QRS--;

}
