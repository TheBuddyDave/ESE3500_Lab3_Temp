#include <avr/io.h>
#include <util/delay.h> 
#include <avr/interrupt.h>
#include "uart.h" 
#define LED_PIN PB5  
#define F_CPU 16000000UL 


//state management (send = 1 -> start read , send = 0 -> waiting for read result) 
static volatile int send = 1;
    
//Tick counter for echo
static volatile uint16_t val = 0;

//Discrete vs continious mode tracker: 
static volatile int discrete = 0;


//Helper to set up the GPIO: 
/* 
 Pin key: 
 * PC0 = input pin to read the photo resistor value for ADC
 * PD6 = 50 percent duty 
 * PD5 = PWM (changeable duty) 
 * PB1 = Trig pin for distance sensor 
 * PB0 = Echo read pin for distance sensor 
 * PB5 = "button" pin that will toggle discrete mode 
 */
void config_GPIO() {
    //Configure da pins ty shi: 
    
    //Debug pin
    DDRB |= (1 << LED_PIN); //LED PIN for debug
    
    //PWM pins
    DDRC &= ~(1 << PC0); //PC0 pin for Photo-resistor read
    DDRD |= (1<<DDD6) | (1<<DDD5); // PDD6 - OCRA 50percent PWM and PDD5 - Duty cycle PWM  
    
    //Distance capture pin 
    DDRB |= (1 << PB1); //trig isignal pin 
    DDRB &= ~(1 << PB0); //input capture for 
    
    
//    //set up for the button press: 
    PCICR |= (1 << PCIE0); //turn on pin chnage interrupt for port B 
    PCMSK0 = (1 << PCINT5); //BIT mask to only react to PB6 


}


//Helper to conifgure the ADC
void set_ADC() {
        // clear power reduction register bit 
        PRR0 &= ~(1 << PRADC); 

        // select the Vref to AVcc 
        ADMUX |= (1 << REFS0);
        ADMUX &= ~(1 << REFS1);

        // set the ADC clock div by 128 
        ADCSRA |= (1 << ADPS0);
        ADCSRA |= (1 << ADPS1); 
        ADCSRA |= (1 << ADPS2); 

        // select PC0 as channel pin
        ADMUX &= ~(1 << MUX0); 
        ADMUX &= ~(1 << MUX1); 
        ADMUX &= ~(1 << MUX2); 
        ADMUX &= ~(1 << MUX3); 

        // auto triggering for the ADC
        ADCSRA |= (1 << ADATE); 

        // free runnning mode
        ADCSRB &= ~(1 << ADTS0); 
        ADCSRB &= ~(1 << ADTS1); 
        ADCSRB &= ~(1 << ADTS2); 

        // disable Digital Input buffer on ADC pin 
        DIDR0 |= (1 << ADC0D); 

        // enable ADC:
        ADCSRA |= (1 << ADEN); 

        // start Conversion: 
        ADCSRA |= (1 << ADSC); 
}


//Helper to set up PWM wrt timer0
void set_PWM() {
        // timer 0 is 8 bit
        // next three lines scale timer0 by a factor of 64
        TCCR0B |= (1<<CS00);
        TCCR0B |= (1<<CS01);
        TCCR0B &= ~(1<<CS02);

        // set timer 0 to phase correct pwm mode 5
        // see 18-9, 18-7
        TCCR0A |= (1<<WGM00);
        TCCR0A &= ~(1<<WGM01);
        TCCR0B |= (1<<WGM02);

        TCCR0A &= ~(1<<COM0A1);
        TCCR0A |= (1<<COM0A0);

        TCCR0A |=  (1<<COM0B1);
        TCCR0A &= ~(1<<COM0B0);

        OCR0A = 200; // set freq
        OCR0B = 29;// set 50% duty cycle
}


//Helper to set up Echo duration wrt timer1
void set_EchoTimer() {
    // ---- SET UP ECHO TIMER (TIMER1 Since its 16 bit)) ----- 
        //Edge settings:
        TCCR1B |= (1 << 6);
        //interrupt enabled:
        TIMSK1 |= (1 << 5);    
        // Clear prescaler bits first
        TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
        // Set prescaler to 1024 (CS12=1, CS11=0, CS10=1)
        TCCR1B |= (1 << CS12) | (1 << CS10);  
    // ---------------------------------- 
}




