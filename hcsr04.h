/*
 * hcsr04.h
 *
 *  Created on: Oct 27, 2024
 *      Author: yoyos
 */
//derived from arduino library https://github.com/ErickSimoes/Ultrasonic

#ifndef YHLIB_HCSR04_HCSR04_H_
#define YHLIB_HCSR04_HCSR04_H_

#include "hcsr04_cfg.h"

typedef enum {
	HCSR04_OK, HCSR04_PARAMETER_ERROR, HCSR04_INTERNAL_ERROR, HCSR04_RESULTNB
} HCSR04_Result;

typedef enum{
	hcsr04Idle = 0,
	hcsr04Init
}ehcsr04_Status;

typedef enum
{
	output=0,
	input
}pindir;

typedef struct {
	TIM_HandleTypeDef *htim;
	ehcsr04_Status status;
	uint32_t pin;
	uint32_t port;
}ts_hcsr04;

HCSR04_Result hcsr04_init(ts_hcsr04 *hcsr04, TIM_HandleTypeDef *fhtim, uint32_t fu32Pin, uint32_t fu32port);
float hcsr04_getDistance(ts_hcsr04 *hcsr04);

#endif /* YHLIB_HCSR04_HCSR04_H_ */
