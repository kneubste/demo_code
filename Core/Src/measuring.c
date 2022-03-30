/** ***************************************************************************
 * @file
 * @brief Measuring voltages with the ADC(s) in different configurations.
 *
 * ==============================================================
 *
 * - ADC in single conversion mode
 * - ADC triggered by a timer and with interrupt after end of conversion
 * - ADC combined with DMA (Direct Memory Access) to fill a buffer
 * - Dual mode = simultaneous sampling of two inputs by two ADCs
 * - Scan mode = sequential sampling of two inputs by one ADC
 * - Simple DAC output is demonstrated as well
 * - Analog mode configuration for GPIOs
 * - Display recorded data on the graphics display
 *
 * Peripherals @ref HowTo
 *
 * @image html demo_screenshot_board.jpg
 *
 *
 * HW used for the demonstrations
 * ==============================
 * A simple HW was used for testing the code.
 * It is connected to the pins marked in red in the above image.
 *
 * @image html demo_board_schematic.png
 *
 * Of course the code runs also without this HW.
 * Simply connect a signal or waveform generator to the input(s).
 *
 *
 * @anchor HowTo
 * How to Configure the Peripherals: ADC, TIMER and DMA
 * ====================================================
 *
 * All the peripherals are accessed by writing to or reading from registers.
 * From the programmer’s point of view this is done exactly as
 * writing or reading the value of a variable.
 * @n Writing to a register configures the HW of the associated peripheral
 * to do what is required.
 * @n Reading from a registers gets status and data from the HW peripheral.
 *
 * The information on which bits have to be set to get a specific behavior
 * is documented in the <b>reference manual</b> of the mikrocontroller.
 *
 *
 * ----------------------------------------------------------------------------
 * @author Hanspeter Hochreutener, hhrt@zhaw.ch
 * @date 17.06.2021
 *****************************************************************************/


/******************************************************************************
 * ADC inputs
 * PF6 = ADC3_IN4 		= PAD_LEFT
 * PC3 = ADC123_IN13 	= PAD_RIGHT
 * PF8 = ADC3_IN6 		= COIL_LEFT
 * PC1 = ADC123_IN11 	= COIL_RIGHT
 *
 * VIRTUAL_GROUND = VCC/2
 *
 * PA5 = ADC12_IN5 = DAC_OUT2
 *****************************************************************************/





/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdio.h>
#include "stm32f4xx.h"
#include "stm32f429i_discovery.h"
#include "stm32f429i_discovery_lcd.h"
#include "stm32f429i_discovery_ts.h"

#include "measuring.h"
#include "displayingdata.h"
#include "calculations.h"

/******************************************************************************
 * Defines
 *****************************************************************************/
 #define ADC_FS          600 			///< Sampling freq. => 12 samples for a 50Hz period
 #define ADC_CLOCK       84000000    	///< APB2 peripheral clock frequency
 #define ADC_CLOCKS_PS   15          	///< Clocks/sample: 3 hold + 12 conversion
 #define TIM_CLOCK       84000000    	///< APB1 timer clock frequency
 #define TIM_TOP         9          	///< Timer top value
 #define TIM_PRESCALE    (TIM_CLOCK/ADC_FS/(TIM_TOP+1)-1) ///< Clock prescaler

/******************************************************************************
 * Variables
 *****************************************************************************/
bool MEAS_data_ready = false;			///< New data is ready
uint32_t MEAS_input_count = 1;			///< Number of input ports
bool DAC_active = false;				///< DAC output active?

static uint32_t ADC_sample_count = 0;  		///< Index for buffer
uint32_t ADC_samples[ADC_NUMS*INPUTS_NUMS]; ///< ADC values of max. 4 input channels
static uint32_t DAC_sample = 0;         	///< DAC output value

int32_t PAD1_samples[ADC_NUMS];				///< Array for the PAD1 samples for calculation and/or displaying
int32_t PAD2_samples[ADC_NUMS];				///< Array for the PAD2 samples for calculation and/or displaying
int32_t COIL1_samples[ADC_NUMS];			///< Array for the COIL1 samples for calculation and/or displaying
int32_t COIL2_samples[ADC_NUMS];			///< Array for the COIL2 samples for calculation and/or displaying

