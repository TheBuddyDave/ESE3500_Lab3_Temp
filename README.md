# ESE3500_Lab3_Temp

# Timer Overflow 
## (R1) What frequency is being generated here? Is it what you expected? Show your work.
TODO 

## (I1)	Attach an image from the oscilloscope showing both waveforms.
![alt text](Image_I1.png)


# Normal Mode
## (R2)	Did you have to prescale the system clock and/or timer clock? If so, by how much?
We did not have to prescale the system clock, but we prescaled the timer clock by 256.

## (R3) What number should OCR0A be in this case? 
We set OCR0A = 70, derived from the formula count = 16MHz/(2*256*440Hz) - 1 = 70.

## (S1)	Attach a screenshot of your code snippet or copy & paste the snippet into a box in the submission document. It should be quite short.

''

    //LAB 3 - PART 2 
    void Initialize() {
        //configrue output LED to PD6
        DDRD |= (1 << PD6);
        DDRB |= (1 << LED_PIN); 

        //set the prescale to 255 
        TCCR0B |= (1 << 2); 
        TCCR0B &= ~(1 << 0);
        TCCR0B &= ~(1 << 1); 
        
        TIMSK0 |= (1 << OCIE0A); 
        
        //SET timer control A to normal mode
        TCCR0A &= ~(1 << 0); 
        TCCR0A &= ~(1 << 1); 
        TCCR0A &= ~(1 << 2); 

        //set the OCR0A Value: 
        OCR0A = 70; 
        //Set the interrupt: 
        TIFR0 |= (1 << OCF0A); 

        sei(); 
        uart_init(); 
    }

    ISR(TIMER0_COMPA_vect){
        PORTD ^= (1 << PORTD6);
        TCNT0 = 0; 
        OCR0A = 70; 
    }

    int main(void) {
        Initialize(); 
        while (1) {
        }
        return 0; 
    }

''

## (I2)	Attach an image from the oscilloscope showing both waveforms.
![alt text](Image_I2.png)


# CTC Mode

## (R4)	What number should OCR0A be in this case? 
OCR0A should still be 70 (same as Part 2) 
## (S2)	Attach a screenshot of your code snippet.
''


    void Initialize() {
        
        // set PD6 (OC0A) as output
        DDRD |= (1 << PD6);

        //set the CTC mode
        TCCR0A = (1 << 1); 

        //toggle OCRA on compare match
        TCCR0A |= (1 << COM0A0); 

        
        OCR0A = 70; 

        //prescaler
        TCCR0B |= (1 << CS02); 

        sei(); 
    }

    int main(void) {
        Initialize(); 
        while (1) {
        }
        return 0; 
    } 


''

## (I3)	Attach an image from the oscilloscope showing both waveforms.
![alt text](Image_I3.png)



# PWM Mode
# (R5)	What number should OCR0A be in this case? Specify which Phase Correct mode you use - namely, what is the TOP value? (Refer to Table 18-9 in the datasheet). 
OCR0A should be 35 this time, since we compare both when we count up and when we count down. Twice the compare means 1/2 the count. 

# (S3)	Attach a screenshot of your code snippet.

''

    //LAB 3 - Part  - Phase Correct
    void Initialize() {

        // set PD6 (OC0A) AND PD5 (OC0B) as outputs
        DDRD |= (1 << PD6) | (1 << PD5);

        // phase Correct PWM (mode 5 in the data sheet) (TOP = OCR0A)
        TCCR0A = (1 << WGM00); 
        TCCR0B = (1 << WGM02);

        // configure compare match output mode: 
        // OC0A: toggle on match
        // OC0B: Non-inverting PWM 
        TCCR0A |= (1 << COM0A0) | (1 << COM0B1);

        // set frequency and duty cycle
        OCR0A = 35;                // Freq (top = 35)
        OCR0B = OCR0A / 2;         // duty cycle == 0.5

        // prescale by 256 (which starts the timer)
        TCCR0B |= (1 << CS02); 

        sei(); 
    }

    int main(void) {
        Initialize(); 
        while (1) {
        }
        return 0; 
    } 


''


# (I4)	Attach an image from the oscilloscope showing both waveforms.![alt text](Image_I4.png)