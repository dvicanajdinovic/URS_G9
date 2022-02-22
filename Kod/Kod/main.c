#define F_CPU 7372800UL

#define MESSAGE_DURATION 500
#define INPUT_DURATION 200 
#define KEYPAD_DELAY 200 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "lcd.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define PIR_DDR		DDRC
#define PIR_PORT	PORTC
#define PIR_PIN		PINC
#define PIR_SENSOR	PC0

#define KEY_DDR 	DDRB
#define KEY_PORT	PORTB
#define KEY_PIN		PINB

#define SIGNAL_DDR 	DDRA
#define SIGNAL_PORT	PORTA
#define SIGNAL_PIN	PINA
#define LED			0

#define PASS_SIZE	6
#define LCD_WIDTH   16

uint8_t reset_password_input = 0;
uint8_t armed = 0;
uint16_t timer0_multip = 0;

unsigned char keypad[4][4] = {
	{'D','#','0','*'},
	{'C','9','8','7'},
	{'B','6','5','4'},
	{'A','3','2','1'}
};

char get_char() {
	uint8_t r,c;
	
	KEY_DDR = 0x0f;
	KEY_PORT = 0x0f;
	
	for (c = 0; c < 4; c++) {
		KEY_DDR = (0b10000000 >> c);
		
		for (r = 0; r < 4; r++) {
			if((KEY_PIN & (0b00001000 >> r)) == 0) {
				return keypad[r][c];
			}
		}
	}
	return 'w';
}

void user_input(char *input) {

	uint8_t i = 0;
	while (i < PASS_SIZE) {
		// Enable timer0 to limit user input time
		if(i == 1) {
			TIMSK = _BV(OCIE0);
		}

		
		char c = get_char();
		
		if ((c != 'w') & (c != 'A') & (c != 'B') & (c != 'C') & (c != 'D') & (c != '*') & (c != '#')) {
			// Reset timer0
			timer0_multip = 0;

			*(input + i) = c;

			lcd_gotoxy((LCD_WIDTH - PASS_SIZE) / 2 + i, 1);
			// lcd_putc('*');
			lcd_putc(c);
			
			i++;
		}
		_delay_ms(KEYPAD_DELAY);
		
		if(reset_password_input) {
			// Disable and reset timer0, set i to point to the begining
			TIMSK &= ~_BV(OCIE0);
			i = 0;
			reset_password_input = 0;

			lcd_clrscr();
			lcd_puts("Input timed out,");
			lcd_gotoxy(3, 1);
			lcd_puts("try again.");
			_delay_ms(MESSAGE_DURATION);

			lcd_clrscr();
			lcd_puts("Password:");
		}
		
	}
	// Call delay function, so the last '*' can be shown too
	_delay_ms(MESSAGE_DURATION);
}

int verify_password(char *tmp_password, char *password) {
	if (!strcmp(tmp_password, password)) {
		lcd_clrscr();
		lcd_puts("Password");
		lcd_gotoxy(3, 1);
		lcd_puts("correct.");
		_delay_ms(MESSAGE_DURATION);

		return 1;
	} else {
		lcd_clrscr();
		lcd_puts("Password");
		lcd_gotoxy(3, 1);
		lcd_puts("incorrect.");
		_delay_ms(MESSAGE_DURATION);

		return 0;
	}
}

void disArm(char *password, char option) {	
	lcd_clrscr();
	lcd_puts("Enter password:");

    char *tmp_password = (char*) malloc(7 * sizeof(char));
	
	user_input(tmp_password);
	
	*(tmp_password+6) = '\0';

    // Disable timer0
	TIMSK &= ~_BV(OCIE0);

	if (verify_password(tmp_password, password)) {
		free(tmp_password);
		if (option == 'A') {
			armed = 1;

			lcd_clrscr();
			lcd_puts("Alarm armed.");
			_delay_ms(MESSAGE_DURATION);
		} else if (option == 'D') {
			// If turned on, turn off the alarm
			if (SIGNAL_PORT == 0xfe) SIGNAL_PORT = 0xff;

			armed = 0;

			lcd_clrscr();
			lcd_puts("Alarm disarmed.");
			_delay_ms(MESSAGE_DURATION);
		}	
	} else {
		lcd_clrscr();
		if(armed) {
			lcd_puts("Alarm armed.");
		} else {
			lcd_puts("Alarm disarmed.");
		}
		//_delay_ms(MESSAGE_DURATION);
	}
}

