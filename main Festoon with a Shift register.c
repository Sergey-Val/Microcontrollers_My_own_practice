/*
 * Festoon with a Shift_Register.c
 *
 * Created: 24.04.2024 16:52:40
 * Author : Сергей
 */ 

#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

uint8_t choice = 0; //A variable to switch between the two programs

ISR (INT1_vect)
{
  choice ^= 1;
}

void ShiftRegister(uint8_t byte)
{
  for (uint8_t i = 0; i < 8; ++i)
  {
    //Data line
	if (byte & (1<<i)) PORTD |= (1<<1);
	else PORTD &= ~(1<<1);

	//Clocking line
	PORTD |= (1<<0);
	_delay_us(10);

	PORTD &= ~(1<<0);
	_delay_us(10);
  }

  //Snap
  PORTD |= (1<<4);
  _delay_us(10);

  PORTD &= ~(1<<4);
  _delay_us(10);
}

void InterruptSettings(void)
{
  //MCU Control Register. The low level of INT1 generates an interrupt request
//  MCUCR &= ~((1<<ISC11) | (1<<ISC10));

//MCU Control Register. The falling edge if INT1 generates an interrupt request
  MCUCR |= (1<<ISC11);
  MCUCR &= ~(1<<ISC10);

  GICR |= (1<<INT1); //Enable external interrupts at pin INT1
  sei(); //Enable global interrupts
}

void sequence(void)
{
  for (uint8_t i = 0; i < 8; ++i)
  {
     ShiftRegister(1<<i);
	 _delay_ms(1000);
	 if (choice == 1) return;
  }
}

void couples(void)
{
   ShiftRegister(0b00010001); //blue
   _delay_ms(1000);
   if (choice == 0) return;

   ShiftRegister(0b00100010); //green
   _delay_ms(1000);
   if (choice == 0) return;

   ShiftRegister(0b01000100); //red
   _delay_ms(1000);
   if (choice == 0) return;

   ShiftRegister(0b10001000); //yellow
   _delay_ms(1000);
   if (choice == 0) return;
}

int main(void)
{
    DDRD = 0b00110011;
	PORTD = 0b00101000;

	InterruptSettings();

    while (1) 
    {
	  if (choice == 0) sequence();
	  if (choice == 1) couples();
    }
}

