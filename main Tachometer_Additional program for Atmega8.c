/*
 * Tachometer _Additional program for Atmega8.c
 *
 * Created: 14.05.2024 14:16:55
 * Author : Сергей
 */ 

#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

void PWM_settings(void)
{
  DDRB |= (1<<PB1); //Make an output at OC1A pin for PWM
  TCCR1B |= (1<<CS10); //Enable Timer/Counter1 with NO prescaler
  TCCR1B |= (1<<WGM13); //Phase and Frequency Correct PWM
  TCCR1A |= (1<<COM1A1); //Non-inverted PWM at pin OC1A
  TCNT1 = 0;
  ICR1 = 65535;
  OCR1A = 32768;
}

int main(void)
{
    PWM_settings();

	//Inputs for the buttons
	DDRD &= ~((1<<PD0) | (1<<PD1));
	PORTD |= (1<<PD0) | (1<<PD1);

    while (1) 
    {
	  if ((~PIND & (1<<0)) && (ICR1 > 1000) && (OCR1A > 500)) //Increase the frequency
	  {
		ICR1 -= 1000;
		OCR1A = ICR1 / 2;
		_delay_ms(50); //Anti-chatter
	  }

	  if ((~PIND & (1<<1)) && (ICR1 < 64535) && (OCR1A < 32768)) //Decrease the frequency
	  {
	    ICR1 += 1000;
	    OCR1A = ICR1 / 2;
		_delay_ms(50); //Anti-chatter
	  }
    }
}

