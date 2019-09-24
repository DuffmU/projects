#include <avr/io.h> 
#include <avr/interrupt.h>

#define LED_RED_PIN 6 
#define LED_GREEN_PIN 5
#define LED_PORT PORTD 
#define LED_DDR DDRD 

#define BYTE unsigned char 
#define t_TIME unsigned long int
#define LOOP() for(;;)

const BYTE redmk = 1<<LED_RED_PIN;
const BYTE greenmk = 1<<LED_GREEN_PIN;

volatile t_TIME time_mil = 0;

void applyMask(volatile uint8_t *port, BYTE mask, BYTE operation); 
void setTimer0();

int main(void){
	LED_DDR |= redmk|greenmk;   
	 
    sei(); //enable interrupts
    setTimer0();
    
    t_TIME temp1 = time_mil;
    t_TIME temp2 = time_mil;
    
    applyMask(&LED_PORT, redmk|greenmk, 'S');

    LOOP(){
        if(time_mil - temp1 > 500){
            applyMask(&LED_PORT, redmk, 'C');
            temp1 = time_mil;
        }
        
        if(time_mil - temp2 > 1000){
            applyMask(&LED_PORT, greenmk, 'C');
            temp2 = time_mil;
        }        
    }

	return 0;
}

ISR (TIMER0_COMPA_vect){  // timer0 overflow interrupt  
    ++time_mil;
}

void setTimer0(){
    TCCR0A |= (1 << WGM01); // Set the Timer Mode to CTC
    OCR0A = 0xF9; // Set the value that you want to count to
    TIMSK0 |= (1 << OCIE0A); //Set the ISR COMPA vect
    TCCR0B |= (1 << CS01) | (1 << CS00); // set prescaler to 64 and start the timer	 
}

void applyMask(volatile uint8_t *port, BYTE mask, BYTE operation){
	switch (operation){
		case 'S':
			*port |= mask; //побитово складываем
			break;
		case 'R':
			*port &= ~mask; //побитово умножаем на инверсированную побитово маску
			break;
		case 'C':
			*port ^= mask; //побитово меняем на противоположный (xor в помощь)
			break;
	}
}
