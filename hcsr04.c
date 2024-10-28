/*
 * hcsr04.c
 *
 *  Created on: Oct 27, 2024
 *      Author: yoyos
 */

#include "hcsr04.h"

float hcsr04_pulseIn(ts_hcsr04 *hcsr04);
HCSR04_Result hcsr04_setPinMode(ts_hcsr04 *hcsr04, pindir fepindirection);
void hcsr04_delayMs(ts_hcsr04 *hcsr04, uint32_t fu32Value);
uint32_t hcsr04_PCLKxTIM(TIM_HandleTypeDef* ftim);

HCSR04_Result hcsr04_init(ts_hcsr04 *hcsr04, TIM_HandleTypeDef *fhtim, uint32_t fu32Pin, uint32_t fu32port)
{
	HCSR04_Result eResult = HCSR04_OK;
	if(fhtim == 0 || hcsr04 == 0)
	{
		eResult = HCSR04_PARAMETER_ERROR;
	}

	else
	{
		if (fhtim->State != HAL_TIM_STATE_READY) {
			  // timer is not initialized, or already started
				eResult = HCSR04_PARAMETER_ERROR;
			}
			else
			{
				if((fhtim->Instance->PSC +1)*1000000 != hcsr04_PCLKxTIM(fhtim))//force pwm resolution to 1µs
				{
					eResult = HCSR04_PARAMETER_ERROR;
				}
			}
	}
	if(eResult == HCSR04_OK)
	{
		hcsr04->htim = fhtim;
		hcsr04->status = hcsr04Init;
		hcsr04->pin = fu32Pin;
		hcsr04->port = fu32port;
		HAL_TIM_Base_Start(hcsr04->htim);
	}
	return eResult;
}

HCSR04_Result hcsr04_setPinMode(ts_hcsr04 *hcsr04, pindir fepindirection)
{
	HCSR04_Result eResult = HCSR04_OK;
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(fepindirection == output)
	{
		GPIO_InitStruct.Pin = hcsr04->pin;
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
	  	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	  	HAL_GPIO_Init(hcsr04->port, &GPIO_InitStruct);
	}
	else if(fepindirection == input)
	{
		GPIO_InitStruct.Pin = hcsr04->pin;
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(hcsr04->port, &GPIO_InitStruct);
	}
	return eResult;
}

void hcsr04_delayMs(ts_hcsr04 *hcsr04, uint32_t fu32Value)
{
	__HAL_TIM_SET_COUNTER(hcsr04->htim, 0);
	while(__HAL_TIM_GET_COUNTER(hcsr04->htim)<fu32Value);
}

float hcsr04_getDistance(ts_hcsr04 *hcsr04){
	float distance = 0;
	if(hcsr04->status==hcsr04Init)
	{
		hcsr04_setPinMode(hcsr04, output);
		HAL_GPIO_WritePin(hcsr04->port, hcsr04->pin, GPIO_PIN_RESET);  // pull the TRIG pin LOW
		hcsr04_delayMs(hcsr04,2U);
		HAL_GPIO_WritePin(hcsr04->port, hcsr04->pin, GPIO_PIN_SET);  // pull the TRIG pin HIGH
		hcsr04_delayMs(hcsr04,20U);
		HAL_GPIO_WritePin(hcsr04->port, hcsr04->pin, GPIO_PIN_RESET);
		hcsr04_setPinMode(hcsr04, input);
		distance = hcsr04_pulseIn(hcsr04);
	}

    return distance;
}

float hcsr04_pulseIn(ts_hcsr04 *hcsr04)
{
	uint32_t t0 = 0;
	__HAL_TIM_SET_COUNTER(hcsr04->htim, 0);
	uint32_t count=0;
	while(count < HCSR04_TIMEOUT)
	{
		if(HAL_GPIO_ReadPin(hcsr04->port, hcsr04->pin)==GPIO_PIN_SET)
		{
			break;
		}
		count++;
	}
	if(count >=	HCSR04_TIMEOUT)
	{
		return 0;
	}

	uint32_t t1 = __HAL_TIM_GET_COUNTER(hcsr04->htim);
	count=0;
	while(count < HCSR04_TIMEOUT)
	{
		if(HAL_GPIO_ReadPin(hcsr04->port, hcsr04->pin)==GPIO_PIN_RESET)
		{
			break;
		}
		count++;
	}
	if(count >=	HCSR04_TIMEOUT)
	{
		return 0;
	}
	uint32_t t2 = __HAL_TIM_GET_COUNTER(hcsr04->htim);
	float distance = ((float)(t2 - t1) * 0.0343 / 2);    // cm

	return(distance);
}

uint32_t hcsr04_PCLKxTIM(TIM_HandleTypeDef* ftim)
{
  /* Get PCLK1 frequency */
  uint32_t pclkx;
  if(ftim->Instance == TIM3 || ftim->Instance == TIM4 || ftim->Instance == TIM5 || ftim->Instance == TIM6 || ftim->Instance == TIM7)
  {
	  pclkx = HAL_RCC_GetPCLK1Freq();
	  if((RCC->CFGR & RCC_CFGR_PPRE1) != 0)
	  {
		  pclkx *=2;
	  }
  }
  else
  {
	  pclkx = HAL_RCC_GetPCLK2Freq();
	  if((RCC->CFGR & RCC_CFGR_PPRE2) != 0)
	  {
		  pclkx *=2;
	  }
  }
  return pclkx;
}
