/*
 * EMG_Trolley.cpp
 *
 * Created: 2021/11/29 21:14:35
 * Author : Haozhe Liu & Hao Dong 
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#define BAUD_RATE 9600
#define BAUD_PRESCALER ( (F_CPU / (BAUD_RATE * 16UL)) - 1 )


#include <avr/interrupt.h>
#include "uart.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <util/delay.h>
using namespace std;

char String[25];
unsigned int BLOCK_SIZE = 300;
unsigned int BUFFER_SIZE = 300;
int data[300];
int fpeaks[5];
int control_flag = 0; // button

uint8_t sine_data [91]=
{
	0,
	4,    9,    13,   18,   22,   27,   31,   35,   40,   44,
	49,   53,   57,   62,   66,   70,   75,   79,   83,   87,
	91,   96,   100,  104,  108,  112,  116,  120,  124,  127,
	131,  135,  139,  143,  146,  150,  153,  157,  160,  164,
	167,  171,  174,  177,  180,  183,  186,  189,  192,  195,       //Paste this at top of program
	198,  201,  204,  206,  209,  211,  214,  216,  219,  221,
	223,  225,  227,  229,  231,  233,  235,  236,  238,  240,
	241,  243,  244,  245,  246,  247,  248,  249,  250,  251,
	252,  253,  253,  254,  254,  254,  255,  255,  255,  255
};

//-----------------------------FFT Function----------------------------------------------//
float sine(int i)
{
	int j=i;
	float out;
	while(j<0){j=j+360;}
	while(j>360){j=j-360;}
	if(j>-1   && j<91){out= sine_data[j];}
	else if(j>90  && j<181){out= sine_data[180-j];}
	else if(j>180 && j<271){out= -sine_data[j-180];}
	else if(j>270 && j<361){out= -sine_data[360-j];}
	return (out/255);
}

float cosine(int i)
{
	int j=i;
	float out;
	while(j<0){j=j+360;}
	while(j>360){j=j-360;}
	if(j>-1   && j<91){out= sine_data[90-j];}
	else if(j>90  && j<181){out= -sine_data[j-90];}
	else if(j>180 && j<271){out= -sine_data[270-j];}
	else if(j>270 && j<361){out= sine_data[j-270];}
	return (out/255);
}

int FFT(int in[],int N,float Frequency)
{
/*
Code to perform FFT on arduino,
setup:
paste sine_data [91] at top of program [global variable], paste FFT function at end of program
Term:
1. in[]     : Data array, 
2. N        : Number of sample (recommended sample size 2,4,8,16,32,64,128...)
3. Frequency: sampling frequency required as input (Hz)

If sample size is not in power of 2 it will be clipped to lower side of number. 
i.e, for 150 number of samples, code will consider first 128 sample, remaining sample  will be omitted.
For Arduino nano, FFT of more than 128 sample not possible due to mamory limitation (64 recomended)
For higher Number of sample may arise Mamory related issue,
Code by ABHILASH
Contact: abhilashpatel121@gmail.com 
Documentation:https://www.instructables.com/member/abhilash_patel/instructables/
2/3/2021: change data type of N from float to int for >=256 samples
*/

unsigned int data[13]={1,2,4,8,16,32,64,128,256,512,1024,2048};
int a,c1,f,o,x;
a=N;  
                                 
      for(int i=0;i<12;i++)                 //calculating the levels
         { if(data[i]<=a){o=i;} }
      
int in_ps[data[o]]={};     //input for sequencing
float out_r[data[o]]={};   //real part of transform
float out_im[data[o]]={};  //imaginory part of transform
           
x=0;  
      for(int b=0;b<o;b++)                     // bit reversal
         {
          c1=data[b];
          f=data[o]/(c1+c1);
                for(int j=0;j<c1;j++)
                    { 
                     x=x+1;
                     in_ps[x]=in_ps[j]+f;
                    }
         }

 
      for(int i=0;i<data[o];i++)            // update input array as per bit reverse order
         {
          if(in_ps[i]<a)
          {out_r[i]=in[in_ps[i]];}
          if(in_ps[i]>a)
          {out_r[i]=in[in_ps[i]-a];}      
         }


