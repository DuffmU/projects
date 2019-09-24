#include <avr/io.h>
#include <avr/interrupt.h>

void Setup();
void Sonar_pulse();

volatile int time = 0, i = 0, pulse_with = 0, speed = 47, working_time = 0, DisDet = 0, ADCresult = 0;

/*
PE0 = TR
PE2 = enable
PE3 = half/full
PE4 = clock
PE5 = control
PE6 = echo
*/
int main(){

	Setup();
	sei();
	
	while(1){
		
		if(DisDet == 1){
			
			EIMSK ^= (1<<INT0);
			
			DisDet = 0;
			
			PORTE |= 0b00100100;
			
			TCCR1B = (1<<CS12) | (1<<CS10); //On 16-bit timer/counter1, clk/1024
			
			OCR2 = speed;
			TCCR2 = (1<<WGM21) | (1<<CS22) | (1<<CS20); //8-bit timer/couner2, Clk/1024
			
			while(working_time < 105){
				if((ADCSRA & (1<<ADSC)) == !1){
					ADCSRA |= (1<<ADSC);
				}
				
				if((pulse_with / 58 <= 400) && (pulse_with / 58 >= 370)){
					working_time -= 30;
				}
				
				if(working_time > 70){
					Sonar_pulse();
				}
			}
			TCCR2 = 0;
			TCCR1B = 0;
			working_time = 0;
			PORTE ^= 0b00100100;
			EIMSK = (1<<INT0); //INT0 - externally interrupted
		}
		
	}
	
}

void Setup(){

	DDRE = 0b00111101;
	
	OCR0 = 16; //1uS
	
	TIMSK = (1<<OCIE2) | (1<<OCIE1A) | (1<<OCIE0); //Interrupts on 8-bit timer/counter2 | Interrupt on compare 16-bit timer/counter1 | Interrupts on compare 8-bit timer/counter0

	OCR1AH = 0b11110100;//4S
	OCR1AL = 0b00100100;
	
	TCCR1A = (1<<WGM12); //16-bit timer/couner1, CTC
	
	EICRA = (1<<ISC01) | (1<<ISC00); //INT0 - externally interrupted
	EIMSK = (1<<INT0);
	
	EICRB = (1<<ISC60);
	
	ADMUX = (1<<REFS0); //set ADC on ADC0 pin
	ADCSRA = (1<<ADEN) | (1<<ADSC) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0); //Interrupt on conversion completion, Clk/128, enable ADC, Start ADC
	
	return;
}

ISR(TIMER0_COMP_vect){
	
	++time;
	
	return;
}

ISR(TIMER2_COMP_vect){
	
	cli();
	
	PORTE ^= (1<<4);
	
	if(OCR2 != speed){
		OCR2 = speed;
		TCCR2 |= (1<<CS22) | (1<<CS20); //8-bit timer/couner2, Clk/1024
		
		sei();
		
		return;
	}
	
	OCR2 = 10; //~0,5us
	TCCR2 ^= (1<<CS20); //Clk/1
	
	sei();
	
	return;
}

ISR(TIMER1_COMPA_vect){
	
	cli();
	
	++working_time;
	
	sei();
	
	return;
}

ISR(ADC_vect){
	
	cli();
	
	ADCresult = ADCL | (ADCH<<8);
	
	if((ADCresult < 200) && (ADCresult > 10)){
		OCR2 = 47; //~3ms
	}
	else if((ADCresult >= 200) && (ADCresult <= 700)){
		OCR2 = 109; //~7ms
	}
	else if(ADCresult > 700){
		OCR2 = 141; //~9ms
	}
	
	sei();
	
	return;
}

ISR(INT6_vect){
	
	cli();
	
	if(i == 1){
		TCCR0 = 0; //Off timer
		EIMSK ^= (1<<INT6); //INT06 - off interrupts
		pulse_with = TCNT0;
		TCNT0 = 0; //Clear counter
		OCR0 = 250; //1mS
		TCCR0 = (1<<WGM01) | (1<<CS02); //On timer, CTC, Clk/64
		--i;
		sei();
		
		return;
	}
	
	if(i == 0){
		TCCR0 = (1<<CS00); //On timer, Clk/1
		++i;
	}
	
	sei();
	
	return;
}

ISR(INT0_vect){
	
	cli();
	
	DisDet = 1;
	
	sei();
	
	return;
}

void Sonar_pulse(){

	if(((PORTE & 1) != 1) && (OCR0 == 16)){ //start impulse
		PORTE |= 1;
		OCR0 = 16; //1uS
		TCCR0 = (1<<WGM01) | (1<<CS00); //On 8-bit timer/couner0, CTC, prescaler(Clk) = 1
	}
	
	if((time >= 10) && (OCR0 == 16)){ //stop impulse
		TCCR0 ^= (1<<CS00); //Off timer/couner0
		time = 0;
		PORTE ^= 1;
		EIMSK = (1<<INT6); //INT06 - Externally interrupted
	}
	
	if((time >= 10) && (OCR0 == 250)){
		OCR0 = 16; //1us
		time = 0;
	}
	
	return;
}