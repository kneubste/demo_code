/** ***************************************************************************
 * @file
 * @brief See measuring.c
 *
 * Prefixes MEAS, ADC, DAC
 *
 * @author Stefan Kneubühl, kneubste@students.zhaw.ch
 * @author Niklaus Schoch,	schocnik@students.zhaw.ch
 * @date 30.11.2021
 *****************************************************************************/

#ifndef MEAS_H_
#define MEAS_H_


/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdbool.h>


/******************************************************************************
 * Defines
 *****************************************************************************/
#define ADC_DAC_RES     12          ///< Resolution
#define ADC_NUMS        60          ///< Number of samples
#define INPUTS_NUMS     4		    ///< Number of inputs
#define MEAS_RES		12			///< Resolution in bits

extern bool MEAS_data_ready;

extern bool MEAS_data_wire;				///< Allow for wire data displaying
extern bool MEAS_data_cable;			///< Allow for cable data displaying
extern bool MEAS_data_angle;			///< Allow for angle data displaying

extern uint32_t MEAS_input_count;

extern bool DAC_active;

extern uint32_t ADC_samples[ADC_NUMS*INPUTS_NUMS];	///< ADC values of max. 4 input channels
extern int32_t PAD1_samples[ADC_NUMS];		///< Array for the PAD1 samples for calculation and displaying
extern int32_t PAD2_samples[ADC_NUMS];		///< Array for the PAD2 samples for calculation and displaying
extern int32_t COIL1_samples[ADC_NUMS];		///< Array for the COIL1 samples for calculation and displaying
extern int32_t COIL2_samples[ADC_NUMS];		///< Array for the COIL2 samples for calculation and displaying


/******************************************************************************
 * Functions
 *****************************************************************************/
void MEAS_GPIO_analog_init(void);
void MEAS_timer_init(void);
void DAC_reset(void);
void DAC_init(void);
void DAC_increment(void);
void ADC_reset(void);
void MEAS_CLEAR_buffer_flags(void);
void MEAS_sort_data(void);
void ADC3_scan_init(void);
void ADC3_scan_start(void);

void ADC1_IN13_ADC2_IN5_dual_init(void);
void ADC1_IN13_ADC2_IN5_dual_start(void);

#endif
