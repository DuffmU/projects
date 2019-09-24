#include <avr/io.h> 
#include <util/delay.h> 

#define DATA_PIN 2 
#define LATCH_PIN 4
#define CLOCK_PIN 7
 
#define SHIFT_PORT PORTD 
#define SHIFT_DDR DDRD

#define LOOP() for(;;)
#define BYTE unsigned char

const BYTE datamk = 1<<DATA_PIN;
const BYTE latchmk = 1<<LATCH_PIN;
const BYTE clockmk = 1<<CLOCK_PIN;

const BYTE Step[4] = {127, 63, 31, 15}

void applyMask(volatile uint8_t *port, BYTE mask, BYTE operation); 
void shiftData(BYTE val);

int main(void){
	SHIFT_DDR |= datamk|latchmk|clockmk;
	applyMask(&SHIFT_PORT, datamk|latchmk|clockmk, 'R');
	
	LOOP(){
		BYTE i;
	
		for(i = 0; i < 3; i++){
			shiftData(Step[i]);
			_delay_ms(1000);
		}
	}
	
	return 0;
}

void applyMask(volatile uint8_t *port, BYTE mask, BYTE operation){
	switch (operation){
		case 'S':
			*port |= mask;
			break;
		case 'R':
			*port &= ~mask;
			break;
		case 'C':
			*port ^= mask;
			break;
	}
}

void shiftData(BYTE val){
	BYTE i;
	
	applyMask(&SHIFT_PORT, latchmk, 'R');
	
	for(i = 0; i < 8; ++i){			
		if(!!(val & (1<<i)))
			applyMask(&SHIFT_PORT, datamk, 'S');
		else
			applyMask(&SHIFT_PORT, datamk, 'R');
			
		applyMask(&SHIFT_PORT, clockmk, 'S');
		applyMask(&SHIFT_PORT, clockmk, 'C');
	}
	
	applyMask(&SHIFT_PORT, latchmk, 'S');
}









