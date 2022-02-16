#define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define PIR_DDR		DDRC
#define PIR_PRT		PORTC
#define PIR_PIN		PINC
#define PIR_SENSOR	PC0

#define KEY_DDR 	DDRB
#define KEY_PRT		PORTB
#define KEY_PIN		PINB

#define SIGNAL_DDR 	DDRA
#define SIGNAL_PRT	PORTA
#define SIGNAL_PIN	PINA

uint8_t coordinates[2] = {3, 4};
unsigned char keypad[4][5] = {{'D','#','0','*'},
{'C','9','8','7'},
{'B','6','5','4'},
{'A','3','2','1', 'w'}};

char password[5] = {'1', '2', '3', '4', '\0'};

void get_char() {
	uint8_t r,c;
	
	KEY_DDR = 0x0f;
	KEY_PRT = 0x0f;
	
	for (c = 0; c < 4; c++) {
		KEY_DDR = (0b10000000 >> c);
		
		for (r = 0; r < 4; r++) {
			if((KEY_PIN & (0b00001000 >> r)) == 0) {
				coordinates[0] = r;
				coordinates[1] = c;
				return;
			}
		}
	}
	coordinates[0] = 3;
	coordinates[1] = 4;
}

void check_password(char temp[]) {
	if (!strcmp(temp, password)) {
		lcd_clrscr();
		lcd_puts("You got it");
		} else {
		lcd_clrscr();
		lcd_puts("Password incorrect");
	}
}

void enter_password() {
	char temporary_password[5];
	
	uint8_t i = 0;
	while (i != 4) {
		get_char();
		char c = keypad[coordinates[0]][coordinates[1]];
		if (c != 'w') {
			temporary_password[i] = c;
			lcd_gotoxy(0, 1);
			i++;
		}
		_delay_ms(200);
	}
	
	temporary_password[4] = '\0';
	check_password(temporary_password);
}

int main(void)
{
	DDRD = _BV(4);

	TCCR1A = _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1B = 128;

	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	
	
	//DDR and PORT setup for LEDS (or buzzer)
	SIGNAL_DDR = 0x00;
	SIGNAL_PRT = 0x00;
	
	
	while (1) {
		if (SIGNAL_DDR & 0x01) {
			lcd_clrscr();
			lcd_puts("NO");
			} else {
			lcd_clrscr();
			lcd_puts("Enter password:");
			enter_password();
		}
		_delay_ms(10000);
	}
}
