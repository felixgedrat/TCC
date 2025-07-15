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
void christov_differentiation(float *input, float *diff_C, int length) {
	int count = 0;
	for (int i = 1; i < length - 1; i++) {
		diff_C[count++] = fabs(input[i + 1] - input[i - 1]);
	}
}

/**
 * @brief Call lfilter to diff_signals
 * @input differentiated values
 * @output filtered values
 */
void chistov_noise(float *diff_signal, float *diff_filtered_signal, uint16_t total_taps, int length) {
	float b[10] = { 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1 };
	uint16_t a[] = {1};
	uint16_t len_b = sizeof(b) / sizeof(b[0]);
	uint16_t len_a = sizeof(a) / sizeof(a[0]);
	total_taps += len_b;

	floatfilter(b, a, len_b, len_a, diff_signal, diff_filtered_signal, length);

	for (int i = 0; i < total_taps; i++) {
		diff_filtered_signal[i] = 0;
	}
}


/**
 * @brief Christov Detection
 * @input Digitized input, Christov differentiated array, sample buffer index, MM and RR
 * @output Spikes detected - Christov
 */
void christov(uint16_t* mock_input, float* MA3, int length, int sample, ChristovState* state){ // int fs, int* QRS, int *len_detection, float *MM, float *RR, int *R_idx) {
	//int qrs_index = *len_detection;
	// float M = 0;
	// float newM5 = 0;
//	float M_slope[250];
//	float F = 0;
//	int R = 0;
//	int Rm = 0;
//	int first = *len_detection;
	int start = (length * sample);
//	int idx = *R_idx;
//	const float increment = 0.0016064257028112205;
//
//	for (int j = 0; j < ms1200 - ms200; ++j) {
//		M_slope[j] = 1.0 - j * increment;
//	}

//	float* F_section = (float*)malloc(ms350 * sizeof(float));
	float F_section[ms350+50];

	for (int i = start; i < length * (sample + 1); i++) {
		if ((i-start) < 5 * state->fs) {
			state->M = 0.6 * max(MA3, i - start + 1);
			if (i < 5){
				state->MM[i] = state->M;
			} else if (i >= 5){
				for (int j = 0; j < 4; j++) {
					state->MM[j] = state->MM[j + 1];
				}
				state->MM[4] = state->M;
			}
		}
		else if (state->qrs_index && i < state->QRS[state->qrs_index - 1] + ms200) {
			if (state->QRS[state->qrs_index - 1] < start){
				state->newM5 = 0.6 * max2(MA3, 0, (i - start));
			}
			else{
				state->newM5 = 0.6 * max2(MA3, state->QRS[state->qrs_index - 1], (i - start));
			}
			if (state->newM5 > 1.5 * state->MM[4]) {
				state->newM5 = 1.1 * state->MM[4];
			}
		}
		else if (state->qrs_index && i == state->QRS[state->qrs_index - 1] + ms200) {
			if (state->newM5 == 0) {
				state->newM5 = state->MM[4];
			}
			for (int j = 0; j < 4; j++) {
				state->MM[j] = state->MM[j + 1];
			}
			state->MM[4] = state->newM5;
			state->M = mean5(state->MM);
		}
		else if (state->qrs_index && i > state->QRS[state->qrs_index - 1] + ms200 && i < state->QRS[state->qrs_index - 1] + ms1200) {
			state->M = (mean5(state->MM)) * state->M_slope[i - (state->QRS[state->qrs_index - 1] + ms200)];
		}
		else if (state->qrs_index && i > state->QRS[state->qrs_index - 1] + ms1200) {
			state->M = 0.6 * (mean5(state->MM));
		}

		if ((i - start) > ms350) {
//			if (F_section == NULL) {
//				exit(1);
//			}
			for (int j = 0; j < ms350; j++) {
				F_section[j] = MA3[i - start - ms350 + j];
			}
			float max_latest = F_section[ms350 - ms50];
			for (int j = ms350 - ms50 + 1; j < ms350; j++) {
				if (F_section[j] > max_latest) {
					max_latest = F_section[j];
				}
			}
			float max_earliest = F_section[0];
			for (int j = 1; j < ms50; j++) {
				if (F_section[j] > max_earliest) {
					max_earliest = F_section[j];
				}
			}

			state->F = state->F + ((max_latest - max_earliest) / 150.0);

		}

		if (state->qrs_index && i < state->QRS[state->qrs_index - 1] + (2.0 / 3.0 * state->Rm)) {
			state->R = 0;
		} else if (state->qrs_index && i > state->QRS[state->qrs_index - 1] + (2.0 / 3.0 * state->Rm) && i < state->QRS[state->qrs_index - 1] + state->Rm) {
			int dec = (state->M - mean5(state->MM)) / 1.4;
			state->R = 0 + dec;
		}

		state->MFR = state->M + state->F + state->R;

		if (!(state->qrs_index) && MA3[i - start] > state->MFR) {
			state->QRS[state->qrs_index++] = i;
		} else if (state->qrs_index && i > state->QRS[state->qrs_index - 1] + ms200 && MA3[i - start] > state->MFR) {
			state->QRS[state->qrs_index++] = i;

			if (state->qrs_index > 2) {
				if (state->rr_index < 5){
					state->RR[state->rr_index] = (state->QRS[state->qrs_index - 1] - state->QRS[state->qrs_index - 2]);
					state->rr_index++;
				}else{
					for (int j = 0; j < 4; j++) {
						state->RR[j] = state->RR[j + 1];
					}
					state->RR[4] = (state->QRS[state->qrs_index - 1] - state->QRS[state->qrs_index - 2]);
				}
				state->Rm = mean5(state->RR);
			}

		}
	}

//	free(F_section);

	for (int l = state->len_detection; l < state->qrs_index; l++) {
		state->QRS[l] = state->QRS[l + 1];
	}
	state->qrs_index--;

//	*len_detection = qrs_index;
//	*R_idx = idx;
}