uint32_t ADC_buffer[INPUTS_NUMS*ADC_NUMS];	///< Buffer for ADC samples

/******************************************************************************
 * Functions
 *****************************************************************************/

/** ***************************************************************************
 * @brief Configure GPIOs in analog mode.
 *
 * @note The input number for the ADCs is NOT equal to the GPIO pin number!
 * - ADC3_IN4 		= GPIO PF6 = PAD_LEFT
 * - ADC123_IN13 	= GPIO PC3 = PAD_RIGHT
 * - ADC3_IN6 		= GPIO PF8 = COIL_LEFT
 * - ADC123_IN11 	= GPIO PC1 = COIL_RIGHT
 * - ADC12_IN5 		= GPIO PA5
 * - DAC_OUT2 		= GPIO PA5 (= same GPIO as ADC12_IN5)
 *****************************************************************************/
void MEAS_GPIO_analog_init(void)
{

	__HAL_RCC_GPIOC_CLK_ENABLE();		// Enable Clock for GPIO port C
	__HAL_RCC_GPIOF_CLK_ENABLE();		// Enable Clock for GPIO port F
	/* Now configure the analog inputs */
	GPIOF->MODER |= GPIO_MODER_MODER6_Msk;	// Analog mode for PF6 = ADC3_IN4
	GPIOC->MODER |= GPIO_MODER_MODER3_Msk;	// Analog mode PC3 = ADC123_IN13
	GPIOF->MODER |= GPIO_MODER_MODER8_Msk;	// Analog mode for PF8 = ADC3_IN6
	GPIOC->MODER |= GPIO_MODER_MODER1_Msk;	// Analog mode PC1 = ADC123_IN11
	__HAL_RCC_GPIOA_CLK_ENABLE();		// Enable Clock for GPIO port A
	GPIOA->MODER |= GPIO_MODER_MODER5_Msk;	// Analog mode for PA5 ADC12_IN5
}


/** ***************************************************************************
 * @brief Resets the DAC
 *
 * when it is no longer used.
 *****************************************************************************/
void DAC_reset(void) {
	RCC->APB1RSTR |= RCC_APB1RSTR_DACRST;	// Reset the DAC
	RCC->APB1RSTR &= ~RCC_APB1RSTR_DACRST;	// Release reset of the DAC
}


/** ***************************************************************************
 * @brief Initialize the DAC
 *
 * The output used is DAC_OUT2 = GPIO PA5
 * @n As DAC_OUT2 = GPIO PA5 (= same GPIO as ADC12_IN5)
 * it is possible to monitor the output voltage DAC_OUT2 by the input ADC12_IN5.
 *****************************************************************************/
void DAC_init(void)
{
	__HAL_RCC_DAC_CLK_ENABLE();			// Enable Clock for DAC
	DAC->CR |= DAC_CR_EN2;				// Enable DAC output 2
}


/** ***************************************************************************
 * @brief Increment the DAC value and write it to the output
 *
 *****************************************************************************/
void DAC_increment(void) {
	DAC_sample += 20;				// Increment DAC output
	if (DAC_sample >= (1UL << ADC_DAC_RES)) { DAC_sample = 0; }	// Go to 0
	DAC->DHR12R2 = DAC_sample;		// Write new DAC output value
}


/** ***************************************************************************
 * @brief Resets the ADCs and the timer
 *
 * to make sure the different demos do not interfere.
 *****************************************************************************/
void ADC_reset(void) {
	RCC->APB2RSTR |= RCC_APB2RSTR_ADCRST;	// Reset ADCs
	RCC->APB2RSTR &= ~RCC_APB2RSTR_ADCRST;	// Release reset of ADCs
	TIM2->CR1 &= ~TIM_CR1_CEN;				// Disable timer
}


/** ***************************************************************************
 * @brief Configure the timer to trigger the ADC(s)
 *
 * @note For debugging purposes the timer interrupt might be useful.
 *****************************************************************************/
