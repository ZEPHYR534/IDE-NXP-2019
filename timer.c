/*
* Rochester Institute of Technology
* Department of Computer Engineering
* CMPE 460  Interfacing Digital Electronics
* Spring 2016
*
* Filename: main_timer_template.c
*/

#include "MK64F12.h"
#include "uart.h"
#include "isr.h"
#include <stdio.h>

/*From clock setup 0 in system_MK64f12.c*/
#define DEFAULT_SYSTEM_CLOCK 20485760u /* Default System clock value */

void initPDB(void);
void initGPIO(void);
void initFTM(void);
void initInterrupts(void);



void initPDB(void){
	//Enable clock for PDB module
	SIM_SCGC6 |= SIM_SCGC6_PDB_MASK;

	// Set continuous mode, prescaler of 128, multiplication factor of 20,
	// software triggering, and PDB enabled

	PDB0_SC |= PDB_SC_CONT_MASK;       // set continuous mode
	PDB0_SC |= PDB_SC_PRESCALER(0x7);  // set prescaler to 128
	PDB0_SC |= PDB_SC_MULT(2);         // set multiplier to 2 => 20
	PDB0_SC |= PDB_SC_TRGSEL(0xF);	   // select software trigger
	PDB0_SC |= PDB_SC_PDBEN_MASK;      // enable PDB

	//Set the mod field to get a 1 second period.
	//There is a division by 2 to make the LED blinking period 1 second.
	//This translates to two mod counts in one second (one for on, one for off)

	PDB0_MOD = ((DEFAULT_SYSTEM_CLOCK*40/128)/1000);

	//Configure the Interrupt Delay register.
	PDB0_IDLY = 10;
	
	//Enable the interrupt mask.
	PDB0_SC |= (PDB_SC_PDBIE_MASK);

	//Enable LDOK to have PDB0_SC register changes loaded.
	PDB0_SC |= (PDB_SC_LDOK_MASK);

	PDB0_SC |= PDB_SC_SWTRIG_MASK ;
	PDB0_SC &= ~PDB_SC_PDBEN_MASK;
	return;
}

void initFTM(void){
	//Enable clock for FTM module (use FTM0)
	SIM_SCGC6 |= SIM_SCGC6_FTM0_MASK;

	//turn off FTM Mode to write protection;
	FTM0_MODE |= (FTM_MODE_WPDIS_MASK);

	//divide the input clock down by 128,
	FTM0_SC |= (FTM_SC_PS_MASK);
	
	//reset the counter to zero
	FTM0_CNT = 0;
	
	//Set the overflow rate
	//(Sysclock/128)- clock after prescaler
	//(Sysclock/128)/1000- slow down by a factor of 1000 to go from
	//Mhz to Khz, then 1/KHz = msec
	//Every 1msec, the FTM counter will set the overflow flag (TOF) and
	FTM0->MOD = (DEFAULT_SYSTEM_CLOCK/(1<<7))/1000;

	//Select the System Clock
	//FTM0_SC |= 0x00000008;
	FTM0_SC |= FTM_SC_CLKS(1);

	//Enable the interrupt mask. Timer overflow Interrupt enable
	FTM0_SC |= FTM_SC_TOIE_MASK;

	FTM0_SC &= ~(FTM_SC_TOIE_MASK);    //Disable timer


	return;
}

/*
Table 1.1: Pins and Ports for LEDs and switch
 ________________________________
| Obj       | Port | Pin | Name  |
|-----------|------|-----|-------|
| Blue LED  |  B   |  21 | PTB21 |
| Red LED   |  B   |  22 | PTB22 |
| Green LED |  E   |  26 | PTE26 |
| Switch 2  |  C   |  6  | PTC6  |
| Switch 3  |  A   |  4  | PTA4  |
|___________|______|_____|_______|
*/
void initGPIO(void){
	/*initialize push buttons and LEDs*/

	//initialize clocks for each different port used.
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTE_MASK;
	SIM_SCGC5 |= SIM_SCGC5_PORTA_MASK; //Switch 3
	SIM_SCGC5 |= SIM_SCGC5_PORTC_MASK; //Switch 2
	
	//Configure Port Control Register for Inputs with pull enable and pull up resistor
	PORTB_PCR21 = PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_SRE_MASK;
	PORTB_PCR22 = PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_SRE_MASK;
	PORTE_PCR26 = PORT_PCR_PE_MASK | PORT_PCR_PS_MASK | PORT_PCR_SRE_MASK;
	PORTC_PCR6 |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	PORTA_PCR4 |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
	
	//***MAY NEED TO RECONFIGURE MUX FOR OUTPUTS HERE***//
	PORTB_PCR21 = PORT_PCR_MUX(1);
	PORTB_PCR22 = PORT_PCR_MUX(1);
	PORTE_PCR26 = PORT_PCR_MUX(1);
	PORTC_PCR6  = PORT_PCR_MUX(1); 
	PORTA_PCR4  = PORT_PCR_MUX(1);
	
	// Switch the GPIO pins to output mode (Red and Blue LEDs)
	GPIOB_PDDR = (1 << 21) | (1 << 22);
	GPIOE_PDDR = (1 << 26);
	
	// Turn off the LEDs
	GPIOB_PSOR = (1UL << 21) | (1UL << 22);
	GPIOE_PSOR = 1UL << 26;

	// Set the push buttons as an input
	GPIOC_PDDR &= (0 << 6);
	GPIOA_PDDR &= (0 << 4);

	// interrupt configuration for SW3(Rising Edge) and SW2 (Either)
	PORTC_PCR6 |= PORT_PCR_IRQC(11); //Either Edge, Switch2
	PORTA_PCR4 |= PORT_PCR_IRQC(9);  //Rising Edge, Switch3

	return;
}

void LED_Activate(unsigned int red, unsigned int blue, unsigned int green){
	// activates LEDs given a binary value (1 or 0) for red, green, and blue in that order.
	// LEDs are active-low, but give this function a 1 to turn it on
	
	GPIOB_PCOR = (red << 22) | (blue << 21); // red, blue
	GPIOE_PCOR = (green << 26);			         // green
}

void LED_Deactivate(void){
	GPIOB_PSOR = (1UL << 21) | (1UL << 22);
	GPIOE_PSOR = 1UL << 26;
}


void initInterrupts(void){
	// Enable NVIC for portA,portC, PDB0,FTM0
	NVIC_EnableIRQ(PORTA_IRQn);
	NVIC_EnableIRQ(PORTC_IRQn);
	NVIC_EnableIRQ(PDB0_IRQn);
	NVIC_EnableIRQ(FTM0_IRQn);

	return;
}
