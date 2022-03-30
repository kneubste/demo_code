/** ***************************************************************************
 * @file
 * @brief See calculations.c
 *
 * Prefixes CALC,
 *

 * @author Stefan Kneubühl, kneubste@students.zhaw.ch
 * @author
 * @date 30.11.2021
 *****************************************************************************/

#ifndef CALC_H_
#define CALC_H_


/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdbool.h>
#include <stdio.h>
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"


/******************************************************************************
 * Defines
 *****************************************************************************/

extern bool CALC_degree_left;		///< Flag for the direction of the signal
extern bool CALC_degree_right;		///< Flag for the direction of the signal

/******************************************************************************
 * Functions
 *****************************************************************************/
int32_t average(int32_t numb_samples, int32_t arr[]);
int32_t RMS(int32_t numb_samples, int32_t arr[]);
int32_t standard_deviation(int32_t avg, int32_t numb_samples, int32_t arr[]);
int32_t distance_to_cable(int32_t meas_mode);
int32_t angle_to_cable(void);
int32_t current(int32_t meas_mode);



#endif