void change_password(char *password) {
	lcd_clrscr();
	lcd_puts("Enter old");
	lcd_gotoxy(3, 1);
	lcd_puts("password.");
	_delay_ms(MESSAGE_DURATION);

	lcd_clrscr();
	lcd_home();
	lcd_puts("Password:");
	
	char *tmp_password = (char*) malloc(7 * sizeof(char));
	user_input(tmp_password);

	if (verify_password(tmp_password, password)){
		free(tmp_password);

		lcd_clrscr();
		lcd_puts("Please enter");
		lcd_gotoxy(3, 1);
		lcd_puts("new password:");
		_delay_ms(MESSAGE_DURATION);
		
		lcd_clrscr();
		lcd_home();
		lcd_puts("Password:");
		
		char *new_password = (char*) malloc(7 * sizeof(char));
		user_input(new_password);

		strcpy(password, new_password);
		
		lcd_clrscr();
		lcd_puts("Password");
		lcd_gotoxy(3, 1);
		lcd_puts("changed.");
		_delay_ms(MESSAGE_DURATION);
		
		lcd_clrscr();
		lcd_home();
		
		if(armed) {
			lcd_puts("Alarm armed.");
		} else {
			lcd_puts("Alarm disarmed.");
		}
	} else {
		lcd_clrscr();
		if(armed) {
			lcd_puts("Alarm armed.");
		} else {
			lcd_puts("Alarm disarmed.");
		}
		//_delay_ms(MESSAGE_DURATION);
	}
}

void set_password(char *password) {
	lcd_clrscr();
	lcd_puts("Please set your");
	lcd_gotoxy(3, 1);
	lcd_puts("password!");
	_delay_ms(MESSAGE_DURATION);

	lcd_clrscr();
	lcd_home();
	lcd_puts("Password:");

	user_input(password);
	*(password+6) = '\0';

	// Disable timer0
	TIMSK &= ~_BV(OCIE0);
	
	lcd_clrscr();
	lcd_gotoxy(1, 0);
	
	lcd_puts("Password set!");
	_delay_ms(MESSAGE_DURATION);
	lcd_clrscr();
	lcd_puts("Alarm armed.");
	_delay_ms(MESSAGE_DURATION);
	armed = 1;
}

ISR(TIMER0_COMP_vect) {
	timer0_multip++;

	// Reset user input after 10 seconds
	// 	if(timer0_multip >= 280) {
	if(timer0_multip >= INPUT_DURATION) {
		timer0_multip = 0;
		reset_password_input = 1;
	}
}

int main(void) {
	DDRD = _BV(4);
	PIR_DDR = 0x00;
	SIGNAL_DDR = 0xff;
	SIGNAL_PORT = 0xff;
	
	// Timer 0 CTC mode, 1024 prescaler
	TCCR0 = _BV(WGM01) | _BV(CS00) | _BV(CS02);
	OCR0 = 255;
	sei();

	// LCD
	TCCR1A = _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1B = 128;
	
	lcd_init(LCD_DISP_ON);

	char *password = (char*) malloc(7 * sizeof(char));
	set_password(password);

	while (1) {
		char option = get_char();
		_delay_ms(KEYPAD_DELAY);
		
		if (armed) {
			// If movement detected, sound the alarm
			if(PIR_PIN & (1 << 0)) {
				SIGNAL_PORT = 0xfe;
			}
			
			if(option == 'D') {
				// Disarm
				disArm(password, option);
			}
		} else {
			if(option == 'C') {
				change_password(password);
			} else if(option == 'A') {
				// Arm
				disArm(password, option);
			}
		} 
	}

	free(password);
}
