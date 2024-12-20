/*
 * Tachometer with Atmega64.c
 *
 * Created: 03.05.2024 12:27:22
 * Author : Сергей
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>

#define MINUTE_FACTOR 60
#define N_SLITS 2
#define RATED_SPEED 1500
#define MINIMUM_SPEED 1050

//Variable for ranks
int thousands = 0;
int hundreds = 0;
int decades = 0;
int units = 0;

int it = 0; //A variable for turning on segments
unsigned int count_interrupts_ovf = 0; //A variable to count the number of interrupts from overflow of Timer/Counter1
uint8_t count_10 = 0; //A variable to calculate the average number of tacts
double tacts_counter = 0;
double average_tacts = 1;
unsigned int speed = 0; //The actual speed of the motor per minute
int k = 0; //Factor that show the difference between the rated speed and the real one
unsigned int VoltageManager = 0; //A variable to control OCR2 with Phase Correct PWM
//------------------------------------------------------------------------------------------
int digits[10] =
{
	0b00111111, //0
	0b00000110, //1
	0b01011011, //2
	0b01001111, //3
	0b01100110, //4
	0b01101101, //5
	0b01111101, //6
	0b00000111, //7
	0b01111111, //8
	0b01101111 //9
};
//----------------------------------------------------------
void ShiftRegister(uint8_t byte)
{
  for (uint8_t i = 0; i < 8; ++i)
  {
     if (byte & (1<<i)) PORTE |= (1<<1);
	 else PORTE &= ~(1<<1);

	 //Clock line SH_CP
	 PORTE |= (1<<0);
	 PORTE &= ~(1<<0);
  }
}
//----------------------------------------------------------
void DisplayValue(unsigned int value)
{
  thousands = value/1000;
  hundreds = value%1000/100;
  decades = value%100/10;
  units = value%10;
}
//----------------------------------------------------------
void TimerCountersSettings(void)
{
	//Timer/Counter0 interrupt settings. Prescaler 8
	TCCR0 |= (1<<CS01);
	TCCR0 &= ~((1<<CS02) | (1<<CS00));
	TIMSK |= (1<<TOIE0); //Enable interrupts from overflow of Timer/Counter0
	TCNT0 = 0;

	//Settings for the TimerCounter1. NO prescaler
	TCCR1B |= (1<<CS10);
	TIMSK |= (1<<TOIE1); //Enable interrupts from overflow of Timer/Counter1
	TCNT1 = 0;
}
//----------------------------------------------------------
ISR(TIMER0_OVF_vect)
{
  //Memory Reset from the previous data
  PORTE &= ~(1<<3);
  PORTE |= (1<<3);

  ++it;
  if (it > 4) it = 1;

  if (it == 1)
  {
    ShiftRegister(0b11111110); //Turning on the fourth rank
	ShiftRegister(digits[units]);
  }

  else if (it == 2)
  {
    ShiftRegister(0b11111101); //Turning on the third rank
	ShiftRegister(digits[decades]);
  }

  else if (it == 3)
  {
    ShiftRegister(0b11111011); //Turning on the second rank
	ShiftRegister(digits[hundreds]);
  }

  else if (it == 4)
  {
    ShiftRegister(0b11110111); //Turning on the first rank
	ShiftRegister(digits[thousands]);
  }

  //Snap line ST_CP
  PORTE |= (1<<2);
  PORTE &= ~(1<<2);
}
//----------------------------------------------------------
ISR(TIMER1_OVF_vect)
{
  ++count_interrupts_ovf;
}
//----------------------------------------------------------
ISR(INT0_vect)
{
  tacts_counter += (TCNT1 + count_interrupts_ovf * 65536);
  TCNT1 = 0;
  count_interrupts_ovf = 0;
  ++count_10;

  if (count_10 == 10)
  {
    average_tacts = tacts_counter / 10.0;
	tacts_counter = 0;
	count_10 = 0;
  }
}
//----------------------------------------------------------
void PWM_settings(void)
{
  DDRB |= (1<<PB7); //Making output at the pin OC2 with PWM
  TCCR2 |= (1<<CS21); //Prescaler 8 for Timer/Counter2
  TCCR2 |= (1<<WGM20); //8-bit Phase Correct PWM mode
  TCCR2 |= (1<<COM21); //Non-inverted PWM
  OCR2 = 127; //Half of 255 (8 bits). The driving voltage is half of the maximum
}
//==========================================================
int main(void)
{
    //Pin settings
	DDRE |= (1<<3) | (1<<2) | (1<<1) | (1<<0);
	PORTE &= ~((1<<2) | (1<<1) | (1<<0)); //Pins for Snap, Data line and clock line correspondingly
	PORTE |= (1<<3); //For MR (Memory Reset). It is an inverted pin

	TimerCountersSettings();
	PWM_settings();

	EIMSK |= (1<<INT0); //Enable external interrupts on pin INT0
	EICRA |= (1<<ISC01) | (1<<ISC00); //Interrupts on rising edges

	sei(); //Enable global interrupts

    while (1) 
    {
	  speed = MINUTE_FACTOR * ((double)8000000.0/average_tacts) / N_SLITS;
	  DisplayValue(speed);
	  //if (speed < MINIMUM_SPEED) TCCR2 &= ~((1<<WGM20) | (1<<COM21)); //Stop the PWM
	  if (speed > RATED_SPEED)
	  {
	    k = speed / RATED_SPEED;
		VoltageManager = k * 127;
		if (VoltageManager > 255) VoltageManager = 255;
		OCR2 = VoltageManager;
	  }
	  else if (speed < RATED_SPEED)
	  {
	    k = RATED_SPEED / speed;
	    OCR2 = 127 / k;
	  }
    }
}