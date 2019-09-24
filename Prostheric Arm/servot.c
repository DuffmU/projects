#include <avr/io.h> 
//#include <avr/interrupt.h>
#include <util/delay.h> 

#define SERVO_DDR DDRB
#define SERVO_PIN 1

#define SERVO_MAX 2400
#define SERVO_MIN 544

#define BYTE unsigned char 
#define t_TIME unsigned long int
#define LOOP() for(;;)

//volatile t_TIME time_mil = 0;

const BYTE servo_step = (SERVO_MAX-SERVO_MIN)/180;

//void applyMask(volatile uint8_t *port, BYTE mask, BYTE operation); 
//void setTimer0();

int main(void){
    //sei();
    //setTimer0();
    
    TCCR1A |= (1<<COM1A1) | (1<<COM1B1) | (1<<WGM11);        //NON Inverted PWM
    TCCR1B |= (1<<WGM13) | (1<<WGM12) | (1<<CS11) | (1<<CS10); //PRESCALER=64 MODE 14(FAST PWM)

    ICR1 = 4999;  //fPWM=50Hz (Period = 20ms Standard).

    SERVO_DDR |= (1<<SERVO_PIN);
   
    LOOP(){
        OCR1A = SERVO_MIN/4;
        _delay_ms(1000);
        OCR1A = SERVO_MAX/4;
        _delay_ms(1000);        
    }

	return 0;
}    
/*
ISR (TIMER0_COMPA_vect){ 
    ++time_mil;
}

void setTimer0(){
    TCCR0A |= (1 << WGM01); 
    OCR0A = 0xF9; 
    TIMSK0 |= (1 << OCIE0A);
    TCCR0B |= (1 << CS01) | (1 << CS00); 
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
*/