int i10,i11,n1;
float e,c,s,tr,ti;

    for(int i=0;i<o;i++)                                    //fft
    {
     i10=data[i];              // overall values of sine/cosine  :
     i11=data[o]/data[i+1];    // loop with similar sine cosine:
     e=360/data[i+1];
     e=0-e;
     n1=0;

          for(int j=0;j<i10;j++)
          {
          c=cosine(e*j);
          s=sine(e*j);    
          n1=j;
          
                for(int k=0;k<i11;k++)
                 {
                 tr=c*out_r[i10+n1]-s*out_im[i10+n1];
                 ti=s*out_r[i10+n1]+c*out_im[i10+n1];
          
                 out_r[n1+i10]=out_r[n1]-tr;
                 out_r[n1]=out_r[n1]+tr;
          
                 out_im[n1+i10]=out_im[n1]-ti;
                 out_im[n1]=out_im[n1]+ti;          
          
                 n1=n1+i10+i10;
                  }       
             }
     }

/*
for(int i=0;i<data[o];i++)
{
Serial.print(out_r[i]);
Serial.print("\t");                                     // un comment to print RAW o/p    
Serial.print(out_im[i]); Serial.println("i");      
}
*/


//---> here onward out_r contains amplitude and our_in conntains frequency (Hz)
    for(int i=0;i<data[o-1];i++)               // getting amplitude from compex number
        {
         out_r[i]=sqrt(out_r[i]*out_r[i]+out_im[i]*out_im[i]); // to  increase the speed delete sqrt
         out_im[i]=i*Frequency/N;
         /*
         Serial.print(out_im[i]); Serial.print("Hz");
         Serial.print("\t");                            // un comment to print freuency bin    
         Serial.println(out_r[i]); 
         */    
        }




x=0;       // peak detection
   for(int i=1;i<data[o-1]-1;i++)
      {
      if(out_r[i]>out_r[i-1] && out_r[i]>out_r[i+1]) 
      {in_ps[x]=i;    //in_ps array used for storage of peak number
      x=x+1;}    
      }


s=0;
c=0;
    for(int i=0;i<x;i++)             // re arraange as per magnitude
    {
        for(int j=c;j<x;j++)
        {
            if(out_r[in_ps[i]]<out_r[in_ps[j]]) 
                {s=in_ps[i];
                in_ps[i]=in_ps[j];
                in_ps[j]=s;}
        }
    c=c+1;
    }



    for(int i=0;i<5;i++)     // updating f_peak array (global variable)with descending order
    {
		fpeaks[i]=out_im[in_ps[i]];
    }
}
//---------------------------------------------------------------------------//


void ADC_Initialize()
{
	//clear power reduction for ADC
	PRR &= (1<<PRADC);
	
	//Vref = ADC
	ADMUX |= (1<<REFS0);
	ADMUX &= ~(1<<REFS1);
	//Set the ADC clock: divided by 128
	ADCSRA |= (1<<ADPS0);
	ADCSRA |= (1<<ADPS1);
	ADCSRA |= (1<<ADPS2);
	//select channel 0
	ADMUX &= ~(1<<MUX0);
	ADMUX &= ~(1<<MUX1);
	ADMUX &= ~(1<<MUX2);
	ADMUX &= ~(1<<MUX3);
	//Set to free running
	ADCSRA |= (1<<ADATE);		//auto triggering of adc
	ADCSRB &= ~(1<<ADTS0);		//free running mode ADT[2:0] = 000
	ADCSRB &= ~(1<<ADTS1);
	ADCSRB &= ~(1<<ADTS2);
	//disable digital input buffer
	DIDR0 |= (1<<ADC0D);
	//Enable ADC
	ADCSRA |= (1<<ADEN);
	//Enable ADC interrupt
	ADCSRA |= (1<<ADIE);
	//start conversation
	ADCSRA |= (1<<ADSC);
}

