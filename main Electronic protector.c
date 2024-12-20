/*
 * Electronic protector.c
 *
 * Created: 30.05.2024 14:18:39
 * Author : Сергей
 */ 

#define F_CPU 1000000UL
#include <avr/io.h>

void ADC_settings(void)
{
   ADCSRA |= (1<<ADEN); //Enable the ADC
   ADCSRA |= (1<<ADFR); //Set Free Running mode

   //Division factor for the CPU is 8. Frequency of discretization 125 kHz.
   ADCSRA &= ~(1<<ADPS2);
   ADCSRA |= (1<<ADPS1) | (1<<ADPS0);

   ADMUX |= (1<<REFS1) | (1<<REFS0); //Internal 2.56V voltage reference with external capacitor at AREF pin
   ADMUX &= ~(1<<ADLAR); //The result of ADC is RIGHT adjusted
   ADMUX |= (1<<MUX0); //Enable ADC1
}

int main(void)
{
    DDRB |= (1<<1) | (1<<0);
	PORTB &= ~((1<<1) | (1<<0));

	ADC_settings();
	ADCSRA |= (1<<ADSC); //Start conversion

    while (1) 
    {
	  if (ADCSRA & (1<<ADIF))
	  {
	    if (ADC < 800) //Voltage is lower than 1.93V. When choosing voltage higher than 2.56V, it's considered infinite, and the condition becomes true.
		{
		  PORTB |= (1<<0) | (1<<1);
		}
		else PORTB &= ~((1<<0) | (1<<1));

		ADCSRA |= (1<<ADIF);
	  }
    }
}

