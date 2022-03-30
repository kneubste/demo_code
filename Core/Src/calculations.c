/** ***************************************************************************
 * @file
 * @brief Calculations for the measured values.
 *
 * ==============================================================
 *
 *
 *
 * ----------------------------------------------------------------------------
 * @author Stefan Kneubühl, kneubste@students.zhaw.ch
 * @author My-Hanh Dang,	dangmyh1@students.zhaw.ch
 * @date 28.11.2021
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdio.h>
#include <math.h>
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"

#include "calculations.h"
#include "measuring.h"
#include "displayingdata.h"



/******************************************************************************
 * Defines
 *****************************************************************************/

int32_t lut_pad_wire[] = {
		#include "lut_pad_wire.csv"
		};								///< Lookup table of the pads for a wire
int32_t lut_pad_cable[] = {
		#include "lut_pad_cable.csv"
		};								///< Lookup table of the pads for a cable
int32_t lut_coil_1_2[] ={
		#include "lut_coil_1_2.csv"
		};								///< Lookup table of the coils for 1.2A
int32_t lut_coil_5[] ={
		#include "lut_coil_5.csv"
		};								///< Lookup table of the coils for 5A

/******************************************************************************
 * Variables
 *****************************************************************************/
int32_t avg = 0; ///< average

int32_t pad1 = 0; ///< Values for pad1
int32_t pad2 = 0; ///< Values for pad2

bool CALC_degree_center = false; ///< Flag for the direction of the signal
bool CALC_degree_left = false;	///< Flag for the direction of the signal
bool CALC_degree_right = false;	///< Flag for the direction of the signal

/******************************************************************************
 * Functions
 *****************************************************************************/


/** **************************************************************************
 * @brief 	Calculate the average of the ADC samples
 * @param	numb_samples Number of samples
 * @param	arr[] Array filed with samples
 * @note	-
 * @return	Calculated average of the ADC samples
 *****************************************************************************/
int32_t average(int32_t numb_samples, int32_t arr[]){
	int i;
	int32_t avg = 0;

	for(i = 0; i < numb_samples; i++){
		avg += arr[i];
	}
	avg /= numb_samples;

	return avg;
}

/** **************************************************************************
 * @brief 	calculate the RMS value
 * @param	numb_samples: number of samples
 * @param	arr[]: array filled with samples
 * @note	-
 * @return 	calculated RMS value
 *****************************************************************************/
int32_t RMS(int32_t numb_samples, int32_t arr[]){
	int i;
	int32_t avg = 0;
	int32_t rms = 0;
	int32_t tmp = 0;
	int32_t diff = 0;

	avg = average(numb_samples, arr);

	for(i = 0; i < numb_samples; i++){
		diff = arr[i] - avg;
		tmp = diff * diff;
		rms = rms + tmp;
	}
	rms = rms / numb_samples;
	rms = sqrt(rms);

	return rms;
}




/** **************************************************************************
 * @brief find the distance between device and cable
 * @note  	measure in the range of [5,200]mm and has a precision of -/+30%
 * @param 	meas_mode	1 = single measurement (10 samples),
 * 			else = accurate measurement (50 samples)
 * @param	meas_type	1 = wire
 * 						else = cable
 * @return 	distance to the cable up to 200mm
 *****************************************************************************/
int32_t distance_to_cable(int32_t meas_mode){

	int32_t dist = 0;
	int32_t e_val = 0; //< e_val is the electrical field value



		if(meas_mode == 1){
			pad1 = RMS(10, PAD1_samples);
			pad2 = RMS(10, PAD2_samples);
		}
		else{
			pad1 = RMS(50, PAD1_samples);
			pad2 = RMS(50, PAD2_samples);
		}
		e_val = (pad1 + pad2) / 2;

	if(MEAS_data_wire){

		if(e_val >= 900){
			dist = 0;
			return dist;
		}
		else if(e_val <= 96){
			dist = -1;
			return dist;
		}
		else{
			while(lut_pad_wire[dist] > e_val){
				dist++;
			}
			return dist;}
		}
		else if(MEAS_data_cable){
		if(e_val >= 500){
			dist = 0;
			return dist;
		}
		else if(e_val <= 72){
			dist = -1;
			return dist;
		}
		else{
			while(lut_pad_wire[dist]>e_val){
				dist++;
			}
			return dist;
		}

		}

	return -1;
}

/** **************************************************************************
 * @brief 	calculate the angle to the cable
 * @note  	range [-45,45]°, precision -/+15
 * 			calibrated @ 20mm distance to wire/cable
 * @n		The field is very location and board depending.
 * @return	angle to wire/cable
 *****************************************************************************/
int32_t angle_to_cable(){

	float diff;
	float ratio;
	float angle;
	int32_t pad1 = 0;
	int32_t pad2 = 0;

	pad1 = RMS(50, PAD1_samples);
	pad2 = RMS(50, PAD2_samples);

	diff = pad1 - pad2;

	if(diff > 232){
		diff = 232;
	}

	if(diff < -222){
		diff = -222;
	}

	if(diff > 20)
	{
		ratio = diff / 232;
		angle = 45 * ratio;
		CALC_degree_left = true;
	}
	else
	{
		ratio = diff / 222;
		angle = 45 * ratio;
		CALC_degree_right = true;
	}



	return angle;

}



/** **************************************************************************
 * @brief 	calculate the current in a wire or cable
 * @param	meas_mode	1 = single, else = accurate
 * @note  	return expected current 5A or 1.2A
 * 			The field is very location and board depending.
 * @return	-1 if no clear value
 *****************************************************************************/
int32_t current(int32_t meas_mode){

	int32_t coil1 = 0;
	int32_t coil2 = 0;
	int32_t b_val = 0; //<
	int current = 0;

	//Calculate RMS for single / accu value
	if(meas_mode == 1){
		coil1 = RMS(10, COIL1_samples);
		coil2 = RMS(10, COIL2_samples);
	}
	else{
		coil1 = RMS(50, COIL1_samples);
		coil2 = RMS(50, COIL2_samples);
	}

	// Mean value for the b-field. The field is very location and board depending.
	b_val = (coil1 + coil2) / 2;

	if(MEAS_data_wire){

		if((b_val <= 850) && (b_val > 400)){
			current = 1200;
			return current;
		}
		else if(b_val > 850){
			current = 5000;
			return current;
		}
	}
	else if(MEAS_data_cable){

		if((b_val <= 420) && (b_val >= 250)){
			current = 1200;
			return current;
		}
		else if((b_val > 400)){
			current = 5000;
			return current;
		}
	}
	return -1;
}



