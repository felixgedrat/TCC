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

/**
 * @brief Max value inside an array with size
 * @input Array, size of array
 * @output Max value
 */
float max(float* arr, int size) {
	float max_val = arr[0];
	for (int i = 1; i < size; i++) {
		if (arr[i] > max_val) {
			max_val = arr[i];
		}
	}
	return max_val;
}

/**
 * @brief Max value inside an array with start and end parameters
 * @input Array, start and end limits
 * @output Max value
 */
float max2(float* arr, int start, int end) {
	float max_val = arr[start];
	start++;
	while(start < end){
		if (arr[start] > max_val) {
			max_val = arr[start];
		}
		start++;
	}
	return max_val;
}

/**
 * @brief Index Max value inside an array
 * @input Array, size of array
 * @output Max Index value
 */
int indexMax(int* arr, int size){
	int max_val = arr[0];
	int index = 0;
	for (int i = 1; i < size; i++) {
		if (arr[i] > max_val) {
			max_val = arr[i];
			index = i;
		}
	}
	return index;
}

/**
 * @brief Function to compare integers for qsort
 * @input two values to be compared
 * @output Result of comparison
 */
int compareInts(const void* a, const void* b) {
	return (*(int*)a - *(int*)b);
}

/**
 * @brief Function to intersection of two arrays
 * @input two arrays to be compared
 * @output intersection of two arrays
 */
void intersect(int* arr1, int len1, int* arr2, int len2, int* result_len, int* result) {
	//int* result = (int*)malloc(sizeof(int) * (len1 > len2 ? len2 : len1));
	int idx = 0;

	for (int i = 0; i < len1; i++) {
		for (int j = 0; j < len2; j++) {
			if (arr1[i] == arr2[j]) {
				result[idx++] = arr1[i];
				break;
			}
		}
	}
	*result_len = idx;
	return;
}

/**
 * @brief Finds first value in array >= lower_bound, assuming array is sorted in ascending order
 * @input array, size of array and lower bound
 * @output index of value that met the criteria
 */
int lowerBound(int* arr, int size, int lower_bound) {
    int low = 0;
    int high = size - 1;

    while (low < high) {
        int mid = (low + high) / 2;
        if (arr[mid] < lower_bound) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }
    return low;
}

/**
 * @brief Finds first value in array <= upper_bound, assuming array is sorted in ascending order
 * @input array, size of array and upper bound
 * @output index of value that met the criteria
 */
int upperBound(int* arr, int size, int upper_bound) {
    int low = 0;
    int high = size - 1;

    while (low < high) {
        int mid = (low + high + 1) / 2;
        if (arr[mid] > upper_bound) {
            high = mid - 1;
        } else {
            low = mid;
        }
    }
    return high;
}

/**
 * @brief Finds values >= lower bound and <= upper_bound in array, assuming array is sorted in ascending order
 * @input array, bounds and result pointer and array len
 * @output void
 */
void findValuesInRange(int* arr, int size, int lower_bound, int upper_bound, int* result_len, int* result) {
    int start = lowerBound(arr, size, lower_bound);
    int end = upperBound(arr, size, upper_bound);

    printf("Valores entre %d e %d:\n", lower_bound, upper_bound);
    for (int i = start; i <= end; i++) {
        printf("%d ", arr[i]);
        result[(*result_len)++] = arr[i];
    }
    printf("\n");
}

/**
 * @brief Returns mean of array of 5 values
 * @input array
 * @output mean
 */
float mean5(float* arr) {
    float mean = (arr[0]+arr[1]+arr[2]+arr[3]+arr[4])/5;
    return mean;
}
