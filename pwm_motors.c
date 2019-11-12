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
#include "pwm_motors.h"

/*From clock setup 0 in system_MK64f12.c*/
#define DEFAULT_SYSTEM_CLOCK 20485760u /* Default System clock value */
#define CLOCK					       20485760u
#define PWM0_FREQUENCY			   10000
#define FTM0_MOD_VALUE			 (CLOCK/PWM0_FREQUENCY)

static volatile unsigned int PWMTick = 0;

/*
 * Change the Motor Duty Cycle and Frequency
 * @param DutyCycle (0 to 100)
 * @param Frequency (~1000 Hz to 20000 Hz)
 * @param dir: 1 for C4 active, else C3 active 
 */
void SetDutyCycleL(unsigned int DutyCycle, unsigned int Frequency, int dir)
{
	// Calculate the new cutoff value
	uint16_t mod = (uint16_t) (((CLOCK/Frequency) * DutyCycle) / 100);
  
	// Set outputs 
	if(dir==1)
    {FTM0_C3V = mod; FTM0_C2V=0;}
		
  else
    {FTM0_C2V = mod; FTM0_C3V=0;}

	// Update the clock to the new frequency
	FTM0_MOD = (CLOCK/Frequency);
}

void SetDutyCycleR(unsigned int DutyCycle, unsigned int Frequency, int dir)
{
	// Calculate the new cutoff value
	uint16_t mod = (uint16_t) (((CLOCK/Frequency) * DutyCycle) / 100);
  
	// Set outputs 
	if(dir==1)
    {FTM0_C1V = mod; FTM0_C0V=0;}
		
  else
    {FTM0_C0V = mod; FTM0_C1V=0;}

	// Update the clock to the new frequency
	FTM0_MOD = (CLOCK/Frequency);
}

/*
 * Initialize the FlexTimer for PWM
 */
void InitPWM0()
{
	// 12.2.13 Enable the Clock to the FTM0 Module
	SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;
	
	// Enable clock on PORT C so it can output
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK;
	
	// 11.4.1 Route the output of FTM channel 0 to the pins
	// Use drive strength enable flag to high drive strength
	// These port/pins may need to be updated for the K64 <Yes, they do. Here are two that work.>
	
	PORTC_PCR1  = PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK; //Ch2
	PORTC_PCR2  = PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK; //Ch3
	PORTC_PCR3  = PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK; //Ch2
	PORTC_PCR4  = PORT_PCR_MUX(4) | PORT_PCR_DSE_MASK; //Ch3
	
	// 39.3.10 Disable Write Protection
	FTM0_MODE |= FTM_MODE_WPDIS_MASK;
	
	// 39.3.4 FTM Counter Value
	// Initialize the CNT to 0 before writing to MOD
	FTM0_CNT = 0;
	
	// 39.3.8 Set the Counter Initial Value to 0
	FTM0_CNTIN = 0;
	
	// 39.3.5 Set the Modulo resister
	FTM0_MOD = FTM0_MOD_VALUE; //FTM0->MOD = (DEFAULT_SYSTEM_CLOCK/(1<<7))/1000;

	// 39.3.6 Set the Status and Control of both channels
	// Used to configure mode, edge and level selection
	// See Table 39-67,  Edge-aligned PWM, High-true pulses (clear out on match)
	FTM0_C3SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0_C3SC &= ~FTM_CnSC_ELSA_MASK;
	
	// See Table 39-67,  Edge-aligned PWM, Low-true pulses (clear out on match)
	FTM0_C2SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0_C2SC &= ~FTM_CnSC_ELSA_MASK;
	
	FTM0_C1SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0_C1SC &= ~FTM_CnSC_ELSA_MASK;
	
	// See Table 39-67,  Edge-aligned PWM, Low-true pulses (clear out on match)
	FTM0_C0SC |= FTM_CnSC_MSB_MASK | FTM_CnSC_ELSB_MASK;
	FTM0_C0SC &= ~FTM_CnSC_ELSA_MASK;
	
	// 39.3.3 FTM Setup
	// Set prescale value to 1 
	// Chose system clock source
	// Timer Overflow Interrupt Enable
	FTM0_SC = FTM_SC_PS(0) | FTM_SC_CLKS(1); 
	//| FTM_SC_TOIE_MASK;

	// Enable Interrupt Vector for FTM
  // NVIC_EnableIRQ(FTM0_IRQn);
}
