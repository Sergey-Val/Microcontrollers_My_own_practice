/*
 * Smoothly blinking LED.c
 *
 * Created: 30.11.2024 12:45:45
 * Author : Сергей
 */ 

#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

void PWM_settings(void)
{
  TCCR1A |= (1<<COM1A1); //Non-inverting PWM
  
  TCCR1A |= (1<<WGM11) | (1<<WGM10); //10-bit PWM. Phase correct

  TCCR1B |= (1<<CS11); //Prescaler 8
  OCR1A = 0; //Initial state of the output compare register
}

int main(void)
{
    DDRB |= (1<<PB1); //Making PB1 output for PWM;
	DDRC |= (1<<PC5);

	PORTC |= (1<<PC5);
	PORTB = 0x00; //All pins are off as default
	PWM_settings();
	unsigned char it = 0;
    while (1) 
    {
	  if (it == 0)
	  {
	    while (OCR1A <= 511)
		{
		  OCR1A++;
		  _delay_ms(5);
		}
		it = 1;
	  }
	  if (it == 1)
	  {
	    while (OCR1A >= 1)
		{
		  OCR1A--;
		  _delay_ms(5);
		}
		it = 0;
	  }
    }
 }
