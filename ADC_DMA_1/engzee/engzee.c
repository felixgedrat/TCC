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
void engzee_differentiation(float *input, float *diff_E) {
	int length = BUF_LEN_HALF;
	for (int i = 0; i < 4; i++){
		diff_E[i] = 0.0;
	}
	for (int i = 4; i < length; i++) {
		diff_E[i] = input[i] - input[i - 4];
	}
}

/**
 * @brief Engzee Detection
 * @input Digitized input, Engzee differentiated array, sample buffer index, MM and Thi_list
 * @output Spikes detected - Engzee
 */
void engzee_lourenco(uint16_t* mock_input, float* diff_E, int sample, EngzeeState* state){ //int *r_peaks, int *peaks_index, float *MM, int *thi_list) {	// note: mudar nome de diff_E pq esse eh diferenciado e filtrado
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
	int first = *(state->peaks_index);
	int start = (length * sample);

//	float increment = 0.0016064257028112205;

//	for (int j = 0; j < ms1200 - ms200; ++j) {
//		M_slope[j] = 1.0 - j * increment;
//	}

	for (int i = start; i < (length * (sample + 1)); i++) {
		//------------------------- AQUI EH PARA ENCONTRAR M -----------------------------
		if (i < 5 * state->fs) {
			state->M = 0.6 * max(diff_E, i + 1);
			if (i < 5){
				state->MM[i] = state->M;
			} else if (i >= 5) {
				for (int j = 0; j < 4; j++) {
					state->MM[j] = state->MM[j + 1];
				}
				state->MM[4] = state->M;
			}
		}
		//------------------------------ELIF 1-----------------------------------------------
		else if (state->qrs_index && i < state->QRS[state->qrs_index - 1] + ms200) {
			if (state->QRS[state->qrs_index - 1] < start){
				state->newM5 = 0.6 * max2(diff_E, 0, (i - start));		// note: aqui deveriamos detectar da ultima amostra em QRS ate o valor atual, descontinuidade?
			}
			else{												// note: entender o uso desse i-start no acesso ao array
				if ((i - start) - state->QRS[state->qrs_index - 1]){
					state->newM5 = 0;
					}
				else{
					state->newM5 = 0.6 * max2(diff_E, state->QRS[state->qrs_index - 1], (i - start));

				}
			}
			if (state->newM5 > 1.5 * state->MM[4]) {
				state->newM5 = 1.1 * state->MM[4];
			}
		}
		//------------------------------ELIF 2-----------------------------------------------
		else if (state->newM5 != 0 && state->qrs_index && i == state->QRS[state->qrs_index - 1] + ms200) {
			if (i >= 5){
				for (int j = 0; j < 4; j++) {
					state->MM[j] = state->MM[j + 1];														// note: criar funcao pop + criar funcao append (quais arrays vao ter malloc?)
				}
				state->MM[4] = state->newM5;
			}
			state->M = mean5(state->MM);
		}
		//------------------------------ELIF 3-------------------------------------------
		else if (state->qrs_index && i > state->QRS[state->qrs_index - 1] + ms200 && i < state->QRS[state->qrs_index - 1] + ms1200) {
			state->M = (mean5(state->MM)) * state->M_slope[i - (state->QRS[state->qrs_index - 1] + ms200)];
		}
		//------------------------------ELIF 4-------------------------------------------
		else if (state->qrs_index && i > state->QRS[state->qrs_index - 1] + ms1200) {
			state->M = 0.6 * (mean5(state->MM));
		}
		//------------------------------ DETECÇÃO ----------------------------------------
		if (!(state->qrs_index) && diff_E[(i-start)] > state->M) {
			state->QRS[state->qrs_index] = i;
			state->thi_list[state->qrs_index] = i;														// note: thi_list pode ser definido dentro de engzee (n precisa de historico)
			state->thi = 1;
			state->qrs_index++;
		} else if (state->qrs_index && i > state->QRS[state->qrs_index - 1] + ms200 && diff_E[(i-start)] > state->M) {
			state->QRS[state->qrs_index] = i;
			state->thi_list[state->qrs_index] = i;
			state->thi = 1;
			state->qrs_index++;
		}
		//------------------------------- THI e THF -------------------------------------
		if (state->thi && i < state->thi_list[state->qrs_index - 1] + ms160) {
			if (diff_E[(i-start)] < -(state->M) && diff_E[i - start - 1] > -(state->M)) {
				state->thf = 1;
			}
			if (state->thf && diff_E[(i-start)] < -(state->M)) {
				state->counter++;
			} else if (diff_E[(i-start)] > -(state->M) && state->thf) {
				state->counter = 0;
				state->thi = 0;
				state->thf = 0;
			}
		} else if (state->thi && i > state->thi_list[state->qrs_index - 1] + ms160) {
			state->counter = 0;
			state->thi = 0;
			state->thf = 0;
		}
		//-------------------------- ENCONTRAR OS PICOS DE FATO ---------------------------
		if (state->counter > neg_threshold) {
			for (int k = state->thi_list[state->qrs_index - 1] - 2; k < i; k++) {
				state->unfiltered_section[state->section_index] = mock_input[k];		// note: me parece estranho
				state->section_index++;
			}
			maxi = indexMax(state->unfiltered_section, max_section_size);

			state->r_peaks[*state->peaks_index] = maxi + state->thi_list[state->qrs_index - 1] - neg_threshold;
			(*state->peaks_index)++;
			state->counter = 0;
			state->thi = 0;
			state->thf = 0;
			state->section_index = 0;
			memset(state->unfiltered_section, 0, max_section_size * sizeof(int));		// note: pq zera unfiltered_section?
		}
	}
	if (first == 0){
		for (int l = 0; l < (*state->peaks_index); l++) {
			state->r_peaks[l] = state->r_peaks[l + 1];
		}
		first++;
		(*state->peaks_index)--;
	}

}
