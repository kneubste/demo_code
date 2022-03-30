/** ***************************************************************************
 * @file
 * @brief Displaying the measured data on the touch screen.
 *
 * ==============================================================
 *
 *
 * ----------------------------------------------------------------------------
 * @author Stefan Kneubühl, kneubste@students.zhaw.ch
 * @author Niklaus Schoch,	schocnik@students.zhaw.ch
 * @date 30.11.2021
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

#include "measuring.h"
#include "calculations.h"
#include "displayingdata.h"

/******************************************************************************
 * Defines
 *****************************************************************************/



/******************************************************************************
 * Variables
 *****************************************************************************/

int32_t dist_single = 0; 	 	///< Value for single measurement
int32_t dist_accu = 0; 	 	///< Value for accurate measurement
float 	current_single = 0; 	///< Value for single measurement
float 	current_accu = 0;		///< Value for accurate measurement

/******************************************************************************
 * Functions
 *****************************************************************************/

/** **************************************************************************
 * @brief Function for displaying the wire data
 * @note  	This function calls the required functions to display the data for a wire measurement
 * @n		Clears the ADC_samples array after displaying all the data
 *****************************************************************************/
void DISP_show_data_wire(void)
{
	const uint32_t Y_OFFSET_PAD = 220;					//Offset for pad-graph
	const uint32_t Y_OFFSET_COIL = 280;					//0ffset for coil-graph
	const uint32_t X_SIZE = 240;
	const uint32_t f = (6 << ADC_DAC_RES) /	Y_OFFSET_COIL + 1;   	// Scaling factor
	uint32_t data;
	uint32_t data_last;


	// Call for calculations
	dist_single = distance_to_cable(1);
	dist_accu = distance_to_cable(0);
	current_single=current(1);
	current_accu=current(0);

	if((dist_accu < 0)||(dist_single < 0)){
		/* Clear the display */
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_FillRect(0, 0, X_SIZE, Y_OFFSET_COIL + 1);
		/* Write single and accurate measurement of wire */
	    BSP_LCD_SetFont(&Font24);
		BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		char text[16];
		BSP_LCD_DisplayStringAt(5,10, (uint8_t *)"Wire", LEFT_MODE);
		BSP_LCD_SetFont(&Font20);
		BSP_LCD_DisplayStringAt(5,50, (uint8_t *)"Single", LEFT_MODE);
		BSP_LCD_SetFont(&Font16);
		snprintf(text, 15, "Distance: %4d", (int)(dist_single));
		BSP_LCD_DisplayStringAt(5, 70, (uint8_t *)text, LEFT_MODE);
		BSP_LCD_SetFont(&Font20);
		BSP_LCD_DisplayStringAt(5,110, (uint8_t *)"Accurate", LEFT_MODE);
		BSP_LCD_SetFont(&Font16);
		snprintf(text, 15, "Distance: %4d", (int)(dist_accu));
		BSP_LCD_DisplayStringAt(5, 130, (uint8_t *)text, LEFT_MODE);
		BSP_LCD_SetFont(&Font24);
		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		BSP_LCD_DisplayStringAt(5,180, (uint8_t *)"OUT OF RANGE", CENTER_MODE);

	}
	else{
	/* Clear the display */
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(0, 0, X_SIZE, Y_OFFSET_COIL + 1);
	/* Write single and accurate measurement of wire */
    BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	char text[16];
	BSP_LCD_DisplayStringAt(5,10, (uint8_t *)"Wire", LEFT_MODE);
	BSP_LCD_SetFont(&Font20);
	BSP_LCD_DisplayStringAt(5,50, (uint8_t *)"Single", LEFT_MODE);
	BSP_LCD_SetFont(&Font16);
	snprintf(text, 15, "Distance: %4d", (int)(dist_single));
	BSP_LCD_DisplayStringAt(5, 70, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 15, "Current:  %4d", (int)(current_single));
	BSP_LCD_DisplayStringAt(5, 85, (uint8_t *)text, LEFT_MODE);
	BSP_LCD_SetFont(&Font20);
	BSP_LCD_DisplayStringAt(5,110, (uint8_t *)"Accurate", LEFT_MODE);
	BSP_LCD_SetFont(&Font16);
	snprintf(text, 15, "Distance: %4d", (int)(dist_accu));
	BSP_LCD_DisplayStringAt(5, 130, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 15, "Current:  %4d", (int)(current_accu));
	BSP_LCD_DisplayStringAt(5, 145, (uint8_t *)text, LEFT_MODE);
	BSP_LCD_SetFont(&Font12);
	BSP_LCD_DisplayStringAt(5,165, (uint8_t *)"Pad:", LEFT_MODE);
	BSP_LCD_DisplayStringAt(5,225, (uint8_t *)"Coil:", LEFT_MODE);

	/* draw value of pad1 in a graph */
	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	data = PAD1_samples[0] / f;
	for (uint32_t i = 1; i < ADC_NUMS; i++){
		data_last = data;
		data = (PAD1_samples[(i)]) / f;
			if (data > Y_OFFSET_PAD) { data = Y_OFFSET_PAD; }// Limit value, prevent crash
			BSP_LCD_DrawLine(4*(i-1), Y_OFFSET_PAD-data_last, 4*i, Y_OFFSET_PAD-data);
	}
	/* draw value of pad2 in a graph*/
		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		data = PAD2_samples[0] / f;
		for (uint32_t i = 1; i < ADC_NUMS; i++){
			data_last = data;
			data = (PAD2_samples[i]) / f;
			if (data > Y_OFFSET_PAD) { data = Y_OFFSET_PAD; }// Limit value, prevent crash
			BSP_LCD_DrawLine(4*(i-1), Y_OFFSET_PAD-data_last, 4*i, Y_OFFSET_PAD-data);
		}

	/* draw the value of coil1 in a graph */
	BSP_LCD_SetTextColor(LCD_COLOR_DARKCYAN);
	data = COIL1_samples[0] / f;
	for (uint32_t i = 1; i < ADC_NUMS; i++){
		data_last = data;
		data = (COIL1_samples[i]) / f;
		if (data > Y_OFFSET_COIL) { data = Y_OFFSET_COIL; }// Limit value, prevent crash
		BSP_LCD_DrawLine(4*(i-1), Y_OFFSET_COIL-data_last, 4*i, Y_OFFSET_COIL-data);
	}
		/* 	draw the value of coil 2 in a graph */

		BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
		data = COIL2_samples[0] / f;
		for (uint32_t i = 1; i < ADC_NUMS; i++){
			data_last = data;
			data = (COIL2_samples[i]) / f;
			if (data > Y_OFFSET_COIL) { data = Y_OFFSET_COIL; }// Limit value, prevent crash
			BSP_LCD_DrawLine(4*(i-1), Y_OFFSET_COIL-data_last, 4*i, Y_OFFSET_COIL-data);

		}
	}
	MEAS_CLEAR_buffer_flags();
}