void Timer1_Initialize()
{
	DDRB &= ~(1 << DDB0);
	PORTB |= (1<<PORTB0);

	TCCR1B |= (1<<CS11);
	TCCR1B |= (1<<ICES1);
	
	TIFR1 |= (1<<ICF1);
	
	TIMSK1 |= (1<<ICIE1);
}

void initialize()
{	
	DDRB |= (1 << DDB4);
	DDRB |= (1 << DDB5);
	DDRB |= (1 << DDB1);
	cli();
	ADC_Initialize();
	Timer1_Initialize();
	sei();
}

void SerialSetup(){
	UART_init(BAUD_PRESCALER);
	sprintf(String,"Hello world! \n");
	UART_putstring(String);
	_delay_ms(1000);
	
	sprintf(String,"The year is %u\n", 2021);
	UART_putstring(String);
	_delay_ms(1000);
}

 ISR(ADC_vect)
 {
   	unsigned long ind = 0;
	
	sprintf(String, "%s: \n","waiting for new block");
	UART_putstring(String);
   	while (ind < BLOCK_SIZE && control_flag == 1){
   		
		// 1. read from ADC input
   		data[ind] = ADC;
		sprintf(String, "ADC: %d \n", data[ind]);
    	UART_putstring(String);
		
		// 2. processing
		// y += x[ind];
		
   		// 3. write to PWM output
   		// OCR2A = y[(ind-MIN_DELAY)&(BUFFER_SIZE-1)];

   		// ind &= BUFFER_SIZE - 1;
   		
		if (ind == BLOCK_SIZE -1){
			
			// Frequency Features
			FFT(data,32,200);        //to get top five value of frequencies of X having 32 sample at 200Hz sampling
			for(int j=0;j<5;j++){
				sprintf(String, "peak frequency: %d \n", fpeaks[j]);
				UART_putstring(String);
			}
			
			//average window Time Fearures
			int threshold;
			
			
			// convert the frequency to different control modes
			int data;
			/*if (fpeaks[0] < 70 && fpeaks[2] >= 60) data = 5;*/
			/*if(fpeaks[1] < 90 && fpeaks[1] >= 70) data = 4;*/
			
			if(fpeaks[0] >= 60) data = 3;
			else if(fpeaks[0] < 30 && fpeaks[0] >= 10 && fpeaks[1] > 0 ) data = 1;
			else data = 2;
			
			sprintf(String, "data case: %d \n", data); UART_putstring(String);
			
			if (data != '\0' && data != '\n')
			{
				 switch (data)
				 {
					 // PB5: high PB4：low
					 case 0: break;
					 case 1: PORTB &= ~(1<<PORTB5); PORTB |= (1<<PORTB4); break;
					 case 2: PORTB |= (1<<PORTB5); PORTB &= ~(1<<PORTB4); break;
					 // case 3: PORTB |= (1<<PORTB5); PORTB |= (1<<PORTB4); break;
				 }
			}
 			
 			_delay_ms(1000);
			
		}
		
		// 4. increment read/write buffer index
		ind++;
	}
	
	ind = 0;
	// 5. start new ADC conversion
	ADCSRA |= (1<<ADSC);
 }
 
ISR(TIMER1_CAPT_vect)
{
	// PORTB ^= (1<<PORTB0);
	control_flag = !control_flag;
	if(control_flag == 1){
		PORTB |= (1 << PORTB1);
		sprintf(String, "EMG control mode start: %d \n",control_flag); UART_putstring(String);
	}
	else{
		PORTB &= ~(1 << PORTB1);
		sprintf(String, "EMG control mode finish %d \n", control_flag); UART_putstring(String);
	}
	_delay_ms(1000);
	
}

//------------------------------------------------------------------------------------//

int main(void)
{
    /* Replace with your application code */
	SerialSetup();
	initialize();
    while (1)
    {		
// 		sprintf(String, "ADC: %d \n",ADC);
// 		UART_putstring(String);
    };
}
