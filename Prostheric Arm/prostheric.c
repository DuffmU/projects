#include <avr/io.h> 
#include <avr/interrupt.h>

#define BYTE unsigned char 
#define t_TIME unsigned long int
#define LOOP() for(;;)

#define LATCH_PIN 4
#define CLOCK_PIN 5
#define SHIFT_PORT PORTB

#define STEPS 8
#define STEP_DELAY_MS 1000/STEPS

volatile t_TIME time_mil = 0;
BYTE stepper_steps[STEPS] = {0x60, 0x20, 0x30, 0x10, 0x18, 0x08, 0x48, 0x40};

const BYTE latch_mk = 1<<LATCH_PIN;
const BYTE clock_mk = 1<<CLOCK_PIN;

typedef struct Steppers_connections{
    volatile uint8_t *port;
    BYTE pin;
    BYTE pin_mk;
};

struct Steppers_connections stepper[13]; 

void setTimer0();
void setPorts();
void setSteppers();

void applyMask(volatile uint8_t *port, BYTE mask, BYTE operation); 
void shiftData(BYTE val, BYTE stepperN);
void step(BYTE stepperN, BYTE dir);

int main(void){
    sei();
    setTimer0();
    setPorts();
    setSteppers();
    
    LOOP(){
    
    }

	return 0;
}    

ISR (TIMER0_COMPA_vect){ 
    ++time_mil;
}

void setPorts(){
    //steppers
    applyMask(&DDRC, 0x3F, 'S'); //0-5
    applyMask(&DDRB, 0x0F, 'S'); //6-9
    applyMask(&DDRD, 0xE0, 'S'); //10-12
    //latch
    DDRB |= 0x10;
    //clock
    DDRB |= 0x20;
    
}

void setSteppers(){
    BYTE i = 0;
    
    for(; i < 6; ++i){
        stepper[i].port = &DDRC;
        stepper[i].pin = 5-i;
        stepper[i].pin_mk = 1<<stepper[i].pin;
    }
    for(; i < 10; ++i){
        stepper[i].port = &DDRB;
        stepper[i].pin = 9-i;
        stepper[i].pin_mk = 1<<stepper[i].pin;
    }        
    for(; i < 13; ++i){
        stepper[i].port = &DDRD;
        stepper[i].pin = 17-i;  
        stepper[i].pin_mk = 1<<stepper[i].pin;      
    }
}

void setTimer0(){
    TCCR0A |= (1<<WGM01);
    OCR0A = 0xF9; 
    TIMSK0 |= (1<<OCIE0A);
    TCCR0B |= (1<<CS01) | (1<<CS00); 
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

void shiftData(BYTE val, BYTE stepperN){
	BYTE i;
	
	applyMask(&SHIFT_PORT, latch_mk, 'R');
	
	for(i = 0; i < 8; ++i){			
		if(!!(val & (1<<i)))
			applyMask(stepper[stepperN].port, stepper[stepperN].pin_mk, 'S');
		else
			applyMask(stepper[stepperN].port, stepper[stepperN].pin_mk, 'R');
			
		applyMask(&SHIFT_PORT, clock_mk, 'S');
		applyMask(&SHIFT_PORT, clock_mk, 'C');
	}
	
	applyMask(&SHIFT_PORT, latch_mk, 'S');
}

void step(BYTE stepperN, BYTE dir){
    t_TIME stdelay = time_mil;
    BYTE i = 0;
    
    if (dir > 0){
        while (i < STEPS){
            if(time_mil - stdelay > STEP_DELAY_MS){
                shiftData(stepper_steps[i], stepperN);
                stdelay = time_mil;
            }
        }
    } else {
        while (i < STEPS){
            if(time_mil - stdelay > STEP_DELAY_MS){
                shiftData(stepper_steps[STEPS - i - 1], stepperN);
                stdelay = time_mil;
            }
        }        
    }
}
