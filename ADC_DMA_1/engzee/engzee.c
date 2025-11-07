/*
 * engzee.c
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
#include "engzee.h"
#include "main.h"


/**
 * @brief Engzee Differentiation
 * @input values ​​after filtering
 * @output differentiated values ​​- Engzee
 */
void engzee_differentiation(struct Signal *input, struct Signal *diff_E) {
// Create and fill array of input signal with state
	uint16_t len_signal_state = input->len_signal + input->len_state;
	uint16_t signal_with_state[len_signal_state];
	uint16_t engzee_diff_start = input->len_state;
	uint16_t i=0;
	for (i=0;i<input->len_state;i++){
		signal_with_state[i] = input->state[i];
	}
	for (i=input->len_state;i<len_signal_state;i++){
		signal_with_state[i] = input->signal[i-input->len_state];
	}
	// Evaluate difference array according to engzee rules
	uint16_t count = 0;
	for (i=engzee_diff_start; i < len_signal_state; i++) {
		diff_E->signal[count++] = signal_with_state[i] - signal_with_state[i - engzee_diff_start];
	}
	// Update input's state
	for (i=0;i<input->len_state;i++){
		input->state[i]=signal_with_state[len_signal_state-input->len_state+i];
	}
}

/**
 * @brief Updates unfiltered section array in Engzee
 * @input Digitized input, Engzee differentiated array, sample buffer index, MM and Thi_list
 * @output Spikes detected - Engzee
 */
void update_unfiltered_section(float* unfiltered_section, uint16_t* len_unfiltered_section) {
	unfiltered_section[0] = unfiltered_section[(*len_unfiltered_section)-3];
	unfiltered_section[1] = unfiltered_section[(*len_unfiltered_section)-2];
	unfiltered_section[2] = unfiltered_section[(*len_unfiltered_section)-1];
	*len_unfiltered_section = 3;
}


/**
 * @brief Engzee Detection
 * @input Digitized input, Engzee differentiated array, sample buffer index, MM and Thi_list
 * @output Spikes detected - Engzee
 */
void engzee_lourenco(Signal* unfiltered_ecg, Signal* MA3, EngzeeState* state){
	uint16_t local_i;
	uint32_t last_QRS = 0;
	uint32_t maxi = 0;
	uint32_t five_seconds = (uint32_t)5*state->fs;

	if (state->len_QRS > 0) last_QRS = state->QRS[state->len_QRS - 1];
	// --------- Zero out filter delay --------- //
	if (state->i_global == 0) {
		memset(MA3->signal,0,TOTAL_TAPS*sizeof(float));
	}

	// ------------ Detection loop ------------ //
	for (local_i = 0; local_i < BUF_LEN_HALF; local_i++) {
		// Updates last_QRS and adds unfiltered_section value
		state->unfiltered_section[state->len_unfiltered_section++] = unfiltered_ecg->signal[local_i];
		//////////////////////////////////////////////////
		// M threshold
		//------------------------------START IF-------------------------------------------
		if (state->i_global < five_seconds) {
			state->M = 0.6 * max(MA3->signal, local_i);
			state->MM_size = append5(state->MM,state->MM_size,state->M);
		}
		//------------------------------ELIF 1-----------------------------------------------
		else if (state->len_QRS && state->i_global < last_QRS + ms200) {
			state->newM5 = 0.6*max(state->M_section,state->len_M_section);
			if (state->newM5 > 1.5 * state->MM[state->MM_size-1]) {
				state->newM5 = 1.1 * state->MM[state->MM_size-1];
			}
		}
		//------------------------------ELIF 2-----------------------------------------------
		else if (state->newM5 != 0 && state->len_QRS && state->i_global == last_QRS + ms200) {
			state->MM_size = append5(state->MM,state->MM_size,state->newM5);
			state->M = mean(state->MM,state->MM_size);
		}
		//------------------------------ELIF 3-------------------------------------------
		else if (state->len_QRS && state->i_global > last_QRS + ms200 && state->i_global < last_QRS + ms1200) {
			state->M = (mean(state->MM,state->MM_size)) * state->M_slope[state->i_global - (last_QRS + ms200)];
		}
		//------------------------------ELIF 4-------------------------------------------
		else if (state->len_QRS && state->i_global > last_QRS + ms1200) {
			state->M = 0.6 * mean(state->MM,state->MM_size);
		}
		//----------------------------- DETECTION ----------------------------------------

		// First detection
		if (!(state->len_QRS) && MA3->signal[local_i] > state->M) {
			state->QRS[state->len_QRS++] = state->i_global;
			last_QRS = state->i_global;
			state->thi = true;
			update_unfiltered_section(state->unfiltered_section,&(state->len_unfiltered_section));
			state->len_M_section = 0;
		// Other detections
		} else if (state->len_QRS && (state->i_global > last_QRS + ms200) && (MA3->signal[local_i] > state->M)) {
			state->QRS[state->len_QRS++] = state->i_global;
			last_QRS = state->i_global;
			state->thi = true;
			update_unfiltered_section(state->unfiltered_section,&(state->len_unfiltered_section));
			state->len_M_section = 0;
		}
		// Updates M_section if necessary
		if (state->len_QRS) state->M_section[state->len_M_section++] = MA3->signal[local_i];
		//------------------------------- THI e THF -------------------------------------
		if (state->thi && (state->i_global < last_QRS + ms160)) {
			if ((MA3->signal[local_i] < -(state->M)) && (MA3->signal[local_i-1] > -(state->M))) {
				state->thf = true;
			}
			if (state->thf && (MA3->signal[local_i] < -(state->M))) {
				state->counter++;
			} else if (MA3->signal[local_i] > -(state->M) && state->thf) {
				state->counter = 0;
				state->thi = false;
				state->thf = false;
			}
		} else if (state->thi && (state->i_global > last_QRS + ms160)) {
			state->counter = 0;
			state->thi = false;
			state->thf = false;
		}
		//-------------------------- FIND ACTUAL PEAKS ---------------------------
		if (state->counter > NEG_THRESHOLD) {
			maxi = indexMax(state->unfiltered_section, state->len_unfiltered_section);

			state->r_peaks[state->len_r_peaks++] = maxi + last_QRS - NEG_THRESHOLD;
			state->counter = 0;
			state->thi = false;
			state->thf = false;
		}
		state->i_global++;
	}
}

