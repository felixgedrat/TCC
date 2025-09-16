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
//	uint16_t length = BUF_LEN_HALF;
// Create and fill array of input signal with state
	uint16_t len_signal_state = input->len_signal + input->len_state;
	uint16_t signal_with_state[len_signal_state];
	uint16_t engzee_diff_start = DIFFERENCE_ENGZEE_STATE;
	int i=0;
	for (i=0;i<input->len_signal;i++){
		signal_with_state[i] = input->state[i];
	}
	for (i=input->len_signal;i<len_signal_state;i++){
		signal_with_state[i] = input->signal[i];
	}
	// Evaluate difference array according to engzee rules
	int count = 0;
	for (i=engzee_diff_start; i < len_signal_state; i++) {
		diff_E->signal[count++] = input->signal[i] - input->signal[i - engzee_diff_start];
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
void engzee_lourenco(Signal* unfiltered_ecg, Signal* diff_E, int sample, EngzeeState* state){ //int *r_peaks, int *peaks_index, float *MM, int *thi_list) {	// note: mudar nome de diff_E pq esse eh diferenciado e filtrado
//	float M_slope[250];
//	state->M = 0;
//	int QRS[max_qrs_size];																			// note: abarcar em struct
//	int qrs_index = 0;
//	int thi = 0;
//	int counter = 0;
//	int thf = 0;
//	float newM5 = 0;
//	int unfiltered_section[25] = {0};
//	int section_index = 0;
//	int maxi;
	int length = BUF_LEN_HALF;
//	int first = *(state->peaks_index);
	int start = (length * sample);
	int local_i;
	uint32_t last_QRS;
//	float increment = 0.0016064257028112205;

//	for (int j = 0; j < ms1200 - ms200; ++j) {
//		M_slope[j] = 1.0 - j * increment;
//	}

	for (local_i = 0; local_i < BUF_LEN_HALF; local_i++) {

		last_QRS = state->QRS[state->qrs_index - 1];
		//------------------------- AQUI EH PARA ENCONTRAR M -----------------------------
		if (state->i_global < 5 * state->fs) {
			state->M = 0.6 * max(diff_E->signal, local_i + 1);
			if (state->MM_size < 5){
				state->MM[state->MM_size++] = state->M;
			} else if (state->MM_size >= 5) {
				for (int j = 0; j < 4; j++) {
					state->MM[j] = state->MM[j + 1];
				}
				state->MM[4] = state->M;
			}
		}
		//------------------------------ELIF 1-----------------------------------------------
		else if (state->qrs_index && state->i_global < last_QRS + ms200) {
			//if (state->QRS[state->qrs_index - 1] < start){
			state->newM5 = 0.6 * max2(diff_E->signal, 0, local_i);		// note: aqui deveriamos detectar da ultima amostra em QRS ate o valor atual, descontinuidade?
//			}
//			else{												// note: entender o uso desse i-start no acesso ao array
//				if ((i - start) - state->QRS[state->qrs_index - 1]){
//					state->newM5 = 0;
//					}
//				else{
//					state->newM5 = 0.6 * max2(diff_E, state->QRS[state->qrs_index - 1], (i - start));
//
//				}
//			}
			if (state->newM5 > 1.5 * state->MM[4]) {
				state->newM5 = 1.1 * state->MM[4];
			}
		}
		//------------------------------ELIF 2-----------------------------------------------
		else if (state->newM5 != 0 && state->qrs_index && state->i_global == last_QRS + ms200) {
			if (state->MM_size == 5){
				for (int j = 0; j < 4; j++) {
					state->MM[j] = state->MM[j + 1];														// note: criar funcao pop + criar funcao append (quais arrays vao ter malloc?)
				}
				state->MM[4] = state->newM5;
			}
			state->M = mean5(state->MM);
		}
		//------------------------------ELIF 3-------------------------------------------
		else if (state->qrs_index && state->i_global > last_QRS + ms200 && state->i_global < last_QRS + ms1200) {
			state->M = (mean5(state->MM)) * state->M_slope[state->i_global - (state->QRS[state->qrs_index - 1] + ms200)];
		}
		//------------------------------ELIF 4-------------------------------------------
		else if (state->qrs_index && state->i_global > last_QRS + ms1200) {
			state->M = 0.6 * (mean5(state->MM));
		}
		//------------------------------ DETECÇÃO ----------------------------------------
		if (!(state->qrs_index) && diff_E->signal[local_i] > state->M) {
			state->QRS[state->qrs_index++] = state->i_global;
//			state->thi_list[state->qrs_index] = state->i_global;														// note: thi_list pode ser definido dentro de engzee (n precisa de historico)
			state->thi = state->i_global;
		} else if (state->qrs_index && state->i_global > last_QRS + ms200 && diff_E->signal[local_i] > state->M) {
			state->QRS[state->qrs_index++] = state->i_global;
//			state->thi_list[state->qrs_index] = state->i_global;
			state->thi = state->i_global;
//			state->qrs_index++;
		}
		//------------------------------- THI e THF -------------------------------------
		if (state->thi && state->i_global < state->thi + ms160) {
			if (diff_E->signal[local_i] < -(state->M) && diff_E->signal[local_i-1] > -(state->M)) {
				state->thf = 1;
			}
			if (state->thf && diff_E->signal[local_i] < -(state->M)) {
				state->counter++;
			} else if (diff_E->signal[local_i] > -(state->M) && state->thf) {
				state->counter = 0;
				state->thi = 0;
				state->thf = 0;
			}
		} else if (state->thi && state->i_global > state->thi + ms160) {
			state->counter = 0;
			state->thi = 0;
			state->thf = 0;
		}
		//-------------------------- ENCONTRAR OS PICOS DE FATO ---------------------------
		if (state->counter > neg_threshold) {
			for (int k = state->thi_list[state->qrs_index - 1] - 2; k < i; k++) {
				state->unfiltered_section[state->section_index] = unfiltered_ecg->signal[k];		// note: me parece estranho
				state->section_index++;
			}
			int maxi = indexMax(state->unfiltered_section, max_section_size);

			state->r_peaks[*state->peaks_index] = maxi + state->thi_list[state->qrs_index - 1] - neg_threshold;
			(*state->peaks_index)++;
			state->counter = 0;
			state->thi = 0;
			state->thf = 0;
			state->section_index = 0;
			memset(state->unfiltered_section, 0, max_section_size * sizeof(int));		// note: pq zera unfiltered_section?
		}
		state->i_global++;
	}
	if (first == 0){
		for (int l = 0; l < (*state->peaks_index); l++) {
			state->r_peaks[l] = state->r_peaks[l + 1];
		}
		first++;
		(*state->peaks_index)--;
	}

}
