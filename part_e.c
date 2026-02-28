#define F_CPU 16000000UL 
#include <avr/io.h>
#include <util/delay.h> 
#include <avr/interrupt.h>
#include "uart.h"
#define LED_PIN PB5  


void set_ADC() {
//Clear power reduction register bit 
        PRR0 &= ~(1 << PRADC); 

        //Select the Vref to AVcc 
        ADMUX |= (1 << REFS0);
        ADMUX &= ~(1 << REFS1);

        //Set the ADC clock div by 128 
        ADCSRA |= (1 << ADPS0);
        ADCSRA |= (1 << ADPS1); 
        ADCSRA |= (1 << ADPS2); 

        //Select PC0 as channel pin
        ADMUX &= ~(1 << MUX0); 
        ADMUX &= ~(1 << MUX1); 
        ADMUX &= ~(1 << MUX2); 
        ADMUX &= ~(1 << MUX3); 

        //Auto triggering for the ADC
        ADCSRA |= (1 << ADATE); 

        //Free runnning mode
        ADCSRB &= ~(1 << ADTS0); 
        ADCSRB &= ~(1 << ADTS1); 
        ADCSRB &= ~(1 << ADTS2); 

        //Disable Digital Input buffer on ADC pin 
        DIDR0 |= (1 << ADC0D); 

        //Enable ADC:
        ADCSRA |= (1 << ADEN); 

        //Start Conversion: 
        ADCSRA |= (1 << ADSC); 
}

//Lab 3 Part E1 
void Initialize() {
        //Configure da pins ty shi
        DDRB |= (1 << LED_PIN);
        DDRC &= ~(1 << PC0); 
        
        set_ADC(); 
        
        
        sei();
    }

    int main(void) {
        Initialize();
        uart_init();

        while (1) {
            _delay_ms(1000); 
            printf("ADC: %u \r \n", ADC); 

            if (ADC < 320) {
                printf("DUTY (in percent): 5 \r \n" );
            } else if (ADC < 396) {
                printf("DUTY (in percent): 10  \r \n" );
            } else if (ADC < 472) {
                printf("DUTY (in percent): 15  \r \n" );
            } else if (ADC < 548) {
                printf("DUTY (in percent): 20  \r \n" );
            } else if (ADC < 624) {
                printf("DUTY (in percent): 25  \r \n" );
            } else if (ADC < 700) {
                printf("DUTY (in percent): 30  \r \n" );
            } else if (ADC < 776) {
                printf("DUTY (in percent): 35  \r \n" );
            } else if (ADC < 852) {
                printf("DUTY (in percent): 40  \r \n" );
            } else if (ADC < 928) {
                printf("DUTY (in percent): 45  \r \n" );
            } else {               
                printf("DUTY (in percent): 50  \r \n" ); 
            } 
        }
        return 0;
    }

