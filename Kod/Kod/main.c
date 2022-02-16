#define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"
#include "stdlib.h"
#include "stdio.h"

#define PIR_DDR		DDRC
#define PIR_PRT		PORTC
#define PIR_PIN		PINC
#define PIR_sensor	PC0

#define KEY_DDR 	DDRB
#define KEY_PRT		PORTB
#define KEY_PIN		PINB

uint8_t coordinates[2] = {-1, -1};
unsigned char keypad[4][4] = {	{'D','#','0','*'},
{'C','9','8','7'},
{'B','6','5','4'},
{'A','3','2','1'}};

void GetKeyPressed() {
	uint8_t r,c;
	
	KEY_DDR = 0x0f;
	KEY_PRT = 0x0f;
	
	// for every column
	for (c = 0; c < 4; c++) {
		KEY_DDR = (0b10000000 >> c);
		
		// check every row
		for (r = 0; r < 4; r++) {
			if((KEY_PIN & (0b00001000 >> r)) == 0) {
				coordinates[0] = r;
				coordinates[1] = c;
				return;
			}
		}
	}
	
}

int main(void)
{
	// LCD setup
	DDRD = _BV(4);

	TCCR1A = _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1B = 128;

	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	

	while (1) {
		lcd_clrscr();
		GetKeyPressed();
		lcd_putc(keypad[coordinates[0]][coordinates[1]]);
		_delay_ms(200);
	}
}
