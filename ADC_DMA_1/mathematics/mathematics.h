/*
 * mathematics.h
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#ifndef MATHEMATICS_H_
#define MATHEMATICS_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

float max(float* arr, int size);
float max2(float* arr, int start, int end);
int indexMax(int* arr, int size);
int compareInts(const void* a, const void* b);
void intersect(int* arr1, int len1, int* arr2, int len2, int* result_len, int* result);
int lowerBound(uint32_t* arr, int size, int lower_bound);
int upperBound(uint32_t* arr, int size, int upper_bound);
void findValuesInRange(uint32_t* arr, uint32_t size, int lower_bound, int upper_bound, int* result_len, int* result);
float mean(float* arr, uint16_t array_size);
uint16_t append5(float* array, uint16_t array_size, float value);
uint16_t append_ms350(float* array, uint16_t array_size, float value);

#endif /* MATHEMATICS_H_ */