//helper function that will change the duty cylce wrt the ADC value: 
void toggle_duty() {
            _delay_ms(1000);
            printf("ADCC: %u \r \n", ADC); 
            if (ADC < 320) {
                printf("DUTY (in percent): 5 \r \n" );
                OCR0B = 1 * OCR0A / 15; 
            } else if (ADC < 396) {
                printf("DUTY (in percent): 10  \r \n" );
                OCR0B = 2 * OCR0A / 15; 
            } else if (ADC < 472) {
                printf("DUTY (in percent): 15  \r \n" );
                OCR0B = 3 * OCR0A / 15; 
            } else if (ADC < 548) {
                printf("DUTY (in percent): 20  \r \n" );
                OCR0B = 4 * OCR0A / 15; 
            } else if (ADC < 624) {
                printf("DUTY (in percent): 25  \r \n" );
                OCR0B = 5 * OCR0A / 15; 
            } else if (ADC < 700) {
                printf("DUTY (in percent): 30  \r \n" );
                OCR0B = 6 * OCR0A / 15; 
            } else if (ADC < 776) {
                printf("DUTY (in percent): 35  \r \n" );
                OCR0B = 7 * OCR0A / 15; 
            } else if (ADC < 852) {
                printf("DUTY (in percent): 40  \r \n" );
                OCR0B = 8 * OCR0A / 15; 
            } else if (ADC < 928) {
                printf("DUTY (in percent): 45  \r \n" );
                OCR0B = 9 * OCR0A / 15; 
            } else {               
                printf("DUTY (in percent): 50  \r \n" ); 
                OCR0B = 10 * OCR0A / 15; 
            } 
}


//Helper to send the trigger signal; 
void activate_trig() {
      //if we are in send == 0, then we send a TRIG signal
            if (send) {
                PORTB |= (1 << PB1); 
                _delay_us(10);        // 10 microsecond delay
                PORTB &= ~(1 << PB1);
            } 
}


void toggle_freq() {
    
    if (discrete) {
        
            if (val < 22) {
                OCR0A = 59;  
            } else if (val < 42) {
                OCR0A = 63;  
            } else if (val < 62) {
                OCR0A = 71;  
            }else if (val < 82) {
                OCR0A = 80; 
            }else if (val < 102) {
                OCR0A = 89;
            }else if (val < 122) {
                OCR0A = 95; 
            }else if (val < 142) {
                OCR0A = 106; 
            } else {
                OCR0A = 120; 
            }
    } else {
    //min OCR0A = 59, max = 120
            double numerator = (double)val;
            if (numerator > 162) numerator = 162.0;  //prevent from faulty reads beyond 162 ticks
            double ratio = numerator / 162.0; 
            OCR0A = (int) (ratio * (61.0) + 59); //OCR0A will range from 59 to 120
    }    
}

 //ISR to count the echo duration
    ISR(TIMER1_CAPT_vect) {
        if (TCCR1B & (1 << 6)) {   //rising edge    
        TCCR1B &= ~(1 << 6);
        send = 0;
        TCNT1 = 0 ;
        } else {  //falling edge 
        TCCR1B |= (1 << 6);
        val = TCNT1;
        //printf("READ: %u\r\n", val);
        send = 1;
        }
    }
    
    
  //ISR to toggle discrete mode: 
 ISR(PCINT0_vect) {
    discrete = 1 - discrete;  //0 -> 1 and 1 -> 0 respectivly 
    _delay_ms(500);
}



//Helper to set up everything: 
void Initialize() { 
    config_GPIO(); 
    set_ADC(); 
    set_PWM(); 
    set_EchoTimer();
    sei();
}


int main(void) {
        Initialize();
        uart_init();
        while (1) {
            activate_trig(); 
            toggle_freq();
            toggle_duty(); 
        }
        return 0;
}
