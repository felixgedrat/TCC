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
 * @brief Call lfilter to diff_signals
 * @input differentiated values
 * @output filtered values
 */
//void christov_noise(float *diff_signal, float *diff_filtered_signal, uint16_t total_taps, int length) {
void christov_noise(struct Signal *diff_signal, struct Signal *diff_filtered_signal) {
	//uint16_t filter_size = FILTER_B_NOISE_ORDER + 1;
	float b[] = { 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1 };
//	uint16_t a[] = {1};
//	uint16_t len_b = sizeof(b) / sizeof(b[0]);
//	uint16_t len_a = sizeof(a) / sizeof(a[0]);
	uint16_t total_taps = (FILTER_B1_ORDER + 1) +
						  (FILTER_B2_ORDER + 1) +
						  (FILTER_B_NOISE_ORDER + 1);

//	floatfilter(b, a, len_b, len_a, diff_signal, diff_filtered_signal, length);
	statefloatfilter(diff_signal,diff_filtered_signal,b);
	for (int i = 0; i < total_taps; i++) {
		diff_filtered_signal->signal[i] = 0;
	}
}


/**
 * @brief Christov Detection
 * @input Digitized input, Christov differentiated array, sample buffer index, MM and RR
 * @output Spikes detected - Christov
 */
//void christov(float* MA3, int sample, ChristovState* state){ // int fs, int* QRS, int *len_detection, float *MM, float *RR, int *R_idx) {
void christov(Signal* MA3, ChristovState* state){
	uint32_t local_i;
	uint32_t last_QRS;
	float F_section_latest[ms50];
	float F_section_earliest[ms50];
	float max_latest;
	float max_earliest;
	last_QRS = state->QRS[state->qrs_index - 1];

	// --------- Zero out filter delay --------- //
	if (state->i_global == 0) {
		memset(&(MA3->signal[0]),0,TOTAL_TAPS);
	}

	// ------------ Detection loop ------------ //
	for (local_i = 0; local_i < BUF_LEN_HALF; local_i++) {
		//////////////////////////////////////////////////
		// M threshold
		if (state->i_global < 5 * state->fs) {
			state->M = 0.6 * max(MA3->signal, local_i);
			state->MM_size = append5(state->MM,state->MM_size,state->M);
		}
		else if (state->qrs_index && state->i_global < last_QRS + ms200) {
			state->newM5 = 0.6*max(state->M_section,state->M_section_index);
			if (state->newM5 > 1.5 * state->MM[state->MM_size-1]) {
				state->newM5 = 1.1 * state->MM[state->MM_size-1];
			}
		}
		else if (state->qrs_index && state->i_global == last_QRS + ms200) {
			if (state->newM5 == 0) {
				state->newM5 = state->MM[state->MM_size-1];
			}
			state->MM_size = append5(state->MM,state->MM_size,state->newM5);
			state->M = mean(state->MM,state->MM_size);
		}
		else if (state->qrs_index && (state->i_global > last_QRS + ms200) && (state->i_global < last_QRS + ms1200)) {
			state->M = mean(state->MM,state->MM_size) * state->M_slope[state->i_global - last_QRS + ms200];
		}
		else if (state->qrs_index && last_QRS + ms1200) {
			state->M = 0.6 * (mean(state->MM,state->MM_size));
		}

		if (state->qrs_index) {
			state->M_section[state->M_section_index++] = MA3->signal[local_i];
		}

		//////////////////////////////////////////////////
		// F threshold
		if ((state->i_global) > ms350) {
//			if (F_section == NULL) {
//				exit(1);
//			}
			max_earliest = max2(state->F_section,0,ms50);
			max_latest = max2(state->F_section,ms350-ms50,ms350);
			state->F = state->F + ((max_latest - max_earliest) / 150.0);

		}
		//////////////////////////////////////////////////
		// R threshold

		if (state->qrs_index && state->i_global < last_QRS + (int)(2.0 / 3.0 * state->Rm)) {
			state->R = 0;
		} else if (state->qrs_index && state->i_global > last_QRS+ (int)(2.0 / 3.0 * state->Rm) && state->i_global < last_QRS + state->Rm) {
			state->R = (state->M - mean(state->MM,state->MM_size)) / 1.4;
		}

		//////////////////////////////////////////////////
		// MFR threshold

		state->MFR = state->M + state->F + state->R;

		if (!(state->qrs_index) && MA3->signal[local_i] > state->MFR) {
			state->QRS[state->qrs_index++] = state->i_global;
			last_QRS = state->i_global;
			state->M_section_index = 0;
		} else if (state->qrs_index && state->i_global > last_QRS + ms200 && MA3->signal[local_i] > state->MFR) {
			state->QRS[state->qrs_index++] = state->i_global;
			last_QRS = state->i_global;
			state->M_section_index = 0;
			if (state->qrs_index > 2) {
				uint32_t RR_add = state->QRS[state->qrs_index-1] - state->QRS[state->qrs_index-2];
				state->rr_index = append5(state->RR,state->rr_index,RR_add);
				state->Rm = mean(state->RR,state->rr_index);
			}

		}
		state->F_section_index = append_ms350(state->F_section,state->F_section_index,MA3->signal[local_i]);
		state->i_global++;
	}

//	free(F_section);

	// the lines below do the functionality of a pop(0) operation in Python
	for (int l = state->len_detection; l < state->qrs_index; l++) {
		state->QRS[l] = state->QRS[l + 1];
	}
	state->qrs_index--;

//	*len_detection = qrs_index;
//	*R_idx = idx;
}