void MEAS_timer_init(void)
{
	__HAL_RCC_TIM2_CLK_ENABLE();		// Enable Clock for TIM2
	TIM2->PSC = TIM_PRESCALE;			// Prescaler for clock freq. = 1MHz
	TIM2->ARR = TIM_TOP;				// Auto reload = counter top value
	TIM2->CR2 |= TIM_CR2_MMS_1; 		// TRGO on update
	/* If timer interrupt is not needed, comment the following lines */
	TIM2->DIER |= TIM_DIER_UIE;			// Enable update interrupt
	NVIC_ClearPendingIRQ(TIM2_IRQn);	// Clear pending interrupt on line 0
	NVIC_EnableIRQ(TIM2_IRQn);			// Enable interrupt line 0 in the NVIC
}

/** ***************************************************************************
 * @brief Interrupt handler for the timer 2
 *
 * @note This interrupt handler was only used for debugging purposes
 * and to increment the DAC value.
 *****************************************************************************/
void TIM2_IRQHandler(void)
{
	TIM2->SR &= ~TIM_SR_UIF;			// Clear pending interrupt flag
	if (DAC_active) {
		DAC_increment();
	}
}


/** ***************************************************************************
 * @brief Interrupt handler for the ADCs
 *
 * Reads one sample from the ADC3 DataRegister and transfers it to a buffer.
 * @n Stops when ADC_NUMS samples have been read.
 *****************************************************************************/
void ADC_IRQHandler(void)
{
	if (ADC3->SR & ADC_SR_EOC) {		// Check if ADC3 end of conversion
		ADC_samples[ADC_sample_count++] = ADC3->DR;	// Read input channel 1 only
		if (ADC_sample_count >= ADC_NUMS) {		// Buffer full
			TIM2->CR1 &= ~TIM_CR1_CEN;	// Disable timer
			ADC3->CR2 &= ~ADC_CR2_ADON;	// Disable ADC3
			ADC_reset();
			MEAS_data_ready = true;
		}

	}
}


/** ***************************************************************************
 * @brief Interrupt handler for DMA2 Stream1
 *
 * The samples from the ADC3 have been transfered to memory by the DMA2 Stream1
 * and are ready for processing.
 *****************************************************************************/
void DMA2_Stream1_IRQHandler(void)
{
	if (DMA2->LISR & DMA_LISR_TCIF1) {	// Stream1 transfer compl. interrupt f.
		NVIC_DisableIRQ(DMA2_Stream1_IRQn);	// Disable DMA interrupt in the NVIC
		NVIC_ClearPendingIRQ(DMA2_Stream1_IRQn);// Clear pending DMA interrupt
		DMA2_Stream1->CR &= ~DMA_SxCR_EN;	// Disable the DMA
		while (DMA2_Stream1->CR & DMA_SxCR_EN) { ; }	// Wait for DMA to finish
		DMA2->LIFCR |= DMA_LIFCR_CTCIF1;// Clear transfer complete interrupt fl.
		TIM2->CR1 &= ~TIM_CR1_CEN;		// Disable timer
		ADC3->CR2 &= ~ADC_CR2_ADON;		// Disable ADC3
		ADC3->CR2 &= ~ADC_CR2_DMA;		// Disable DMA mode
		ADC_reset();
		MEAS_data_ready = true;
	}
}


/** ***************************************************************************
 * @brief Interrupt handler for DMA2 Stream3
 *
 * The samples from the ADC3 have been transfered to memory by the DMA2 Stream1
 * and are ready for processing.
 *****************************************************************************/
