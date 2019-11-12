/*
 * Pulse-Width-Modulation Code for K64
 * PWM signal can be connected to output pins PC3 and PC4
 * 
 * Author: Brent Dimmig <bnd8678@rit.edu>
 * Modified by: 
 * Created: 2/20/2014
 * Modified: 3/07/2015
 */
#include "MK64F12.h"
#include "pwm_servo.h"

/*From clock setup 0 in system_MK64f12.c*/
#define DEFAULT_SYSTEM_CLOCK 20485760u /* Default System clock value */
#define CLOCK					       20485760u
#define PWM3_FREQUENCY			 50                
#define FTM3_MOD_VALUE			 (CLOCK/128/PWM3_FREQUENCY)

static volatile unsigned int PWMTick = 0;

/*
 * Change the SERVO Duty Cycle and Frequency
 * @param DutyCycle (0 to 100)
 * @param Frequency (~1000 Hz to 20000 Hz)
 * @param dir: 1 for C4 active, else C3 active 
 */
void SetDutyCycleServo(unsigned int DutyCycle, unsigned int Frequency)
{
	// Calculate the new cutoff value
 
  FTM3_C4V = (uint16_t) (((CLOCK/128/Frequency) * DutyCycle) / 100);
	
	// Update the clock to the new frequency
	FTM3_MOD = FTM3_MOD_VALUE;
}

/*
 * Initialize the FlexTimer for PWM
 */
void InitPWM3()
{
	// 12.2.13 Enable the Clock to the FTM3 Module
	SIM_SCGC3 |= SIM_SCGC3_FTM3_MASK;
	
	// Enable clock on PORT A so it can output
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
	
	// 11.4.1 Route the output of FTM channel 0 to the pins
	// Use drive strength enable flag to high drive strength
	//These port/pins may need to be updated for the K64 <Yes, they do. Here are two that work.>
	
	PORTC_PCR8 = PORT_PCR_MUX(3)| PORT_PCR_DSE_MASK; //Ch2
		
	// 39.3.10 Disable Write Protection
	FTM3_MODE |= FTM_MODE_WPDIS_MASK;
	
	// 39.3.4 FTM Counter Value
	// Initialize the CNT to 0 before writing to MOD
	FTM3_CNT = 0;
	
	// 39.3.8 Set the Counter Initial Value to 0
	FTM3_CNTIN = 0;
	
	// 39.3.5 Set the Modulo resister
	FTM3_MOD = FTM3_MOD_VALUE;
	//FTM3->MOD = (DEFAULT_SYSTEM_CLOCK/(1<<7))/1000;

	// 39.3.6 Set the Status and Control of both channels
	// Used to configure mode, edge and level selection
	// See Table 39-67,  Edge-aligned PWM, High-true pulses (clear out on match)
	FTM3_C4SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM3_C4SC &= ~FTM_CnSC_ELSA_MASK;
	
	// 39.3.3 FTM Setup
	// Set prescale value to 1 
	// Chose system clock source
	// Timer Overflow Interrupt Enable
	FTM3_SC = FTM_SC_PS(7) | FTM_SC_CLKS(1); 
	//| FTM_SC_TOIE_MASK;

	// Enable Interrupt Vector for FTM
  //NVIC_EnableIRQ(FTM3_IRQn);
}

