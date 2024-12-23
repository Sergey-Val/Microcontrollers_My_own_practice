/*
 * LCD.h
 *
 * Created: 07.04.2024 13:14:28
 *  Author: Сергей
 */ 


#ifndef LCD_H_
#define LCD_H_

#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>

#define RS1 PORTD |= (1<<2) //Sending data
#define RS0 PORTD &= ~(1<<2) //Sending commands

#define E1 PORTD |= (1<<3) //High potential at pin E as default
#define E0 PORTD &= ~(1<<3) //Logical 0 enables to display the data

#define DATA 1
#define COMMAND 0

//Prototypes of the functions
void DisplayInit(void);
void SendLowNibble(uint8_t data);
void SendByte(uint8_t byte, uint8_t mode);
void String(char sentence[]);
void Line(uint8_t line, uint8_t position);

#endif /* LCD_H_ */