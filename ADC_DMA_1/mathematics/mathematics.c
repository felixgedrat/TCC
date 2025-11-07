/*
 * mathematics.c
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <mathematics.h>
#include <stdint.h>
#include <christov.h>
#include <stdint.h>

/**
 * @brief Finds the maximum value within an array.
 * * @param arr Pointer to the array of floating-point numbers.
 * @param size The number of elements in the array.
 * @return float The maximum floating-point value found in the array.
 */
float max(float* arr, uint16_t size) {
	float max_val = arr[0];
	uint16_t i;
	for (i = 1; i < size; i++) {
		if (arr[i] > max_val) {
			max_val = arr[i];
		}
	}
	return max_val;
}

/**
 * @brief Finds the maximum value within a specified range of an array.
 * * **NOTE ON LOGIC:** The implementation must be corrected to use arr[i]
 * when updating max_val, and boundary checks are recommended.
 * * @param arr Pointer to the array of floating-point numbers.
 * @param start The starting index of the range (inclusive).
 * @param end The ending index of the range (exclusive).
 * @return float The maximum floating-point value found in the sub-array [start, end).
 */
float maxStartEnd(float* arr, uint16_t start, uint16_t end){
	uint16_t i;
	float max_val = arr[start];
	for (i = start + 1; i < end; i++)
		if (arr[i] > max_val) {
			max_val = arr[i];
		}
	return max_val;
}

/**
 * @brief Finds the index of the maximum value within an array.
 * * @param arr Pointer to the array of floating-point numbers.
 * @param size The number of elements in the array.
 * @return int The index of the first occurrence of the maximum value.
 */
uint16_t indexMax(float* arr, uint16_t size){
	float max_val = arr[0];
	uint16_t index = 0;
	uint16_t i;
	for (i = 1; i < size; i++) {
		if (arr[i] > max_val) {
			max_val = arr[i];
			index = i;
		}
	}
	return index;
}

/**
 * @brief Comparison function for integers, designed for use with qsort().
 * * **NOTE ON SAFETY:** For maximum safety, explicit comparison (if/else) is
 * recommended to prevent potential integer overflow during subtraction.
 * * @param a Pointer to the first integer value to be compared.
 * @param b Pointer to the second integer value to be compared.
 * @return int A value less than, equal to, or greater than zero if the first
 * argument is considered to be respectively less than, equal to, or greater
 * than the second.
 */
int compareInts(const void* a, const void* b) {
	return (*(int*)a - *(int*)b);
}


/**
 * @brief Performs binary search to find the index of the first element
 * greater than or equal to a specified lower bound.
 * * Assumes the input array is sorted in ascending order.
 * * @param arr Pointer to the array of unsigned 32-bit integers (uint32_t).
 * @param size The number of elements in the array.
 * @param lower_bound The value to search for (lower bound criteria).
 * @return int The index of the first value that meets the criteria. Returns 'size' if no such element is found (or 'low' in your implementation).
 */
uint16_t lowerBound(uint32_t* arr, uint16_t size, uint32_t lower_bound) {
	uint16_t low = 0;
	uint16_t high = size - 1;

    while (low < high) {
    	uint16_t mid = (low + high) / 2;
        if (arr[mid] < lower_bound) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return low;
}

/**
 * @brief Performs binary search to find the index of the last element
 * less than or equal to a specified upper bound.
 * * Assumes the input array is sorted in ascending order.
 * * @param arr Pointer to the array of unsigned 32-bit integers (uint32_t).
 * @param size The number of elements in the array.
 * @param upper_bound The value to search for (upper bound criteria).
 * @return int The index of the last value that meets the criteria. Returns -1 if no such element is found.
 */
uint16_t upperBound(uint32_t* arr, uint16_t size, uint32_t upper_bound) {
    uint16_t low = 0;
    uint16_t high = size - 1;

    while (low < high) {
        uint16_t mid = (low + high + 1) / 2;
        if (arr[mid] > upper_bound) {
            high = mid - 1;
        } else {
            low = mid;
        }
    }
    return high;
}

/**
 * @brief Finds and extracts values within a specified range [lower_bound, upper_bound].
 * * Assumes the input array is sorted in ascending order.
 * * @param arr Pointer to the array of unsigned 32-bit integers (uint32_t).
 * @param size The number of elements in the array.
 * @param lower_bound The inclusive lower boundary.
 * @param upper_bound The inclusive upper boundary.
 * @param result_len [in,out] Pointer to a variable holding the current size of the result array, updated with the new count.
 * @param result [out] The pre-allocated array where the found values will be stored.
 * @attention This function does NOT check the maximum allocated size of 'result'.
 * The caller must ensure 'result' has enough space to prevent **buffer overflow**.
 * @return void
 */
void findValuesInRange(uint32_t* arr, uint32_t size, uint32_t lower_bound, uint32_t upper_bound, uint8_t* result_len, uint32_t* result) {
	uint16_t i;
    uint16_t start = lowerBound(arr, size, lower_bound);
    uint16_t end = upperBound(arr, size, upper_bound);
    for (i = start; i <= end; i++) {
        result[(*result_len)++] = arr[i];
    }
}

/**
 * @brief Calculates the arithmetic mean (average) of an array of floating-point values.
 * * @param arr Pointer to the array of floating-point numbers.
 * @param array_size The number of elements in the array.
 * @return float The calculated arithmetic mean.
 */
float mean(float* arr, uint16_t array_size) {
    float mean = 0;
    for (uint16_t i=0; i<array_size; i++) {
    	mean = mean + arr[i];
    }
    mean = mean/array_size;
    return mean;
}

/**
 * @brief Appends a value to a fixed-size array (size 5), implementing a circular buffer logic.
 * * If the array is full (size 5), the first element is discarded (FIFO / push-pop(0) behavior).
 * * @param array The array to append the value to. Must be of size 5 or more.
 * @param array_size The current number of valid elements in the array.
 * @param value The floating-point value to append.
 * @return uint16_t The new size of the array (will be max 5).
 */
uint16_t append5(float* array, uint16_t array_size, float value){
	if (array_size < 5){
		array[array_size++]= value;
	} else if (array_size == 5) {
		for (int j = 0; j < 4; j++) {
			array[j] = array[j + 1];
		}
		array[4] = value;
	}
	return array_size;
}


/**
 * @brief Appends a value to a fixed-size array (size MS350), implementing a circular buffer logic.
 * * If the array is full (size MS350), the first element is discarded (FIFO / push-pop(0) behavior).
 * * @param array The array to append the value to. Must be of size MS350 or more.
 * @param array_size The current number of valid elements in the array.
 * @param value The floating-point value to append.
 * @return uint16_t The new size of the array (will be max MS350).
 */
uint16_t append_ms350(float* array, uint16_t array_size, float value){
	if (array_size < ms350){
		array[array_size++]= value;
	} else if (array_size == ms350) {
		for (int j = 0; j < ms350 - 1; j++) {
			array[j] = array[j + 1];
		}
		array[ms350-1] = value;
	}
	return array_size;
}
