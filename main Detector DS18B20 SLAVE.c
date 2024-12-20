/*
 * Detector DS18B20 (SLAVE).c
 *
 * Created: 07.10.2023 14:58:40
 * Author : Сергей
 */ 
 //Передаётся только полбайта (4 бита), вследствие чего информация искажена
#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//Defining the numbers of PORTB
#define SS 2
#define MOSI 3
#define MISO 4
#define SCK 5

unsigned int thousands = 0;
unsigned int hundreds = 0;
unsigned int decades = 0;
unsigned int units = 0;

uint8_t it = 1; //A variable for iteration
unsigned int Temperature = 0;

//Variable to compile the temperature
unsigned int FirstPart = 0;
unsigned int SecondPart = 0;

uint8_t InterruptCount = 0; //A variable to count how many times the interrupt occurred
uint8_t SignFlag = 0; //A flag to distinguish between positive and negative values. 0 means a positive value.
//1 means a negative value

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

void DisplaySettings(void)
{
  //Settings for the ports
  DDRC |= 0b0001111;
  PORTC |= 0b0001111;

  DDRD = 0xff;
  PORTD = 0x00;
  
  //Settings for TimerCounter0
  TCCR0 |= (1<<CS01) | (1<<CS00); //Prescaler 64
  TCNT0 = 0; //Initial value of TimerCounter0
  TIMSK |= (1<<TOIE0); //Enable interrupts from TimerCounter0
  sei(); //Enable global interrupts
}

void DisplayValue(int number)
{
	thousands = number/1000;
	hundreds = number%1000/100;
	decades = number%100/10;
	units = number%10;
}

//Interrupts for dynamic indication
ISR(TIMER0_OVF_vect)
{
  if (it == 1)
  {
	PORTC |= (1<<3) | (1<<2) | (1<<1);
	PORTC &= ~(1<<0);
	if (SignFlag) PORTD = 0b01000000; //Display a minus
	else PORTD = digits[thousands];
  }
  else if (it == 2)
  {
    PORTC |= (1<<3) | (1<<2) | (1<<0);
	PORTC &= ~(1<<1);
    PORTD = digits[hundreds];
  }
  else if (it == 3)
  {
    PORTC |= (1<<3) | (1<<1) | (1<<0);
	PORTC &= ~(1<<2);
    PORTD = digits[decades];
	PORTD |= (1<<7); //Display a dot
  }
  else if (it == 4)
  {
    PORTC |= (1<<2) | (1<<1) | (1<<0);
	PORTC &= ~(1<<3);
	PORTD = digits[units];
  }
  ++it;
  if (it > 4) it = 1;
}

void SPI_SlaveSettings(void)
{
  DDRB &= ~((1<<MOSI) | (1<<SCK));
  DDRB |= (1<<MISO);


  //SPI Control Register
  SPCR |= (1<<SPIE) | (1<<SPE); //Enable SPI and interrupts from it
  SPCR &= ~(1<<MSTR); //Make the MC a slave

  //Frequency SCK F_CPU / 16 is set by the master

  //Data is sampled on the leading rising edge of SCK
  SPCR &= ~(1<<CPOL);
  SPCR &= ~(1<<CPHA);
  //--------------------------------------------------
  SPCR &= ~(1<<DORD); //The most significant bit is read first

  DDRB &= ~(1<<SS); //Making SS pin an input to receive a signal from the Master
  PORTB |= (1<<SS); //Making SS ready
}

ISR(SPI_STC_vect)
{
  ++InterruptCount;
  while(~SPSR & (1<<SPIF)) //Waiting for data to be received
  ;
  if (InterruptCount == 1) SignFlag = SPDR;
  else if (InterruptCount == 2) FirstPart = SPDR;
  else if (InterruptCount == 3)
  {
    SecondPart = SPDR;
	InterruptCount = 0;
  }
}
//======================================================
int main(void)
{
    DisplaySettings();
	SPI_SlaveSettings();

    while (1) 
    {
	  Temperature = FirstPart + SecondPart;
	  DisplayValue(Temperature);
    }
}