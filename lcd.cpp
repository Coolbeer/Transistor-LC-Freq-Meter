#include "lcd.h"
#include <avr/io.h>
#include <util/delay.h>

void t_lcd::init(void)
{
	//Set the ports as output
	DDR(RS_PORT)	|= (1 << DD(RS_PORT, RS_PIN));
	DDR(E_PORT)		|= (1 << DD(E_PORT, E_PIN));
	DDR(RW_PORT)	|= (1 << DD(RW_PORT, RW_PIN));
	DDR(DB4_PORT)	|= (1 << DD(DB4_PORT, DB4_PIN));
	DDR(DB5_PORT)	|= (1 << DD(DB5_PORT, DB5_PIN));
	DDR(DB6_PORT)	|= (1 << DD(DB6_PORT, DB6_PIN));
	DDR(DB7_PORT)	|= (1 << DD(DB7_PORT, DB7_PIN));

	CLEARPIN(RS_PORT,  RS_PIN);
	CLEARPIN(E_PORT,   E_PIN);
	CLEARPIN(RW_PORT,  RW_PIN);
	CLEARPIN(DB4_PORT, DB4_PIN);
	CLEARPIN(DB5_PORT, DB5_PIN);
	CLEARPIN(DB6_PORT, DB6_PIN);
	CLEARPIN(DB7_PORT, DB7_PIN);

	_delay_ms(40);

	SETPIN(E_PORT, E_PIN);
	setDBPort(0x3); //0b0011

	_delay_ms(5);
	CLEARPIN(E_PORT, E_PIN);

	_delay_ms(1);
	SETPIN(E_PORT, E_PIN);

	_delay_ms(1);
	CLEARPIN(E_PORT, E_PIN);

	SETPIN(E_PORT, E_PIN);

	_delay_ms(1);

	CLEARPIN(E_PORT, E_PIN);

	setDBPort(0x2);
	_delay_ms(1);
	waitBusy();
	
	sendCmd(0x22);

	sendCmd(0x01);
	sendCmd(0x06);

	sendCmd(0x0C);
}

void t_lcd::waitBusy(void)
{
	CLEARPIN(DB7_PORT, DB7_PIN);
	CLEARPIN(RS_PORT, RS_PIN);
	SETPIN(RW_PORT, RW_PIN);

	while(PIN(DB7_PORT) & (1 << PINX(DB7_PORT, DB7_PIN)))
		asm volatile ( "nop" );

	CLEARPIN(RW_PORT, RW_PIN);
}

void t_lcd::sendString(char *data)
{
	uint8_t cnt = 0;
	while(data[cnt] != '\0')
	{
		sendData(data[cnt]);
		++cnt;
	}
}

void t_lcd::gotoXY(uint8_t x, uint8_t y)
{
	uint8_t gotoCMD = 0x80;
	if(y)
		gotoCMD |= 0x40;
	gotoCMD |= x;
	sendCmd(gotoCMD);
}

void t_lcd::sendStringXY(uint8_t x, uint8_t y, char *data)
{
	gotoXY(x,y);
	sendString(data);
}

void t_lcd::writeInt(uint32_t value, uint8_t padding)
{
	char stringToPrint[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	uint8_t pos = 19;
	uint8_t start = 0;
	while(value)
	{
		stringToPrint[pos] = value%10;
		value /= 10;
		--pos;
	}

	if(padding == 0)
		while(stringToPrint[start] == 0) ++start;
	else
		start = 20 - padding;

	for(pos = start; pos != 20; ++pos)
		sendData(stringToPrint[pos]+48);
}

void t_lcd::writeIntXY(uint8_t x, uint8_t y, uint32_t value, uint8_t padding)
{
	gotoXY(x,y);
	writeInt(value, padding);
}

void t_lcd::setDBPort(uint8_t bits)
{
	if(bits & (1 << 0))
		SETPIN(DB4_PORT, DB4_PIN);
	else
		CLEARPIN(DB4_PORT, DB4_PIN);

	if(bits & (1 << 1))
		SETPIN(DB5_PORT, DB5_PIN);
	else
		CLEARPIN(DB5_PORT, DB5_PIN);

	if(bits & (1 << 2))
		SETPIN(DB6_PORT, DB6_PIN);
	else
		CLEARPIN(DB6_PORT, DB6_PIN);

	if(bits & (1 << 3))
		SETPIN(DB7_PORT, DB7_PIN);
	else
		CLEARPIN(DB7_PORT, DB7_PIN);
}

void t_lcd::sendByte(uint8_t data, bool type)
{
	uint8_t tmpData;
	if(type)
		SETPIN(RS_PORT, RS_PIN);
	else
		CLEARPIN(RS_PORT, RS_PIN);

	tmpData = data >> 4;
	for(uint8_t teller = 0; teller != 2; ++teller)
	{
		_delay_us(1);
		SETPIN(E_PORT, E_PIN);

		setDBPort(tmpData);
		
		_delay_us(1);

		CLEARPIN(E_PORT, E_PIN);

		tmpData = data;
	}
	_delay_ms(1);
	waitBusy();
}
