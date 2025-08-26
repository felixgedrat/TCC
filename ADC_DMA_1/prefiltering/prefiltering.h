/*
 * prefiltering.h
 *
 *  Created on: Jun 9, 2024
 *      Author: João Pedro
 */

#ifndef PREFILTERING_H_
#define PREFILTERING_H_

#include <stdio.h>
#include <stdlib.h>
#include "filter.h"
#include "main.h"

#define FILTER_B1_ORDER 4
#define FILTER_B2_ORDER 6

struct Signal;

void prefiltering(struct Signal *unfiltered_ecg, struct Signal *filtered_ecg);

#endif /* PREFILTERING_H_ */
