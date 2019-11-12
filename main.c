/*
This is the main file.
*/

#include "MK64F12.h"
#include "uart.h"
#include "stdio.h"
#include "camera_FTM.h"
#include "pwm_motors.h"
#include "pwm_servo.h"

#define CLOCK					       20485760u

//Grab the external GVs
extern uint16_t line[128];
extern int debugcamdata;
extern int capcnt;

//Necessary variables for the camera
float SmoothVals[128];
int BinTrace[128];
char strbuff[100];

//Set the Max and Min Cuttoff Freqs.
float CuttoffHigh = 21000;
float CuttoffLow = 5000;

//Car Stuff
int dc_duty_cycle = 20;
int servo_duty_cycle = 8;

int dc_freq = 10000;
int servo_freq = 50;

int FWD = 1;
int BCK = 0;
/*********************************************************************/

void delay(int del){
	int i;
	for (i=0; i<del*50000; i++){
		// Do nothing
	}
}

void filter_data_manipulation(){
	/*
	Manipulates the data to do a smooth trace and edge detection
	*/
	int state = 0x00; // The state of the values
	for(int i=0; i<128; i++){ //Make an int array for the camera values		
		//Smooth Trace
		if ((i >= 2) && (i <= 125)){ SmoothVals[i] = (line[i-2] + line[i-1] + line[i] + line[i+1] + line[i+2])/5.0; }	
		else{	SmoothVals[i] = 0; }
	}
	for(int i =0; i < 128; i++){
		if(state == 0x00){
			if(SmoothVals[i] > CuttoffHigh){
				state = 1;
			}	
		}
		else{//state = 0x01}
			if(SmoothVals[i] < CuttoffLow){
				state = 0x00;
			}
		}
		if(state == 0x01){BinTrace[i] = 0xF;}
		else{BinTrace[i] = 0x00;}
	}
}

void initialize_periferials(){
	/*
	Ititialized all components nesessary for the perifierals to run as
	well as components necessary for the microcontroller.
	*/
	init_uart();
	uart_put("UART Initialized.\r\n");
	init_GPIO(); // For CLK and SI output on GPIO
	uart_put("GPIO Initialized.\r\n");
	init_FTM2(); // To generate CLK, SI, and trigger ADC
	uart_put("FTM2 Initialized.\r\n");
	init_ADC0();
	uart_put("ADC0 Initialized.\r\n");
	init_PIT();	// To trigger camera read based on integration time
	uart_put("PIT Initialized.\r\n");
	uart_put("All camera components initialized.\r\n");
	InitPWM0();
	uart_put("PWM0 Initialized.\r\n");
	InitPWM3();
	uart_put("PWM3 Initialized.\r\n");
	uart_put("All motor and servo components initialized.\r\n");
	uart_put("All components initialized.\r\n");
}

int main(void)
{
	/*
	The main function for the code. Will initialize everything needed
	and then run the code for the car to operate.
	*/
	initialize_periferials();
	for(;;) {
		if (debugcamdata) {
			if (capcnt >= (500)) {          // Every 2 seconds
				GPIOB_PCOR |= (1 << 22);		  // send the array over uart
				sprintf(strbuff,"%i\n\r",-1); // start value
				uart_put(strbuff);
				filter_data_manipulation();
				SmoothVals[127] = -2.0;
				for (int i = 0; i < 127; i++) {
					sprintf(strbuff,"%f\n", SmoothVals[i]);
					uart_put(strbuff);
				}
				sprintf(strbuff,"%i\n\r",-2); // end value
				uart_put(strbuff);
				capcnt = 0;
				GPIOB_PSOR |= (1 << 22);
			}
		}
	} //for
} //main