void DMA2_Stream3_IRQHandler(void)
{
	if (DMA2->LISR & DMA_LISR_TCIF3) {	// Stream3 transfer compl. interrupt f.
		NVIC_DisableIRQ(DMA2_Stream3_IRQn);	// Disable DMA interrupt in the NVIC
		NVIC_ClearPendingIRQ(DMA2_Stream3_IRQn);// Clear pending DMA interrupt
		DMA2_Stream3->CR &= ~DMA_SxCR_EN;	// Disable the DMA
		while (DMA2_Stream3->CR & DMA_SxCR_EN) { ; }	// Wait for DMA to finish
		DMA2->LIFCR |= DMA_LIFCR_CTCIF3;// Clear transfer complete interrupt fl.
		TIM2->CR1 &= ~TIM_CR1_CEN;		// Disable timer
		ADC2->CR2 &= ~ADC_CR2_ADON;		// Disable ADC2
		ADC2->CR2 &= ~ADC_CR2_DMA;		// Disable DMA mode
		ADC_reset();
		MEAS_data_ready = true;
	}
}


/** ***************************************************************************
 * @brief Interrupt handler for DMA2 Stream4
 *
 * Here the interrupt handler is used together with ADC1 and ADC2
 * in dual mode where they sample simultaneously.
 * @n The samples from both ADCs packed in a 32 bit word have been transfered
 * to memory by the DMA2 and are ready for unpacking.
 * @note In dual ADC mode two values are combined (packed) in a single uint32_t
 * ADC_CDR[31:0] = ADC2_DR[15:0] | ADC1_DR[15:0]
 * and are therefore extracted before further processing.
 *****************************************************************************/
void DMA2_Stream4_IRQHandler(void)
{
	if (DMA2->HISR & DMA_HISR_TCIF4) {	// Stream4 transfer compl. interrupt f.
		NVIC_DisableIRQ(DMA2_Stream4_IRQn);	// Disable DMA interrupt in the NVIC
		NVIC_ClearPendingIRQ(DMA2_Stream4_IRQn);// Clear pending DMA interrupt
		DMA2_Stream4->CR &= ~DMA_SxCR_EN;	// Disable the DMA
		while (DMA2_Stream4->CR & DMA_SxCR_EN) { ; }	// Wait for DMA to finish
		DMA2->HIFCR |= DMA_HIFCR_CTCIF4;// Clear transfer complete interrupt fl.
		TIM2->CR1 &= ~TIM_CR1_CEN;		// Disable timer
		ADC1->CR2 &= ~ADC_CR2_ADON;		// Disable ADC1
		ADC2->CR2 &= ~ADC_CR2_ADON;		// Disable ADC2
		ADC->CCR &= ~ADC_CCR_DMA_1;		// Disable DMA mode
		/* Extract combined samples */
		for (int32_t i = ADC_NUMS-1; i >= 0; i--){
			ADC_samples[2*i+1] = (ADC_samples[i] >> 16);
			ADC_samples[2*i]   = (ADC_samples[i] & 0xffff);
		}
		ADC_reset();
		MEAS_data_ready = true;
	}
}


/** ***************************************************************************
 * @brief Clears buffer and flag
 * @n This function get's called after the data is displayed
 * @n Only the ADC_samples array gets set to 0;
 *****************************************************************************/
void MEAS_CLEAR_buffer_flags(void)
{
	/* Clear buffer and flag */
	for (uint32_t i = 0; i < (ADC_NUMS*INPUTS_NUMS); i++){
		ADC_samples[i] = 0;
	}
	ADC_sample_count = 0;
}


/** ***************************************************************************
 * @brief Initialize ADC, timer and DMA for sequential acquisition = scan mode
 *
 * Uses ADC3 and DMA2_Stream1 channel2
 * @n The ADC3 trigger is set to TIM2 TRGO event
 * @n At each trigger both inputs are converted sequentially
 * and transfered to memory by the DMA.
 * @n As each conversion triggers the DMA, the number of transfers is doubled.
 * @n The DMA triggers the transfer complete interrupt when all data is ready.
 * @n All 4 inputs are scanned and put into the ADC_samples array
 *****************************************************************************/
