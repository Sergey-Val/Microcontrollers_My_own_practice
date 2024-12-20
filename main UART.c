/*
 *
 * Created: 29.10.2024 12:49:28
 * Author : Сергей
 */ 

#include <avr/io.h>
#include <stdio.h> //A library for printf()
#include <stdlib.h> //For using type float

float voltage = 0;

void USART_settings(unsigned int ubrr)
{
	UBRRH = (unsigned char)(ubrr >> 8); //UBRR consists of 12 bits from 0 to 11. The maximum value is 2047.
	UBRRL = (unsigned char)(ubrr); //These bits set the speed of transmitting in BAUD (bits per second)
	UCSRA |= (1<<U2X); //Doubling the speed of data transmission
	UCSRB = (1<<RXEN) | (1<<TXEN); //Enable transmitter and receiver
	UCSRC = (1<<URSEL) | (1<<USBS) | (1<<UCSZ1) | (1<<UCSZ0); //Asynchronous mode (by UMSEL = 0), 2 stop bits for transmitter, data-package of 8 bits
}

void ADC_settings(void)
{
  DDRC &= ~(1<<3);
  ADCSRA |= (1<<ADEN) | (1<<ADFR); //Enable ADC and set Free running mode

  //Setting division factor 8 for ADC
  ADCSRA &= ~(1<<ADPS2);
  ADCSRA |= (1<<ADPS1) | (1<<ADPS0);

  //Voltage reference selection
  ADMUX &= ~(1<<REFS1);
  ADMUX |= (1<<REFS0);

  //ADMUX &= ~(1<<ADLAR); //The result of a conversion is right adjusted (it's default)

  //Making PC3 an input channel of ADC
  ADMUX &= ~((1<<MUX3) | (1<<MUX2));
  ADMUX |= (1<<MUX1) | (1<<MUX0);

  ADCSRA |= (1<<ADSC); //Start conversion
}

//Function for deducing a symbol
static int uart_show_char(char symbol, FILE *stream)
{
  while (~UCSRA & (1<<UDRE)); //Waiting for transmission to finish
  UDR = symbol;
  return 0;
}
//---------------------------------------------------------------------
//Function for entering a symbol
static int uart_get_char(FILE *stream)
{
	while (~UCSRA & (1<<RXC)); //Waiting for reception to finish
	return UDR;
}

/*
Making our own stream called 'MySTD_out'
Appoint function 'uart_set_char' for output
Since we don't appoint a function for input, we write NULL
Set flag 'FDEV_SETUP_WRITE' since we only use input
*/

//static FILE MySTD_out = FDEV_SETUP_STREAM(uart_show_char, NULL, _FDEV_SETUP_WRITE); //Stream for output

/*
Making our own stream called 'MySTD_out_in'
Appoint function 'uart_get_char' for output
Since we appoint functions for both input and output, we write uart_show_char and uart_get_char
Set flag 'FDEV_SETUP_RW' since we use input and output
*/

static FILE MySTD_out_in = FDEV_SETUP_STREAM(uart_show_char, uart_get_char, _FDEV_SETUP_RW); //Stream for both input and output

int main(void)
{
    char symbol;
	DDRB = 0xff;
	PORTB = 0x00;

	USART_settings(12); //The speed of 9600 boud. Frequency 1 MHz. UX2 = 1;
	ADC_settings();

	//stdout = &MySTD_out; //Stream for output
	stdout = stdin = &MySTD_out_in; //Stream for both input and output

    while (1) 
    {
	  /*
	  if (ADCSRA & (1<<ADIF))
	  {
	    voltage = (ADC*5.0)/1024;
		printf("Voltage = %.2f\r", voltage);

		ADCSRA |= (1<<ADIF);
	  }
	  */

	  char trigger;
	  printf("Enter 's' to start\r");
	  do 
	  {
	    trigger = getchar();
	  } while (trigger != 's');
	  //---------------------------------------------------------------------------------
	  printf("\r");
	  printf("Enter a number between 0 or 9\r");
	  printf("Menu:\r");
	  //--------------------------------------------------------------------------------
	  printf("0 All ports 0\r");
	  printf("1 PB0 1\r");
	  printf("2 PB1 1\r");
	  printf("3 PB2 1\r");
	  printf("4 PB3 1\r");
	  printf("5 PB4 1\r");
	  printf("6 PB5 1\r");
	  printf("7 PB6 1\r");
	  printf("8 PB7 1\r");
	  printf("9 All ports 9\r");
	  //---------------------------------------------------------------------------------
	  printf("\rEnter q to quit\r\r");

	  symbol = getchar();
	  if (symbol == 'q')
	  {
	    printf("Gooood bye, bye, bye, bye, bye");
		PORTB = 0x00;
		break;
	  }
	  
	  switch (symbol)
	  {
	    case '0': PORTB = 0x00; break;
		case '1': PORTB = (1<<0); break;
		case '2': PORTB = (1<<1); break;
		case '3': PORTB = (1<<2); break;
		case '4': PORTB = (1<<3); break;
		case '5': PORTB = (1<<4); break;
		case '6': PORTB = (1<<5); break;
		case '7': PORTB = (1<<6); break;
		case '8': PORTB = (1<<7); break;
		case '9': PORTB = 0xff; break;
		default: printf("\rERROR! You can only enter a number between 0 and 9\r");
	  }

	  if (PINB == 0b11111111) printf("All ports are on\r");
	  else
	  {
	    for (int i = 0; i < 8; ++i)
	    {
		    if (PINB & (1<<i)) printf("\rPB%d is on\r", i);
	    }
	  }
    }
}
