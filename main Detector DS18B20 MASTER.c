/*
 * Detector DS18B20 (MASTER).c
 *
 * Created: 01.10.2023 13:48:18
 * Author : Сергей
 */ 

#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define SS PB2
#define MOSI PB3
#define MISO PB4
#define SCK PB5

unsigned int temperature = 0;
uint8_t Negative_marker = 0; //An indicator of negative temperature. 0 means a positive value.
//1 means a negative value

unsigned int thousands = 0;
unsigned int hundreds = 0;
unsigned int decades = 0;
unsigned int units = 0;

int it = 1; //A variable for iteration
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
	DDRC |= 0b0011101;
	PORTC |= 0b0011101;

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
		PORTC |= (1<<4) | (1<<3) | (1<<2);
		PORTC &= ~(1<<0);
		if (Negative_marker == 1) PORTD = 0b01000000; //Display a minus
		else PORTD = digits[thousands];
	}
	else if (it == 2)
	{
		PORTC |= (1<<4) | (1<<3) | (1<<0);
		PORTC &= ~(1<<2);
		PORTD = digits[hundreds];
	}
	else if (it == 3)
	{
		PORTC |= (1<<4) | (1<<2) | (1<<0);
		PORTC &= ~(1<<3);
		PORTD = digits[decades];
		PORTD |= (1<<7); //Display a dot
	}
	else if (it == 4)
	{
		PORTC |= (1<<3) | (1<<2) | (1<<0);
		PORTC &= ~(1<<4);
		PORTD = digits[units];
	}
	++it;
	if (it > 4) it = 1;
}

void SPI_MasterSettings(void)
{
  DDRB |= (1<<SS) | (1<<MOSI) | (1<<SCK);
  PORTB |= (1<<SS); //It's necessary to supply high potential at this pin in advance
  DDRB &= ~(1<<MISO);

  //SPI Control Register
  SPCR |= (1<<SPIE) | (1<<SPE); //Enable SPI and interrupts from it
  SPCR |= (1<<MSTR); //Make the MC Master

  //Frequency of SCK is F_CPU / 16
  SPSR &= ~(1<<SPI2X);
  SPCR &= ~(1<<SPR1);
  SPCR |= (1<<SPR0);

  SPCR &= ~(1<<CPOL); //Reading data at rising edges of SCK
  SPCR &= ~(1<<CPHA); //Data is sampled at the leading edges of SCK. Refer to Figure 59 on page 128 in the data sheet
  SPCR &= ~(1<<DORD); //The most significant bit is transmitted first
}

