/*
 * LCD.c
 *
 * Created: 07.04.2024 13:36:53
 *  Author: Сергей
 */ 

 #include "LCD.h"

 void DisplayInit(void)
 {
	 //All the commands below applied in accordance to the datasheet of the LCD
	 _delay_ms(15);
	 SendByte(0b00110000, COMMAND);
	 _delay_ms(5);
	 SendByte(0b00110000, COMMAND);
	 _delay_us(100);
	 SendByte(0b00110000, COMMAND);
	 //SendByte(0b00000011, COMMAND); This is another implication of the command above, and it also works

	 SendByte(0b00000010, COMMAND);

	 SendByte(0b00101000, COMMAND); //4 bits mode, 2 strings, font 5x8
	 _delay_us(40); //See the table of initializing commands in the datasheet (it recommends at least 38 us)

	 SendByte(0b00001100, COMMAND); //Turn on the LCD with NO cursor
	 _delay_us(40); //In the datasheet it is recommended at least 38 us

	 SendByte(0b00000001, COMMAND); //Clear the LCD
	 _delay_ms(2); //In the datasheet it is at least recommended 1.58 ms

	 //SendByte(01100000, COMMAND); This is another implication of the command below, and it also works
	 SendByte(0b00000110, COMMAND); //Increment and shifting the cursor to the right
	 _delay_us(40);
 }
 //---------------------------------------------------------------------------
 void SendLowNibble(uint8_t data)
 {
   data <<= 4;
   E1;
   _delay_us(50);
   PORTD &= 0b00001111;
   PORTD |= data;
   E0;
   _delay_us(50);
 }
 //----------------------------------------------------------------
 void SendByte(uint8_t byte, uint8_t mode)
 {
   if (mode == 0) RS0;
   else RS1;

   uint8_t HighNibble = byte >> 4;
   SendLowNibble(HighNibble);
   SendLowNibble(byte);
 }
//-----------------------------------------------------------------
void String(char sentence[])
{
  uint8_t i = 0;
  for (i = 0; sentence[i] != '\0';)
  {
    ++i;
  }
  if (i > 15)
  {
    SendByte('T', DATA);
	SendByte('o', DATA);
	SendByte('o', DATA);
	SendByte(' ', DATA);
	SendByte('m', DATA);
    SendByte('a', DATA);
    SendByte('n', DATA);
    SendByte('y', DATA);
    SendByte(' ', DATA);
    SendByte('s', DATA);
	SendByte('y', DATA);
	SendByte('m', DATA);
	SendByte('b', DATA);
	SendByte('o', DATA);
	SendByte('l', DATA);
	SendByte('s', DATA);
  }
  else
  {
    for (i = 0; sentence[i] != '\0'; ++i)
	{
	  SendByte(sentence[i], DATA);
	}
  }
}
//-----------------------------------------------------------------
void Line(uint8_t line, uint8_t position)
{
  switch (line)
  {
    case 1: SendByte(position, COMMAND); break;
	case 2: SendByte((position + 0x40), COMMAND); break;
	default: String("Wrong position!");
  }
}