/*
 * isr.c
 */

#include "isr.h"
#include "MK64F12.h"
#include <stdio.h>
#include "uart.h"
#include "timer.h"

//variables global to the IRQ handlers which dictates if timer is enabled &  timer counter
//static uint8_t SWITCH2_STATUS    = 0x00;
//static uint8_t TIMER_STATUS_FTM  = 0x00;
static int 	   COUNTER           = 0;

void PDB0_IRQHandler(void){        //For PDB timer
	PDB0_SC &= ~(PDB_SC_PDBIF_MASK); //Clear the interrupt
	GPIOB_PTOR = (1UL << 21);        //Toggle value for LED
	return;
}
	
void FTM0_IRQHandler(void){            //For FTM timer
	FTM0_SC &= ~(FTM_SC_TOF_MASK);       //Clear inturupt
		if((GPIOC_PDIR & (1 << 6)) == 0){
			COUNTER += 1;                    //Itterate counter
		}
	return;
}
	
void PORTA_IRQHandler(void){
	//For switch 3, PDB
	PORTA_ISFR &= ~(PORT_PCR_ISF_MASK); //Clear interupt with interupt service flag
	if(PDB0_SC & PDB_SC_PDBEN_MASK){    //If the enable is 1
		PDB0_SC &= ~(PDB_SC_PDBEN_MASK);  //Disable timer
	} else {
		PDB0_SC |= (PDB_SC_PDBEN_MASK);   //Enable Timer
		PDB0_SC |= PDB_SC_SWTRIG_MASK;
		//PORTA_PCR4 = PORT_PCR_IRQC(11);
	}
	return;
}
	
void PORTC_IRQHandler(void){         //For switch 2, FTM
	PORTC_ISFR &= ~PORT_ISFR_ISF_MASK; //Clear the inturupt
	if((GPIOC_PDIR & (1 << 6)) == 0){  //If the button is pressed
//		SWITCH2_STATUS = 0xFF; //set GV to affect the timer2 function
		FTM0_CNT = 0;		       //reset FlexTimer
		COUNTER =0;            //Inc counter every time int is hit
		LED_Activate(0,1,0);	 //turn on blue LED
		
		while((GPIOC_PDIR & (1 << 6)) == 0){
			//Do nothing while button is being pressed
			COUNTER += 1;
		}
		COUNTER = COUNTER/10000;
		//Print statment when button is released
//		SWITCH2_STATUS = 0x00;           //reset GV for timer2 function
		LED_Deactivate();		             //turn off blue LED
		uart_put("Button pressed for "); //print res "Button pressed for xx miliseconds"
		char StringCounter[16];
		sprintf(StringCounter, "%d", COUNTER);
		uart_put(StringCounter);
		uart_put(" ms. \r\n");
	}
	return;
}
