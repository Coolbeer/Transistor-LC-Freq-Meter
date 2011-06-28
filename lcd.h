#ifndef LCD_H
#define LCD_H

#define RS_PIN 2
#define RS_PORT D

#define RW_PIN 6
#define RW_PORT D

#define E_PIN 7
#define E_PORT D

#define DB4_PIN 0
#define DB4_PORT B

#define DB5_PIN 1
#define DB5_PORT B

#define DB6_PIN 2
#define DB6_PORT B

#define DB7_PIN 3
#define DB7_PORT B

#define _CONCAT(a,b) a##b
#define _CONCAT3(a,b,c) a##b##c
#define PORT(x) _CONCAT(PORT,x)
#define PIN(x) _CONCAT(PIN,x)
#define DDR(x) _CONCAT(DDR,x)
#define DD(x,y) _CONCAT3(DD,x,y)
#define PORTX(x,y) _CONCAT3(PORT,x,y)
#define PINX(x,y) _CONCAT3(PIN,x,y)

#define SETPIN(x,y) PORT(x) |= (1 << PORTX(x, y))
#define CLEARPIN(x,y) PORT(x) &= ~(1 << PORTX(x, y))

#define sendCmd(c) (writeByte(c,false))
#define sendData(d) (writeByte(d,true))

#ifndef F_CPU
	#define F_CPU 10000000
#endif

#include <avr/io.h>

enum initFlags { DUALDISPLAY = 0x08, FONT = 0x04 };
enum displayFlags { DISPLAYON = 0x04, CURSORON = 0x02, CURSORBLINK = 0x01};
enum shiftFlags { LEFTSHIFT = 0x00, RIGHTSHIFT = 0x04 };

class t_lcd
{
	public:
		void init(initFlags flags = DUALDISPLAY);
		void writeByte(uint8_t data, bool type);
		void writeString(char *data);
		void writeStringXY(uint8_t x, uint8_t y, char *data);
		void gotoXY(uint8_t x, uint8_t y);
		void writeInt(uint32_t value, uint8_t padding = 0);
		void writeIntXY(uint8_t x, uint8_t y, uint32_t value, uint8_t padding = 0);
		void clearDisplay(void);
		void returnHome(void);
		void setDisplayFlags(displayFlags flags = DISPLAYON);
		void displayShift(shiftFlags flags = LEFTSHIFT);
	private:
		void setDBPort(uint8_t data);
		void waitBusy(void);
};


#endif