void ADC3_scan_init(void)
{

	__HAL_RCC_DMA2_CLK_ENABLE();		// Enable Clock for DMA2
	DMA2_Stream1->CR &= ~DMA_SxCR_EN;	// Disable the DMA stream 1
	while (DMA2_Stream1->CR & DMA_SxCR_EN) { ; }	// Wait for DMA to finish
	DMA2->LIFCR |= DMA_LIFCR_CTCIF1;	// Clear transfer complete interrupt fl.
	DMA2_Stream1->CR |= DMA_SxCR_CHSEL_1;	// Select channel 2
	DMA2_Stream1->CR |= DMA_SxCR_PL_1;		// Priority high
	DMA2_Stream1->CR |= DMA_SxCR_MSIZE_1;	// Memory data size = 32 bit
	DMA2_Stream1->CR |= DMA_SxCR_PSIZE_1;	// Peripheral data size = 32 bit
	DMA2_Stream1->CR |= DMA_SxCR_MINC;	// Increment memory address pointer
	DMA2_Stream1->CR |= DMA_SxCR_TCIE;	// Transfer complete interrupt enable
	DMA2_Stream1->NDTR = INPUTS_NUMS*ADC_NUMS;		// Number of data items to transfer
	DMA2_Stream1->PAR = (uint32_t)&ADC3->DR;	// Peripheral register address
	DMA2_Stream1->M0AR = (uint32_t)ADC_samples;	// Buffer memory loc. address
	DMA2_Stream1->CR |= DMA_SxCR_EN;	// Enable DMA
	NVIC_ClearPendingIRQ(DMA2_Stream1_IRQn);	// Clear pending DMA interrupt
	NVIC_EnableIRQ(DMA2_Stream1_IRQn);	// Enable DMA interrupt in the NVIC
	__HAL_RCC_ADC3_CLK_ENABLE();		// Enable Clock for ADC3
	ADC3->SQR1 |= ADC_SQR1_L_0 | ADC_SQR1_L_1;	// Convert 4 inputs
	ADC3->SQR3 |= ( 4UL << ADC_SQR3_SQ1_Pos);	// Input  4 = 1. conversion Pad Left
	ADC3->SQR3 |= (13UL << ADC_SQR3_SQ2_Pos);	// Input 13 = 2. conversion Pad Right
	ADC3->SQR3 |= ( 6UL << ADC_SQR3_SQ3_Pos);	// Input  6 = 3. conversion Coil Left
	ADC3->SQR3 |= (11UL << ADC_SQR3_SQ4_Pos);	// Input 11 = 4. conversion Coil Right
	ADC3->CR1 |= ADC_CR1_SCAN;			// Enable scan mode
	ADC3->CR2 |= ADC_CR2_EXTEN_0;		// Enable external trigger on rising e.
	ADC3->CR2 |= ADC_CR2_EXTSEL_1 | ADC_CR2_EXTSEL_2; // Timer 2 TRGO event
	ADC3->CR2 |= ADC_CR2_DMA;			// Enable DMA mode
	ADC3->CR2 |= ADC_CR2_ADON;			// Enable ADC3

}


/** ***************************************************************************
 * @brief Start DMA, ADC and timer
 * @n pads
 *****************************************************************************/
void ADC3_scan_start(void)
{
	DMA2_Stream1->CR |= DMA_SxCR_EN;	// Enable DMA
	NVIC_ClearPendingIRQ(DMA2_Stream1_IRQn);	// Clear pending DMA interrupt
	NVIC_EnableIRQ(DMA2_Stream1_IRQn);	// Enable DMA interrupt in the NVIC
	ADC3->CR2 |= ADC_CR2_ADON;			// Enable ADC3
	TIM2->CR1 |= TIM_CR1_CEN;			// Enable timer
}


/** ***************************************************************************
 * @brief Sorts the data from ADC_samples to a array for each input
 * @note	  The array has the same size as ADC_NUMS = 60
 *****************************************************************************/
void MEAS_sort_data(void){
	for(int i=0;i<ADC_NUMS;i++){

		PAD1_samples[i]=ADC_samples[(4*i)];
		PAD2_samples[i]=ADC_samples[1+((4*i))];
		COIL1_samples[i]=ADC_samples[2+(4*i)];
		COIL2_samples[i]=ADC_samples[3+(4*i)];
	}
}