/*
 * Laser siren.c
 *
 * Created: 07.04.2024 13:12:53
 * Author : Сергей
 */ 

#include "LCD.h"

//Устройство лазерной сирены срабатывает, когда между источником лазера и фоторезистором появляется препятствие.
//В этот момент потениал на выводе PB0 микроконтроллера Atmega8 становится нулевым.
//После возвращения высокого потенциала необходимо отключить сирену вручную кнопкой Reset

int main(void)
{
    DDRD |= 0b11111100; //Making outputs at the pins PD2 - PD7

	DDRB &= ~(1<<0);
	PORTB &= ~(1<<0); //Using a pull-up resistor causes high potential at PB0

	DDRD |= (1<<1);
	PORTD &= ~(1<<1);

    DisplayInit();
	Line(1, 0);

	SendByte(0b00000001, COMMAND); //Clear the LCD
	_delay_ms(2);
	
	String("Ok");
	PORTD &= ~(1<<1);
	_delay_ms(2);

    while (1) 
    {
	  if (~PINB & (1<<0))
	  {
		SendByte(0b00000001, COMMAND); //Clear the LCD
		_delay_ms(2); //In the datasheet it is at least recommended 1.58 ms

		String("DANGER!");
		PORTD |= (1<<1);
		_delay_ms(2);
	  }
    }
}

