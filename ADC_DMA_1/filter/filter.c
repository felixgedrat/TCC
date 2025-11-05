/*
 * filter.c
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "filter.h"
#include <main.h>

/**
 * @brief int filter
 * @input integer array to be filtered
 * @output float filtered array
 */
void intfilter(float* b, uint16_t* a, uint16_t len_b, uint16_t len_a, uint32_t* x, float* y, uint16_t len_x) {
    for (int i = 0; i < len_x; i++) {
        y[i] = 0;

        for (int j = 0; j < len_b && i - j >= 0; j++) {
            y[i] += b[j] * x[i - j];
        }

        for (int j = 1; j < len_a && i - j >= 0; j++) {
            y[i] -= a[j] * y[i - j];
        }
    }
}

/**
 * @brief float filter
 * @input float array to be filtered
 * @output float filtered array
 */
void floatfilter(float *b, uint16_t *a, uint16_t len_b, uint16_t len_a, float *x, float *y, uint16_t len_x) {
    for (int i = 0; i < len_x; i++) {
        y[i] = 0;

        for (int j = 0; j < len_b && i - j >= 0; j++) {
            y[i] += b[j] * x[i - j];
        }

        for (int j = 1; j < len_a && i - j >= 0; j++) {
            y[i] -= a[j] * y[i - j];
        }
    }
}

/**
 * @brief transform buffer of uint16_t values to float buffer
 * @input uint16_t input array
 * @output float output array
 */
void array_conversion(uint16_t* int_array, float* float_array, uint16_t buffer_len) {
	for(uint16_t i=0; i<buffer_len; i++) {
		float_array[i] = (float)int_array[i];
	}
}



/**
 * @brief implementation of float filter with state keeping
 * @input
 * @output
 */

//void statefloatfilter(uint32_t* x, uint16_t len_x,uint32_t* state,uint32_t* y, uint32_t* filter, uint16_t filter_order){
void statefloatfilter(struct Signal* input_signal, struct Signal* output_signal, float* filter){
	uint16_t len_state = input_signal->len_state;
	uint16_t len_input_signal = input_signal->len_signal;
	float y_state[BUF_LEN];
	memset(y_state,0,BUF_LEN*sizeof(float));
	uint16_t i;
	uint16_t j;

//	for(i=0; i<len_state;i++){
//		x_state[i] = input_signal->state[i];
//	}
//	for(i=len_state; i<len_input_signal+len_state;i++){
//		x_state[i] = input_signal->signal[i-len_state];
//	}

	for(i=0; i<len_input_signal+len_state ; i++){
		for (j = 0; j < len_state+1 && i - j >= 0; j++) {
		            y_state[i] += filter[j] * input_signal->signal[i - j];
		        }
	}

	for(i=0; i<len_state;i++) {
		output_signal->signal[i]=y_state[i]+input_signal->state[i];
	}

	for(i=len_state; i<len_input_signal;i++){
		output_signal->signal[i]=y_state[i];
	}

	for(i=len_input_signal;i<len_state+len_input_signal;i++){
		input_signal->state[i-len_input_signal]=y_state[i];
	}

}

