#ifndef PWM0_H_
#define PWM0_H_

void SetDutyCycle0(unsigned int DutyCycle, unsigned int Frequency, int dir);
void SetDutyCycleL(unsigned int DutyCycle, unsigned int Frequency, int dir);
void SetDutyCycleR(unsigned int DutyCycle, unsigned int Frequency, int dir);
void InitPWM0(void);
void FTM0_IRQHandler(void);

#endif /* PWM0_H_ */
