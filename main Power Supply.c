/*
 * Power_Supply.c
 *
 * Created: 19.06.2024 12:31:50
 * Author : Сергей
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define VREF 5.00 //Voltage reference
#define R 1 //Resistance which the current is measured through
#define PRECISION 100 //The constant that defines how many symbols after the dot will be displayed

//Variables to display ranks of voltage
int thousands_voltage = 0;
int hundreds_voltage = 0;
int decades_voltage = 0;
int units_voltage = 0;
//------------------------------------------------
//Variables to display ranks of current
int thousands_current = 0;
int hundreds_current = 0;
int decades_current = 0;
int units_current = 0;

int it = 0; //The variable for turning on segments
uint8_t n = 0; //The variable to count how many times ADC has been calculated
uint8_t channel = 1; //The variable to choose a channel
float voltage = 0;
float current = 0;
unsigned int comp = 993;
//============================================================
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
void DisplayVoltage(unsigned int voltage)
{
	thousands_voltage = voltage/1000;
	hundreds_voltage = voltage%1000/100;
	decades_voltage = voltage%100/10;
	units_voltage = voltage%10;
}
//----------------------------------------------------------
void DisplayCurrent(unsigned int current)
{
	thousands_current = current/1000;
	hundreds_current = current%1000/100;
	decades_current = current%100/10;
	units_current = current%10;
}
//----------------------------------------------------------
void Timer_Counter_Settings(void)
{
  TCCR0 |= (1<<CS01); //TimerCounter0 prescaler 8
  TCNT0 = 0;
  TIMSK |= (1<<TOIE0); //Enable interrupts from overflow of TimerCounter0
}
//----------------------------------------------------------
void ShiftRegister_Voltage(uint8_t byte)
{
  for (int i = 0; i < 8; ++i)
  {
    if (byte & (1<<i)) PORTD |= (1<<1);
	else PORTD &= ~(1<<1);

	//Clock line SH_CP
	PORTD |= (1<<0);
	PORTD &= ~(1<<0);
  }
}
//----------------------------------------------------------
void ShiftRegister_Current(uint8_t byte)
{
	for (int i = 0; i < 8; ++i)
	{
		if (byte & (1<<i)) PORTD |= (1<<5);
		else PORTD &= ~(1<<5);

		//Clock line SH_CP
		PORTD |= (1<<4);
		PORTD &= ~(1<<4);
	}
}
//---------------------------------------------------------
void ADC_settings()
{
  ADCSRA |= (1<<ADEN); //Enable ADC
  ADCSRA &= ~(1<<ADFR); //Single conversion
  ADCSRA |= (1<<ADPS1); //Division factor 4
  ADCSRA |= (1<<ADIE); //Enable interrupts from ADC
  ADMUX |= (1<<REFS0); //External source of voltage reference 5V with external capacitor 100 nF at AREF pin

  //Channel ADC_1 as default
  ADMUX &= ~((1<<MUX3) | (1<<MUX2) | (1<<MUX1));
  ADMUX |= (1<<MUX0);
  ADMUX &= ~(1<<ADLAR); //The result of conversion is RIGHT adjusted

  ADCSRA |= (1<<ADSC); //Start the first conversion
}
//---------------------------------------------------------
void PWM_settings(void)
{
  //Settings for Phase Correct PWM on TimerCounter1
  TCCR1B |= (1<<CS10); //No prescaler (division factor is 1). Frequency of the Phase Correct PWM is 3910.07 Hz
  TCCR1A |= (1<<COM1A1); //Non-inverting mode
  
  //10-bit Phase Correct PWM
  TCCR1A |= (1<<WGM11) | (1<<WGM10);

  OCR1A = comp; //Compare Register. Определяет величину задающего напряжения

  //Settings for Fast PWM on TimerCounter2
  TCCR2 |= (1<<CS21); //Prescaler 8 for TimerCounter2. Frequency of the Fast PWM is 3906.25 Hz.
  TCCR2 |= (1<<COM21); //Non-inverting PWM
  TCCR2 |= (1<<WGM21) | (1<<WGM20); //Fast PWM
}
//=========================================================
ISR(TIMER0_OVF_vect)
{
	//Memory Reset from the previous data
	PORTD &= ~((1<<7) | (1<<3));
	PORTD |= (1<<7) | (1<<3);

	++it;
	if (it > 4) it = 1;

	if (it == 1)
	{
		ShiftRegister_Voltage(0b11111110); //Turning on the fourth rank
		ShiftRegister_Voltage(digits[units_voltage]);

		ShiftRegister_Current(0b11111110); //Turning on the fourth rank
		ShiftRegister_Current(digits[units_current]);
	}

	else if (it == 2)
	{
		ShiftRegister_Voltage(0b11111101); //Turning on the third rank
		ShiftRegister_Voltage(digits[decades_voltage]);

		ShiftRegister_Current(0b11111101); //Turning on the third rank
		ShiftRegister_Current(digits[decades_current]);
	}

	else if (it == 3)
	{
		ShiftRegister_Voltage(0b11111011); //Turning on the second rank
		ShiftRegister_Voltage(digits[hundreds_voltage] | (1<<7));

		ShiftRegister_Current(0b11111011); //Turning on the second rank
		ShiftRegister_Current(digits[hundreds_current] | (1<<7));
	}

	else if (it == 4)
	{
		ShiftRegister_Voltage(0b11110111); //Turning on the first rank
		ShiftRegister_Voltage(digits[thousands_voltage]);

		ShiftRegister_Current(0b11110111); //Turning on the first rank
		ShiftRegister_Current(digits[thousands_current]);
	}

	//Snap line ST_CP
	PORTD |= (1<<6) | (1<<2);
	PORTD &= ~((1<<6) | (1<<2));
}
//----------------------------------------------------------
ISR(ADC_vect)
{
  if (channel == 1)
  {
	  voltage = ADC*VREF*PRECISION/1024;
	  DisplayVoltage(voltage);
	  channel = 2;

	  //Enable ADC_2 channel
	  ADMUX &= ~((1<<MUX3) | (1<<MUX2) | (1<<MUX0));
	  ADMUX |= (1<<MUX1);

	  ADCSRA |= (1<<ADSC); //Start a new conversion
  }
  else if (channel == 2)
  {
	  current = (voltage - ADC*VREF*PRECISION/1024) / R;
	  DisplayCurrent(current);
	  channel = 1;

	  //Enable ADC_1 channel
	  ADMUX &= ~((1<<MUX3) | (1<<MUX2) | (1<<MUX1));
	  ADMUX |= (1<<MUX0);

	  ADCSRA |= (1<<ADSC); //Start a new conversion
  }
}
//==========================================================
int main(void)
{
    //Pins for the Shift Register configured as outputs
    DDRD = 0xff;
    PORTD |= (1<<7) | (1<<3); //MR is an inverted pin
    PORTD &= ~((1<<6) | (1<<5) | (1<<4) | (1<<2) | (1<<1) | (1<<0));
	
	//Signal indicating that MC is working
	DDRB |= (1<<PB4);
	PORTB |= (1<<PB4);

	DDRB |= (1<<3) | (1<<1); //Outputs for PWM
	
	//Buttons
	DDRB &= ~((1<<0) | (1<<2));
	PORTB |= (1<<0) | (1<<2);

	Timer_Counter_Settings();
	ADC_settings();
	PWM_settings();

	sei();
	
    while (1) 
    {
	  if (~PINB & (1<<0))
	  {
		comp += 30;
		_delay_ms(1);
		if (comp > 993) comp = 993;
		OCR1A = comp;
	  }

	  if (~PINB & (1<<2))
	  {
		comp -= 30;
		_delay_ms(1);
		if (comp < 42) comp = 42;
		OCR1A = comp;
	  }
    }
}