/** **************************************************************************
 * @brief Function for displaying the cable data
 * @note  	This function calls the required functions to display the data for a cable measurement
 * @n		Clears the ADC_samples array after displaying all the data
 *****************************************************************************/

void DISP_show_data_cable(void)
{
	const uint32_t Y_OFFSET_PAD = 220;					//Offset for pad-graph
	const uint32_t Y_OFFSET_COIL = 280;					//0ffset for coil-graph
	const uint32_t X_SIZE = 240;
	const uint32_t f = (6 << ADC_DAC_RES) /	Y_OFFSET_COIL + 1;   	// Scaling factor
	uint32_t data;
	uint32_t data_last;

	current_single = current(1);
	current_accu = current(0);

	//Call for calculations
	dist_single = distance_to_cable(1);
	dist_accu = distance_to_cable(0);
	if((dist_accu < 0)||(dist_single < 0)){
		/* Clear the display */
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_FillRect(0, 0, X_SIZE, Y_OFFSET_COIL + 1);
		/* Write single and accurate measurement of wire */
	    BSP_LCD_SetFont(&Font24);
		BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
		BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
		char text[16];
		BSP_LCD_DisplayStringAt(5,10, (uint8_t *)"Cable", LEFT_MODE);
		BSP_LCD_SetFont(&Font20);
		BSP_LCD_DisplayStringAt(5,50, (uint8_t *)"Single", LEFT_MODE);
		BSP_LCD_SetFont(&Font16);
		snprintf(text, 15, "Distance: %4d", (int)(dist_single));
		BSP_LCD_DisplayStringAt(5, 70, (uint8_t *)text, LEFT_MODE);
		BSP_LCD_SetFont(&Font20);
		BSP_LCD_DisplayStringAt(5,110, (uint8_t *)"Accurate", LEFT_MODE);
		BSP_LCD_SetFont(&Font16);
		snprintf(text, 15, "Distance: %4d", (int)(dist_accu));
		BSP_LCD_DisplayStringAt(5, 130, (uint8_t *)text, LEFT_MODE);
		BSP_LCD_SetFont(&Font24);
		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		BSP_LCD_DisplayStringAt(5,180, (uint8_t *)"OUT OF RANGE", CENTER_MODE);

	}
	else{

	/* Clear the display */
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(0, 0, X_SIZE, Y_OFFSET_COIL + 1);
	/* Write single and accurate measurement of wire */
    BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	char text[16];
	BSP_LCD_DisplayStringAt(5,10, (uint8_t *)"Cable", LEFT_MODE);
	BSP_LCD_SetFont(&Font20);
	BSP_LCD_DisplayStringAt(5,50, (uint8_t *)"Single", LEFT_MODE);
	BSP_LCD_SetFont(&Font16);
	snprintf(text, 15, "Distance: %4d", (int)(dist_single));
	BSP_LCD_DisplayStringAt(5, 70, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 15, "Current:  %4d", (int)(current_single));
	BSP_LCD_DisplayStringAt(5, 85, (uint8_t *)text, LEFT_MODE);
	BSP_LCD_SetFont(&Font20);
	BSP_LCD_DisplayStringAt(5,110, (uint8_t *)"Accurate", LEFT_MODE);
	BSP_LCD_SetFont(&Font16);
	snprintf(text, 15, "Distance: %4d", (int)(dist_accu));
	BSP_LCD_DisplayStringAt(5, 130, (uint8_t *)text, LEFT_MODE);
	snprintf(text, 15, "Current:  %4d", (int)(current_accu));
	BSP_LCD_DisplayStringAt(5, 145, (uint8_t *)text, LEFT_MODE);
	BSP_LCD_SetFont(&Font12);
	BSP_LCD_DisplayStringAt(5,165, (uint8_t *)"Pad:", LEFT_MODE);
	BSP_LCD_DisplayStringAt(5,225, (uint8_t *)"Coil:", LEFT_MODE);

	/* draw value of pad1 in a graph */
	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	data = PAD1_samples[0] / f;
	for (uint32_t i = 1; i < ADC_NUMS; i++){
		data_last = data;
		data = (PAD1_samples[(i)]) / f;
			if (data > Y_OFFSET_PAD) { data = Y_OFFSET_PAD; }// Limit value, prevent crash
			BSP_LCD_DrawLine(4*(i-1), Y_OFFSET_PAD-data_last, 4*i, Y_OFFSET_PAD-data);
	}
	/* draw value of pad2 in a graph*/
		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		data = PAD2_samples[0] / f;
		for (uint32_t i = 1; i < ADC_NUMS; i++){
			data_last = data;
			data = (PAD2_samples[i]) / f;
			if (data > Y_OFFSET_PAD) { data = Y_OFFSET_PAD; }// Limit value, prevent crash
			BSP_LCD_DrawLine(4*(i-1), Y_OFFSET_PAD-data_last, 4*i, Y_OFFSET_PAD-data);
		}

	/* draw the value of coil1 in a graph */
	BSP_LCD_SetTextColor(LCD_COLOR_DARKCYAN);
	data = COIL1_samples[0] / f;
	for (uint32_t i = 1; i < ADC_NUMS; i++){
		data_last = data;
		data = (COIL1_samples[i]) / f;
		if (data > Y_OFFSET_COIL) { data = Y_OFFSET_COIL; }// Limit value, prevent crash
		BSP_LCD_DrawLine(4*(i-1), Y_OFFSET_COIL-data_last, 4*i, Y_OFFSET_COIL-data);
	}
		/* 	draw the value of coil 2 in a graph */

		BSP_LCD_SetTextColor(LCD_COLOR_ORANGE);
		data = COIL2_samples[0] / f;
		for (uint32_t i = 1; i < ADC_NUMS; i++){
			data_last = data;
			data = (COIL2_samples[i]) / f;
			if (data > Y_OFFSET_COIL) { data = Y_OFFSET_COIL; }// Limit value, prevent crash
			BSP_LCD_DrawLine(4*(i-1), Y_OFFSET_COIL-data_last, 4*i, Y_OFFSET_COIL-data);

		}
	}
	MEAS_CLEAR_buffer_flags();
}


