/*
 * prefiltering.c
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#include <stdio.h>
#include <stdlib.h>
#include "filter.h"
#include "main.h"
/**
 * @brief prefilter
 * @input digital input, size of digital input
 * @output digital input filtered, total taps
 */
//void prefiltering(uint32_t *unfiltered_ecg, uint16_t *total_taps, float *filtered_ecg) {
void prefiltering(Signal *unfiltered_ecg, Signal *filtered_ecg) {
	uint16_t length = unfiltered_ecg->len_signal + unfiltered_ecg->len_state + 1;
	float b_i = 1/7;
    float b1[] = { 0.2, 0.2, 0.2, 0.2, 0.2 };
    float b2[] = { b_i, b_i, b_i, b_i, b_i, b_i, b_i};
//    uint16_t a[] = {1};
//    uint16_t len_b1 = sizeof(b1) / sizeof(b1[0]);
//    uint16_t len_b2 = sizeof(b2) / sizeof(b2[0]);
//    uint16_t len_a = sizeof(a) / sizeof(a[0]);

//    *total_taps += len_b1 + len_b2;

    Signal MA1;
    MA1.len_signal = length;

    statefloatfilter(unfiltered_ecg,&MA1,b1);
//    intfilter(b1, a, len_b1, len_a, unfiltered_ecg, MA1, length);

    statefloatfilter(&MA1,filtered_ecg,b2);
//    floatfilter(b2, a, len_b2, len_a, MA1, filtered_ecg, length);
}


