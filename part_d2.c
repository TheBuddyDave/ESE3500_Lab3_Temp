#define F_CPU 16000000UL 
#include <avr/io.h>
#include <util/delay.h> 
#include <avr/interrupt.h>
#include "uart.h"
#define LED_PIN PB5  

void Initialize() {    
        //Configure da pins ty shi
        DDRB |= (1 << LED_PIN);
        DDRD |= (1 << PD6);
        DDRD |= (1 << PD5);
        DDRB &= ~(1 << PB0);

        
        
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

        
        
        // ---- SET UP PWM TIMER (TIMER0 cuz thats the instructions ) ----- 
        //set the CTC mode
        TCCR0A = (1 << 1); 
        //toggle OCRA on compare match
        TCCR0A |= (1 << COM0A0); 
        //initial OCRA0
        OCR0A = 59; 
        //Prescale to 64 
        TCCR0B |= (1 << CS00); 
        TCCR0B |= (1 << CS01); 
        // ---------------------------------- 
        sei();
    }

    //state management (send = 1 -> start read , send = 0 -> waiting for read result) 
    static volatile int send = 1;
    
    //Tick counter for echo
    static volatile uint16_t val = 0;
    
    //ISR to count the echo duration
    ISR(TIMER1_CAPT_vect) {
        if (TCCR1B & (1 << 6)) {   //rising edge    
        TCCR1B &= ~(1 << 6);
        send = 0;
        TCNT1 = 0 ;
        } else {  //falling edge 
        TCCR1B |= (1 << 6);
        val = TCNT1;
        printf("READ: %u\r\n", val);
        send = 1;
        }
    }

    int main(void) {
        Initialize();
        uart_init();
        
        while (1) {      
            
            //if we are in send == 0, then we send a TRIG signal
            if (send) {
                PORTD |= (1 << PD5);  
                _delay_us(10);        // 10 microsecond delay
                PORTD &= ~(1 << PD5);
            } 
            
            
            
            //we will split the 2-162 range into 8 equal parts
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
        }
        
        return 0;
    }
