/*
 * filter.h
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#ifndef FILTER_H_
#define FILTER_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <main.h>

void intfilter(float* b, uint16_t* a, uint16_t len_b, uint16_t len_a, uint32_t* x, float* y, uint16_t len_x);
void floatfilter(float *b, uint16_t *a, uint16_t len_b, uint16_t len_a, float *x, float *y, uint16_t len_x);
void statefloatfilter(struct Signal* input_signal,struct Signal* output_signal, float* filter);
void array_conversion(uint32_t* int_array, float* float_array, uint16_t buffer_len);


#endif /* FILTER_H_ */