//Interrupts from SPI to free the Slave Select (SS) line
ISR(SPI_STC_vect)
{
  while (~SPSR & (1<<SPIF)) //Waiting for the previous data to be transmitted
  ;
  PORTB |= (1<<SS); //Pulling up high potential at SS to indicate finishing a data transfer (SS line is free)
}
//------------------------------------------------------------------------------------------------------------------
bool isDetectorOn(void)
{
  bool reply;
  cli(); //Forbid interrupts

  //Occupying the data bus
  DDRC |= (1<<PC1);
  PORTC &= ~(1<<PC1);
  _delay_us(490);

  //Releasing the data bus. The pull-up resistor provides high potential at PC1
  DDRC &= ~(1<<PC1);
  //PORTC &= ~(1<<PC1); //The bit is already zero as default
  _delay_us(100);

  if (~PINC & (1<<PC1)) //Checking for zero
  reply = true;
  else reply = false;

  sei(); //Enable interrupts
  _delay_us(440); //Waiting for the slot to finish. See the data sheet for DS18B20 on page 15.

  return reply;
}
//---------------------------------------------------------------------------------------------------------
void WriteByte(uint8_t byte)
{
  for (uint8_t i = 0; i < 8; ++i)
  {
    if (byte & (1<<i))
	{
	  cli();
	  //Pulling the 1-Wire bus low
	  DDRC |= (1<<PC1);
	  PORTC &= ~(1<<PC1);
	  _delay_us(2);

	  //Releasing the 1-Wire bus
	  DDRC &= ~(1<<PC1);
	  //PORTC &= ~(1<<PC1); //The bit is already zero as default
	  sei();
	  _delay_us(58);
	}
	else
	{
	  cli();
	  //Pulling the 1-Wire bus low
	  DDRC |= (1<<PC1);
	  PORTC &= ~(1<<PC1);
	  _delay_us(60);

	  //Releasing the 1-Wire bus
	  DDRC &= ~(1<<PC1);
	  sei();
	}
  }
}
//-------------------------------------------------------------------------------------------------
uint8_t ReadByte(void)
{
  char byte = 0; //A variable to store the whole byte compiled of the returned bits
  for (uint8_t i = 0; i < 8; ++i)
  {
    cli();
	//Occupying the 1-Wire bus
	DDRC |= (1<<PC1);
	PORTC &= ~(1<<PC1);
	_delay_us(2);

	//Releasing the 1-Wire bus
	DDRC &= ~(1<<PC1);
	_delay_us(10);

	//The detector gets 45 microseconds to return a value of '0' or '1'
	//The detector returns bits from the least significant to the most significant
	//As those bits are being stored in the "byte" variable, they get settled at the corresponding position.
	if (PINC & (1<<PC1))  byte |= (1<<i);
	else byte &= ~(1<<i);
	_delay_us(45);
	sei();
  }
  return byte;
}
//--------------------------------------------------------------------------------------------------------
//Information about the temperature consists of two bytes, so we need a function for reading them.
short ReadTwoBytes(void)
{
  //Variables to store the data from the detector
  uint8_t LowByte = 0;
  uint16_t Two_Bytes = 0;

  if (isDetectorOn())
  {
    WriteByte(0xCC); //Skip ROM command to address the detector (or all the detectors if there are many)
	WriteByte(0x44); //This command to make a single conversion of the temperature
	_delay_ms(750); //Time for conversion the temperature with 12-bit

	isDetectorOn(); //This function is needed here to initialize the detector once again
	WriteByte(0xCC); //Skip ROM
	WriteByte(0xBE); //Command to the detector to read data

	//The low byte is returned first, then goes the high byte
	LowByte = ReadByte(); //Receiving the low byte
	Two_Bytes = ReadByte(); //Receiving the high byte
	Two_Bytes = (Two_Bytes << 8) | LowByte; //Compiling the two bytes together
  }
  return Two_Bytes;
}
//---------------------------------------------------------------
void Bytes_to_TemperatureConversion(uint16_t bytes)
{
  if (bytes < 2048) //The temperature is higher than 0. 2048 = 0b1000 0000 0000, the 11th bit is responsible for negative values
  {
    Negative_marker = 0;
	temperature = bytes * 0.0625 * 10; //Each bit cell contains 0.0625 of the temperature.
	//Multiplier 10 is needed to display float values with one symbol after a dot.
  }
  else //The temperature is lower than 0.
  {
	Negative_marker = 1;
	temperature = (~bytes + 1) * 0.0625 * 10; //Inversion + 1 makes the value positive.
  }
}
//===============================================================
int main(void)
{
    //For LED
	DDRB |= (1<<PB0);
	PORTB &= ~(1<<PB0);
	sei(); //Enable global interrupts
	SPI_MasterSettings();
	DisplaySettings();

    while (1) 
    {
	  if (isDetectorOn()) PORTB |= (1<<PB0);
	  else PORTB &= ~(1<<PB0);
	  Bytes_to_TemperatureConversion(ReadTwoBytes());
	  DisplayValue(temperature);

	  //Transmitting the sign
	  PORTB &= ~(1<<SS); //Occupying the SS line to indicate a start of transmitting data
	  SPDR = Negative_marker;
	  _delay_ms(1);

	  if (temperature <= 25)
	  {
	    //Transmitting the temperature
	    PORTB &= ~(1<<SS); //Occupying the SS line to indicate a start of transmitting data
	    SPDR = temperature;
		_delay_ms(1);

		//Transmitting 0
		PORTB &= ~(1<<SS); //Occupying the SS line to indicate a start of transmitting data
		SPDR = 0;
	  }
	  else if (temperature > 25)
	  {
	    //Transmitting 25
		PORTB &= ~(1<<SS); //Occupying the SS line to indicate a start of transmitting data
		SPDR = 25;
		_delay_ms(1);

		//Transmitting the residual
		PORTB &= ~(1<<SS); //Occupying the SS line to indicate a start of transmitting data
		SPDR = (temperature - 25);
	  }
	  _delay_ms(1);
    }
}