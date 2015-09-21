/*****************************************************
This program was produced by the
CodeWizardAVR V1.25.7a Evaluation
Automatic Program Generator
© Copyright 1998-2007 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : mobile_robot
Version : 1.1
Date    : 1.2.2008
Author  : Michal Vokac
Company : ctu fel
Comments: 


Chip type           : ATmega8
Program type        : Application
Clock frequency     : 16,000000 MHz
Memory model        : Small
External SRAM size  : 0
Data Stack size     : 256
*****************************************************/
#include <math.h>
#include <mega8.h>
#include <delay.h>
#include <stdlib.h>

// Alphanumeric LCD Module functions
#asm
   .equ __lcd_port=0x12 ;PORTD
#endasm
#include <lcd.h> 

#define ADC_VREF_TYPE 0xE0


unsigned char string[3];                        // vypoctena hodnota napeti
unsigned int napeti;				// obsah AD prevodniku

//-------------------------------------------------------------------------------------------------
// ADC interrupt service routine
interrupt [ADC_INT] void adc_isr(void){	

	napeti= (unsigned int)(ADCH)*4;		// vypocet napeti, 8bit presnost
	itoa(napeti,string);				// konverze namer. hodnoty int na retezec, ulozi hodnotu do promenne string
}

//--------------------------------------------------------------------------------------------------
/* funkce pro provedeni zakladnich nastaveni a inicializaci */ 
void init(void){
	
	lcd_init(16);
	lcd_gotoxy(0,0);
	lcd_putsf("Ahoj,jsem ZANDAR");
	delay_ms(500);
	lcd_gotoxy(0,1);
	lcd_putsf("Mobilni robot");
	delay_ms(2000);
	lcd_clear();
	lcd_putsf("Inicializace...");
	lcd_gotoxy(0,1);
	delay_ms(500);
	lcd_putsf("Spoustim program");
	delay_ms(1000);
 
}

//-----------------------------------------------------------------------------------
/* funkce pro sledovani stavu mikrospinacu */
unsigned int ch_switch(void){
	if(PINC.2 != 1)return 1;	// vraci 1 pokud narazi na prekazku, sepne kontakt
        return 0;                       // jinak vraci 0
}

//-----------------------------------------------------------------------------------
/* funkce pro detekci prekazky IR signalem */
unsigned int ch_infra(void){
	unsigned int i;
	for(i = 5;i>0;i--){	// vysila detekcni signal
          PORTB.4 = 0;
          PORTB.3 = 0;
	  delay_us(14);
	  PORTB.4 = 1;
	  PORTB.3 = 1;
	  delay_us(14);
	  }
	  if(PINC.4 != 1)return 1;	// kontrola signalu vlevo, pokud je detekce vraci 1
	  if(PINC.3 != 1)return 2;      // kontrola signalu vpravo, pokud je detekce vraci 2
	  return 0;			// pokud nedetekuje vraci 0
} 

//-----------------------------------------------------------------------------------
/* funkce pro jizdu vpred */
unsigned int go_forward(void){
  for(;;){
	if(ch_switch())return 3;	// kontroluje naraz do prekazky, pokud narazil vraci 3
	switch(ch_infra()){             // kontrola infra signalu
	case 1:
	 return 1;			// pokud detekuje signal vlevo, vraci 1
	 break;
	case 2:
	 return 2;			// pokud detekuje signal vpravo, vraci 2
	 break;                         // jinak jede dal vpred
	 }
	PORTC.0 = 1;
	PORTC.1 = 1;
	delay_us(1300);
	PORTC.0 = 0;
	delay_us(700);
	PORTC.1 = 0;
	delay_ms(18);
	}                    
}
//-----------------------------------------------------------------------------------
/* funkce pro jizdu vzad */
void go_back(void){
  unsigned int i = 25;
  for(;i>0;i--){
	PORTC.0 = 1;
	PORTC.1 = 1;
	delay_us(1000);
	PORTC.1 = 0;
	delay_us(800);
	PORTC.0 = 0;
	delay_ms(18);
	}  
}
//-----------------------------------------------------------------------------------
/* funkce pro zataceni vlevo */
void turn_l(unsigned int i){	// otaci se o i kroku vlevo
  	for(;i>0; i--){	
  	  PORTC.0 = 1;
	  PORTC.1 = 1;
	  delay_ms(2);
	  PORTC.0 = 0;
	  PORTC.1 = 0;
	  delay_ms(18);
	  }
}
//------------------------------------------------------------------------------------
/* funkce pro zataceni vpravo */
void turn_r(unsigned int i){	// otaci se o i kroku vpravo
  	for(;i>0; i--){	
  	  PORTC.0 = 1;
	  PORTC.1 = 1;
	  delay_ms(1);
	  PORTC.1 = 0;
	  PORTC.0 = 0;
	  delay_ms(19);
	  }
}


/********************************************************/
/* funkce main */
/********************************************************/
void main(void){

// Port B initialization , LED indikace
// IR TxL	PORTB.4
// IR TxR	PORTB.3 
DDRB=0xFF;
PORTB=0x00;

// Port C initialization
// leve servo	PORTC.0
// prave servo	PORTC.1
// mikrospinace	PORTC.2
// IR RxR	PORTC.3
// IR RxL       PORTC.4
// ADC input	PORTC.7
DDRC= 0b00000011;
PORTC=0b11111100;
// Port D initialization , LCD module
DDRD=0xFF;
PORTD=0x00;

// External Interrupt(s) initialization 
MCUCR=0x00;

// ADC initialization
// ADC Clock frequency: 1000,000 kHz
// ADC Voltage Reference: AVCC pin
// Only the 8 most significant bits
ADMUX= 7 | (ADC_VREF_TYPE & 0xff);		// vstup AD na PORTD.7
ADCSRA=0xCB;

// Global enable interrupts
#asm("sei")

//initialization
init();

//------------------------------------------
/* hlavni smycka */

lcd_clear();
lcd_putsf("Cekam na START!");


while(ch_switch()){
  ADCSRA|=0x40;	// Start the AD conversion
  lcd_gotoxy(12,1);
  lcd_puts(string);
  lcd_gotoxy(13,1);
  lcd_puts(string);
  lcd_gotoxy(13,1);
  lcd_putsf(".");
  delay_ms(100);
	};
				// kontrola stisku startovaciho tlacitka
lcd_gotoxy(0,0);
lcd_putsf("Program spusten!");

for(;;){

  switch(go_forward()){
    case 1:
     delay_ms(200);
     turn_r(5);   
     delay_ms(200);
     break;
     
    case 2:
     delay_ms(200);
     turn_l(5); 
     delay_ms(200);
     break;

    case 3:
     delay_ms(200);
     go_back();
     delay_ms(200);
     if(rand()<16400)turn_l(20);
     else turn_r(20);
     break;
    }	   
  ADCSRA|=0x40;	// Start the AD conversion
  lcd_gotoxy(12,1);
  lcd_puts(string);
  lcd_gotoxy(13,1);
  lcd_puts(string);
  lcd_gotoxy(13,1);
  lcd_putsf(".");
   }

  }
  
    	