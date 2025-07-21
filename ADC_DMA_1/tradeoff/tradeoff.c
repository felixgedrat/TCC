/*
 * tradeoff.c
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
#include "main.h"

//#define MAX_LEN 320
#define MAX_LEN 500
#define MAX_SEARCH_INTERVAL 200
#define DELTA 40	// DELTA = missed beats will be searched in the detections[i]+delta, detections[i+1]-delta section

/**
 * @brief Tradeoff
 * @input Christov and Engzee detections and sizes
 * @output final detections after tradeoff
 */
void tradeoff(EngzeeState* engzee_state, ChristovState* christov_state, FinalDetect* final_detect) {
//void tradeoff(int* engzee_detection, int len_engzee, int* christov_detection, int len_christov, int* detections, int* len_detections) {
    // static int search_interval[MAX_SEARCH_INTERVAL];

    final_detect->len_detections = engzee_state->len_engzee;
    for (int i = 0; i < engzee_state->len_engzee; i++) {
        final_detect->detections[i] = engzee_state->r_peaks[i];
    }

    if (final_detect->len_detections > 2*DELTA) {
    	for (int i = 0; i < engzee_state->len_engzee - 1; i++) {

    	        int a = final_detect->detections[i] + DELTA;         // determines upper and lower bounds for searching missed
    	        int b = final_detect->detections[i + 1] - DELTA;     //      detections in Christov
    	        if (b - a < 0) {
    	            continue;
    	        }

    	        int missed_beat_len = 0;
    	        int missed_beat[christov_state->len_detection];      // maximum number of missed detections is equal to Christov length
    	        findValuesInRange(christov_state->QRS, christov_state->len_detection, a, b, &missed_beat_len, missed_beat);
    	        if (missed_beat_len > 0) {
    	            for (int k = 0; k < missed_beat_len; k++) {
    	                final_detect->detections[(final_detect->len_detections)++] = missed_beat[k];    // add missed detections to array
    	            }
    	        }
    	}
//    	int len_detections_ini = *len_detections;
//        for (int i = 0; i < len_detections_ini - 1; i++) {
//            int a = detections[i] + 10;
//            int b = detections[i + 1] - 10;
//            if (b - a < 0) {
//                continue;
//            }
//            int search_interval_len = b - a + 1;
//            for (int j = 0; j < search_interval_len; j++) {
//                search_interval[j] = a + j;
//            }
//
//            int missed_beat_len = 0;
//            int missed_beat[MAX_LEN];
//            intersect(search_interval, search_interval_len, christov_detection, len_christov,&missed_beat_len, &missed_beat[0]);
//
//            if (missed_beat_len > 0) {
//                if (*len_detections + missed_beat_len > MAX_LEN) {
//                    exit(1);
//                }
//                for (int k = 0; k < missed_beat_len; k++) {
//                    detections[(*len_detections)++] = missed_beat[k];
//                }
//
//            }
//        }
    } else {
        for (int i = 0; i < christov_state->len_detection; i++) {
            final_detect->detections[i] = christov_state->QRS[i];
        }
        final_detect->len_detections = christov_state->len_detection;
    }

    qsort(final_detect->detections, final_detect->len_detections, sizeof(int), compareInts);
}