/** **************************************************************************
 * @brief Function for displaying the angle data
 * @note  	This function calls the required functions to display the data for an angle measurement.
 * @n		Shows 2 dots on the screen for visualization of the direction
 * @n		Shows an error on display if the data is unclear
 * @n		Clears the ADC_samples array after displaying all the data
 *****************************************************************************/

void DISP_show_data_angle(void)
{
	const uint32_t X_SIZE = 240;
	const uint32_t Y_OFFSET_PAD = 280;
	char text[16];

	int32_t angle = 666;
	angle = angle_to_cable();

	/* Clear the display */
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_FillRect(0, 0, X_SIZE, Y_OFFSET_PAD + 1);
	/* Write the measurement of the angle */
	BSP_LCD_SetFont(&Font24);
	BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
	BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
	BSP_LCD_DisplayStringAt(5,10, (uint8_t *)"Angle", LEFT_MODE);
	BSP_LCD_SetFont(&Font20);
	BSP_LCD_DisplayStringAt(5,50, (uint8_t *)"Value in Degree", LEFT_MODE);

	//Two points for a rough indication of direction
	BSP_LCD_DrawCircle(45,220,20);
	BSP_LCD_DrawCircle(195,220,20);

	//Color middle part for direction / error
	if(CALC_degree_left){
		CALC_degree_left = false;
		snprintf(text, 15, "Angle:  %4d", (int)(angle));
		BSP_LCD_DisplayStringAt(5, 90, (uint8_t *)text, LEFT_MODE);
		BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
		BSP_LCD_FillCircle(45,220,10);
	}
	else if(CALC_degree_right){
		CALC_degree_right = false;
		snprintf(text, 15, "Angle: %4d", (int)(angle));
		BSP_LCD_DisplayStringAt(5, 90, (uint8_t *)text, LEFT_MODE);
		BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
		BSP_LCD_FillCircle(195,220,10);
	}
	else{
		BSP_LCD_SetTextColor(LCD_COLOR_RED);
		BSP_LCD_DisplayStringAt(5,90, (uint8_t *)"NO VALUE", CENTER_MODE);
		BSP_LCD_FillCircle(120,150,10);
		BSP_LCD_FillCircle(45,220,10);
		BSP_LCD_FillCircle(195,220,10);
	}

	MEAS_CLEAR_buffer_flags();
}
