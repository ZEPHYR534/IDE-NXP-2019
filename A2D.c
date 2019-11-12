/*
* Rochester Institute of Technology
* Department of Computer Engineering
* CMPE 460  Interfacing Digital Electronics
* Spring 2016
*
* Filename: main_A2D_template.c
*/
 
#include "uart.h"
#include "MK64F12.h"
#include "stdio.h"

 
void PDB_INIT(void) {
    //Enable PDB Clock
    SIM_SCGC6 |= SIM_SCGC6_PDB_MASK;
    PDB0_MOD = 50000; // 50,000,000 / 50,000 = 1000

    PDB0_SC = PDB_SC_PDBEN_MASK | PDB_SC_CONT_MASK | PDB_SC_TRGSEL(0xf) | PDB_SC_LDOK_MASK;
    PDB0_CH1C1 = PDB_C1_EN(0x01) | PDB_C1_TOS(0x01);
}
 
void ADC1_INIT(void) {
	
    unsigned int calib;
 
    // Turn on ADC1
    SIM_SCGC3 |= SIM_SCGC3_ADC1_MASK;

    //(Insert your code here.)
 
    // Configure CFG Registers
		//16-bit single ended
    
	  // Configure ADC to divide 50 MHz down to 6.25 MHz AD Clock, 16-bit single ended
		ADC1_CFG1 |= ADC_CFG1_ADIV(0x3);
		ADC1_CFG1 |= ADC_CFG1_MODE(0x3); 
    
	  // Do ADC Calibration for Singled Ended ADC. Do not touch.
    ADC1_SC3 = ADC_SC3_CAL_MASK;
    while ( (ADC1_SC3 & ADC_SC3_CAL_MASK) != 0 );
    calib = ADC1_CLP0;
    calib += ADC1_CLP1;
    calib += ADC1_CLP2;
    calib += ADC1_CLP3;
    calib += ADC1_CLP4;
    calib += ADC1_CLPS;
    calib = calib >> 1;
    calib |= 0x8000;
    ADC1_PG = calib;
 
    // Configure SC registers.
    // Select hardware trigger.
		ADC1_SC2 |= ADC_SC2_ADTRG_MASK;
 
 
    // Configure SC1A register.
    // Select ADC Channel and enable interrupts. Use ADC1 channel DADP3  in single ended mode.
		ADC1_SC1A &= ADC_SC1_DIFF_MASK; 
		ADC1_SC1A |= ADC_SC1_AIEN_MASK; //0x40
		ADC1_SC1A |= ADC_SC1_ADCH(0x3);
 
    // Enable NVIC interrupt
    NVIC_EnableIRQ(ADC1_IRQn);
}
 
// ADC1 Conversion Complete ISR
void ADC1_IRQHandler(void) {
    // Read the result (upper 12-bits). This also clears the Conversion complete flag.
    unsigned short i = ADC1_RA >> 4;

    //Set DAC output value (12bit)
    DAC0_DAT0L = (i & 0x00FF);
    DAC0_DAT0H = ((i >> 8) & 0x00FF);

}

void DAC0_INIT(void) {
    //enable DAC clock
    SIM_SCGC2 |= SIM_SCGC2_DAC0_MASK;
    DAC0_C0 = DAC_C0_DACEN_MASK | DAC_C0_DACRFS_MASK;
    DAC0_C1 = 0;
}
 
int A2D_main(void) {
    int i; char str[100];
   
    // Enable UART Pins
    //(Insert your code here.)
   
    // Initialize UART
		uart_init();
    uart_put("UART Initialized.\r\n");
    DAC0_INIT();
	  uart_put("DAC0 Initialized.\r\n");
    ADC1_INIT();
    uart_put("ADC1 Initialized.\r\n");
		PDB_INIT();
		uart_put("PDB Initialized.\r\n");
	
    // Start the PDB (ADC Conversions)
    PDB0_SC |= PDB_SC_SWTRIG_MASK;
 
    for(;;) {	
			float temperature_C = (((ADC1_RA*(.0000503))-.75)/.010) + 25; //Temp in C
			float temperature_F = (temperature_C *(1.8)) + 32;
			sprintf(str,"\n Temp (C): %f Temp (F): %f \n\r",temperature_C,temperature_F);
			uart_put(str);
			for( i=0; i < 5000000; ++i ){
			//DO NOTHING									 
			}
    }
 
    return 0;
}
