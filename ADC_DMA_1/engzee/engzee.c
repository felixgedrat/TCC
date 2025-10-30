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
 * @brief Engzee Detection
 * @input Digitized input, Engzee differentiated array, sample buffer index, MM and Thi_list
 * @output Spikes detected - Engzee
 */
void engzee_lourenco(Signal* unfiltered_ecg, Signal* MA3, EngzeeState* state){
	int local_i;
	uint32_t last_QRS;
	uint32_t maxi = 0;

	// --------- Zero out filter delay --------- //
	if (state->i_global == 0) {
		memset(&(MA3->signal[0]),0,TOTAL_TAPS);
	}

	// ------------ Detection loop ------------ //
	last_QRS = state->QRS[state->qrs_index - 1];
	for (local_i = 0; local_i < BUF_LEN_HALF; local_i++) {
		// Updates last_QRS and adds unfiltered_section value
		state->unfiltered_section[state->section_index++] = unfiltered_ecg->signal[local_i];
		//------------------------- AQUI EH PARA ENCONTRAR M -----------------------------
		if (state->i_global < 5 * state->fs) {

			state->M = 0.6 * max(MA3->signal, local_i);
			state->MM_size = append5(state->MM,state->MM_size,state->M);
		}
		//------------------------------ELIF 1-----------------------------------------------
		else if (state->qrs_index && state->i_global < last_QRS + ms200) {
			//if (state->QRS[state->qrs_index - 1] < start){
			state->newM5 = 0.6 * max2(MA3->signal, 0, local_i);		// note: aqui deveriamos detectar da ultima amostra em QRS ate o valor atual, descontinuidade?
//			}
//			else{												// note: entender o uso desse i-start no acesso ao array
//				if ((i - start) - state->QRS[state->qrs_index - 1]){
//					state->newM5 = 0;
//					}
//				else{
//					state->newM5 = 0.6 * max2(MA3, state->QRS[state->qrs_index - 1], (i - start));
//
//				}
//			}
			if (state->newM5 > 1.5 * state->MM[4]) {
				state->newM5 = 1.1 * state->MM[4];
			}
		}
		//------------------------------ELIF 2-----------------------------------------------
		else if (state->newM5 != 0 && state->qrs_index && state->i_global == last_QRS + ms200) {
			state->MM_size = append5(state->MM,state->MM_size,state->newM5);
			state->M = mean(state->MM,state->MM_size);
		}
		//------------------------------ELIF 3-------------------------------------------
		else if (state->qrs_index && state->i_global > last_QRS + ms200 && state->i_global < last_QRS + ms1200) {
			state->M = (mean(state->MM,state->MM_size)) * state->M_slope[state->i_global - (last_QRS + ms200)];
		}
		//------------------------------ELIF 4-------------------------------------------
		else if (state->qrs_index && state->i_global > last_QRS + ms1200) {
			state->M = 0.6 * mean(state->MM,state->MM_size);
		}
		//----------------------------- DETECTION ----------------------------------------
		if (!(state->qrs_index) && MA3->signal[local_i] > state->M) {
			state->QRS[state->qrs_index++] = state->i_global;
			last_QRS = state->i_global;
//			state->thi_list[state->qrs_index] = state->i_global;														// note: thi_list pode ser definido dentro de engzee (n precisa de historico)
			state->thi = state->i_global;
			// Updates unfiltered section
			state->unfiltered_section[0] = state->unfiltered_section[state->section_index-3];
			state->unfiltered_section[1] = state->unfiltered_section[state->section_index-2];
			state->unfiltered_section[2] = state->unfiltered_section[state->section_index-1];
			state->section_index = 3;

		} else if (state->qrs_index && state->i_global > last_QRS + ms200 && MA3->signal[local_i] > state->M) {
			state->QRS[state->qrs_index++] = state->i_global;
			last_QRS = state->i_global;
//			state->thi_list[state->qrs_index] = state->i_global;
			state->thi = state->i_global;
//			state->qrs_index++;
			state->unfiltered_section[0] = state->unfiltered_section[state->section_index-3];
			state->unfiltered_section[1] = state->unfiltered_section[state->section_index-2];
			state->unfiltered_section[2] = state->unfiltered_section[state->section_index-1];
			state->section_index = 3;
		}
		//------------------------------- THI e THF -------------------------------------
		if (state->thi && state->i_global < state->thi + ms160) {
			if (MA3->signal[local_i] < -(state->M) && MA3->signal[local_i-1] > -(state->M)) {
				state->thf = 1;
			}
			if (state->thf && MA3->signal[local_i] < -(state->M)) {
				state->counter++;
			} else if (MA3->signal[local_i] > -(state->M) && state->thf) {
				state->counter = 0;
				state->thi = 0;
				state->thf = 0;
			}
		} else if (state->thi && state->i_global > state->thi + ms160) {
			state->counter = 0;
			state->thi = 0;
			state->thf = 0;
		}
		//-------------------------- FIND ACTUAL PEAKS ---------------------------
		if (state->counter > neg_threshold) {
//			for (int k = state->thi_list[state->qrs_index - 1] - 2; k < i; k++) {
//				state->unfiltered_section[state->section_index] = unfiltered_ecg->signal[k];		// note: me parece estranho
//				state->section_index++;
//			}
			maxi = indexMax(state->unfiltered_section, max_section_size);

			state->r_peaks[state->peaks_index++] = maxi + last_QRS - neg_threshold;
			state->counter = 0;
			state->thi = 0;
			state->thf = 0;
		}
		state->i_global++;
	}
//	if (first == 0){
//		for (int l = 0; l < (*state->peaks_index); l++) {
//			state->r_peaks[l] = state->r_peaks[l + 1];
//		}
//		first++;
//		(*state->peaks_index)--;
//	}

}

