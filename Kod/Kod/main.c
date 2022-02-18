#define F_CPU 7372800UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

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
#define LED			0

#define PASS_SIZE	6
#define LCD_WIDTH   16

uint8_t reset_password_input = 0;
uint8_t armed = 0;
uint8_t password_set = 0;
uint8_t movement_detected = 0;
uint16_t timer0_multip = 0;

uint8_t coordinates[2] = {3, 4};
unsigned char keypad[4][5] = {
	{'D','#','0','*'},
	{'C','9','8','7'},
	{'B','6','5','4'},
	{'A','3','2','1', 'w'}
};

// +1 for '\0'
char password[PASS_SIZE + 1];
char temporary_password[PASS_SIZE + 1];

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

void check_password() {
	if (!strcmp(temporary_password, password)) {
		armed = 0;
		movement_detected = 0;
		
		lcd_clrscr();
		lcd_puts("You got it!");
	} else {
		lcd_clrscr();
		lcd_puts("Pass incorrect");

	}
}

void enter_password() {
	//disable timer0
	TIMSK &= ~_BV(OCIE0);
	
	lcd_clrscr();
	lcd_puts("Enter password:");
	
	uint8_t i = 0;
	while (i < PASS_SIZE) {
		//enable timer0
		if(i == 1) {
			TIMSK = _BV(OCIE0);
		}
		get_char();
		char c = keypad[coordinates[0]][coordinates[1]];
		if (c != 'w') {
			timer0_multip = 0;
			temporary_password[i] = c;
			lcd_gotoxy((LCD_WIDTH - PASS_SIZE) / 2 + i, 1);
			lcd_putc(c);
			i++;
		}
		_delay_ms(200);
		
		if(reset_password_input) {
			i = 0;
			reset_password_input = 0;
			lcd_clrscr();
			lcd_puts("Enter password:");
		}
		
	}
	//disable timer0
	TIMSK &= ~_BV(OCIE0);
	
	// call delay function, so the last '*' can be shown too
	_delay_ms(1000);
	
	temporary_password[PASS_SIZE] = '\0';
	check_password();
}

//dodaj neku bool varijablu koja ce kontrolirati smije li se unijeti novi pass
void change_password() {
	enter_password();
	check_password(temporary_password);
	enter_password();
	strcpy(password, temporary_password);
	
	lcd_clrscr();
	lcd_puts("Pass changed");
}

void set_password() {
	
	//disable timer0
	TIMSK &= ~_BV(OCIE0);
	lcd_clrscr();
	lcd_puts("Please set your");
	lcd_gotoxy(3, 1);
	lcd_puts("password!");
	_delay_ms(1000);
	lcd_clrscr();
	lcd_home();
	lcd_puts("Password:");
	
	uint8_t i = 0;
	while (i < PASS_SIZE) {
		//enable timer0
		if(i == 1) {
			TIMSK = _BV(OCIE0);
		}
		get_char();
		char c = keypad[coordinates[0]][coordinates[1]];
		if (c != 'w') {
			timer0_multip = 0;
			password[i] = c;
			lcd_gotoxy((LCD_WIDTH - PASS_SIZE) / 2 + i, 1);
			lcd_putc(c);
			i++;
		}
		_delay_ms(200);
		
		if(reset_password_input) {
			i = 0;
			reset_password_input = 0;
			lcd_clrscr();
			lcd_home();
			lcd_puts("Password:");
		}
	
	}
	
	//disable timer0
	TIMSK &= ~_BV(OCIE0);
	
	// call delay function, so the last '*' can be shown too
	_delay_ms(1000);
	
	lcd_clrscr();
	lcd_gotoxy(1, 0);
	password[PASS_SIZE] = '\0';
	lcd_puts("Password set!");
	_delay_ms(1000);
	lcd_clrscr();
	lcd_puts("Armed");
	_delay_ms(1000);
	

	password_set = 1;
	armed = 1;
}

ISR(TIMER0_COMP_vect) {
	timer0_multip++;
	//if(timer0_multip >= 280) {
	if(timer0_multip >= 50) {
		timer0_multip = 0;
		reset_password_input = 1;
	}
}

int main(void)
{
	DDRD = _BV(4);
	PIR_DDR = 0x00;
	SIGNAL_DDR = 0xff;
	SIGNAL_PRT = 0xff;
	
	//Timer 0 CTC mode, 1024 prescaler
	TCCR0 = _BV(WGM01) | _BV(CS00) | _BV(CS02);
	OCR0 = 255;
	sei();

	TCCR1A = _BV(COM1B1) | _BV(WGM10);
	TCCR1B = _BV(WGM12) | _BV(CS11);
	OCR1B = 128;
	
	lcd_init(LCD_DISP_ON);
	
	set_password();
	
	while (1) {
		get_char();
		char c = keypad[coordinates[0]][coordinates[1]];
		_delay_ms(200);
		if (!armed) {
			SIGNAL_PRT = 0xff;
			lcd_clrscr();
			lcd_puts("Unlocked!");
		} else if(c == 'A') {
			enter_password();
		} else {
			//movement detected
			if(PIR_PIN & (1 << 0)) {
				SIGNAL_PRT = 0xfe;
				movement_detected = 1;
			}
			
			
		}
	}
}


