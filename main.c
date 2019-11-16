/*
This is the main file.
*/

#include "MK64F12.h"
#include "uart.h"
#include "stdio.h"
#include "camera_FTM.h"
#include "pwm_motors.h"
#include "pwm_servo.h"
#include "timer.h" //Only for LED functions

#define CLOCK	20485760u
#define INTEGRATION_TIME .0075f
#define	DC_FREQ 10000
#define SERVO_FREQ 50
#define FWD 1
#define BCK 0
#define RIGHT 8.0
#define LEFT 10.6
#define STRAIGHT 9.0


//Grab the external GVs
extern uint16_t line[128];
extern int debugcamdata;
extern int capcnt;

//Necessary variables for the camera
float SmoothVals[128];
int BinTrace[128];
char strbuff[100];
int CAMERA_ERROR_MARGIN = 10;


//Set the Max and Min Cuttoff Freqs.
float CuttoffHigh = 13000;
float CuttoffLow = 7000;

//Car Stuff
int dc_duty_cycle = 50;
int servo_duty_cycle = 8;

int dc_freq = 10000;
int servo_freq = 50;

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
				state = 0x01;
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

void initialize_peripherals(){
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
	initialize_peripherals();
	for(;;) {
		//GPIOB_PCOR |= (1 << 22);//Turn on red LED?
		filter_data_manipulation();
		//GPIOB_PSOR |= (1 << 22); //Turn off red LED?
		
		if (debugcamdata) {
			//Print UART Data if being sent
			if (capcnt >= (50)) { // Every 2 seconds
				sprintf(strbuff,"%i\n\r",-1); // start value
				uart_put(strbuff);
				for (int i = 0; i < 127; i++) {
					sprintf(strbuff,"%f\n", SmoothVals[i]);
					uart_put(strbuff);
				}
				SmoothVals[127] = -2.0;
				sprintf(strbuff,"%i\n\r",-2); // end value
				uart_put(strbuff);
				capcnt = 0; //restart counter for printing data point
			}
		}
		//Move the Servo and motors based on GVs set from filter_data_manbipulation
		int Left_Avg = 0; 
		int Right_Avg = 0;
		for(int i = 0; i < 64; i++){
			if(BinTrace[i] == 0x0F){Left_Avg += 1;}
		}
		for(int i = 64; i < 127; i++){
			if(BinTrace[i] == 0x0F){Right_Avg += 1;}
		}
		if((Right_Avg < 10) && (Left_Avg < 10)){
			SetDutyCycleR(0, DC_FREQ, FWD);
			SetDutyCycleL(0, DC_FREQ, FWD);
		}
		//Determine the direction from the values calculated.
		//If car is leaning to the left, turn left & vice versa.
		else if(Right_Avg > (Left_Avg + CAMERA_ERROR_MARGIN)){
			//Turn Right
			SetDutyCycleServo(RIGHT, SERVO_FREQ);
			SetDutyCycleL(dc_duty_cycle-25, DC_FREQ, FWD);
			SetDutyCycleR(dc_duty_cycle+5-25, DC_FREQ, FWD);
			LED_Activate(0,0,1); //Turn Green when going right
		}
		else if(Left_Avg > (Right_Avg + CAMERA_ERROR_MARGIN)){
			//Turn Left
			SetDutyCycleServo(LEFT, SERVO_FREQ);
			SetDutyCycleL(dc_duty_cycle-25, DC_FREQ, FWD);
			SetDutyCycleR(dc_duty_cycle+5-25, DC_FREQ, FWD);
			LED_Activate(0,1,0); //Turn Blue when going left
		}
		else{
			//Go Straight
			SetDutyCycleServo(STRAIGHT, SERVO_FREQ);
			SetDutyCycleL(dc_duty_cycle, DC_FREQ, FWD);
			SetDutyCycleR(dc_duty_cycle+5, DC_FREQ, FWD);
			LED_Activate(1,1,1); //Turn White when going straight
		}
	} //for
} //main

int main_servo_debug(void){
	initialize_peripherals();
	for(;;){
		SetDutyCycleServo(10.6, SERVO_FREQ);
		delay(30);
		SetDutyCycleServo(9.0, SERVO_FREQ);
		delay(30);
		SetDutyCycleServo(8.0, SERVO_FREQ);
		delay(30);
	}
}

