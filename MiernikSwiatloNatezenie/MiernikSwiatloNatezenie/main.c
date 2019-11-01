/*
 * MiernikSwiatloNatezenie.c
 *
 * Created: 2019-08-09 11:06:11
 * Author : g580
 */ 

#define F_CPU 1000000
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#define BAUD 9600
#define MYUBRR  F_CPU/BAUD/16-1		//tryb asynchroniczny normalny



void USART_init(unsigned int myubrr)
{
	UBRRH = (unsigned char)(myubrr>>8);
	UBRRL = (unsigned char)myubrr;

	UCSRB = (1<<TXEN);

	UCSRC = (1<<URSEL)|(3<<UCSZ0);
}



static int USART_Transmit(char c, FILE *stream)
{
	while(!(UCSRA & (1<<UDRE)));
	UDR = c;

	return 0;
}


static FILE wyjscie = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE);




unsigned int readADC0()
{
	ADMUX = 0x00;
	ADCSRA = 0x00;
	ADMUX |= (1 << REFS0);
	//ADMUX |= (0 << ADLAR);
	ADMUX |= (0<<MUX4) |(0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (0 << MUX0);
	ADCSRA |=(1<<ADEN)|(1<<ADPS0);
	ADCSRA |= (1 << ADSC);
	return ADC;
}

unsigned int readADC1()
{
	ADMUX = 0x00;
	ADCSRA = 0x00;
	ADMUX |= (1 << REFS0);
	//ADMUX |= (0 << ADLAR);
	ADMUX |= (0<<MUX4) |(0 << MUX3) | (0 << MUX2) | (0 << MUX1) | (1 << MUX0);
	ADCSRA |=(1<<ADEN)|(1<<ADPS0);
	ADCSRA |= (1 << ADSC);
	return ADC;
}

unsigned int readADC2()
{
	ADMUX = 0x00;
	ADCSRA = 0x00;
	ADMUX |= (1 << REFS0);
	//ADMUX |= (0 << ADLAR);
	ADMUX |= (0<<MUX4) |(0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (0 << MUX0);
	ADCSRA |=(1<<ADEN)|(1<<ADPS0);
	ADCSRA |= (1 << ADSC);
	return ADC;
}


unsigned int readADC3()
{
	ADMUX = 0x00;
	ADCSRA = 0x00;
	ADMUX |= (1 << REFS0);
	//ADMUX |= (0 << ADLAR);
	ADMUX |= (0<<MUX4) |(0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (1 << MUX0);
	ADCSRA |=(1<<ADEN)|(1<<ADPS0);
	ADCSRA |= (1 << ADSC);
	return ADC;
}

int wypisz(float napiecie)
{
	int lux = ((1650/napiecie) - 500)/10;
	char LDRSHOW [7];
	float rezystancja = (napiecie*10/(3.3-napiecie));
	dtostrf(rezystancja, 4, 1, LDRSHOW);
	printf(LDRSHOW);
	printf(" KOhm    ");
	dtostrf(napiecie, 6, 3, LDRSHOW);
	printf(LDRSHOW);
	printf(" Volts    ");
	dtostrf(lux, 6,0, LDRSHOW);
	printf(LDRSHOW);
	printf(" Lux");
	printf("\r\n");
	return lux;
}

 int main(void)

 {

	USART_init(MYUBRR);

	stdout = &wyjscie;
	
	 _delay_ms(50);

	 DDRA = 0;

	 float LDR= 0;

	 _delay_ms(50);

	int counter = 0;
	int max = 0;
	unsigned int all = 0;
	int now_check = 0;
	float VoltADC =0;

	 while(1)

	 {
		 VoltADC = readADC0()/310.3;
		printf("ADC0: ");
		now_check = wypisz(VoltADC);
		max = now_check; 
		 
		 _delay_ms(50);
		 
		VoltADC = readADC1()/310.3;
		printf("ADC1: ");
		now_check = wypisz(VoltADC);
		if(max < now_check)
		{
			max = now_check;
		}
		_delay_ms(50);
		
		VoltADC = readADC2()/310.3;
		printf("ADC2: ");
		now_check = wypisz(VoltADC);
		if(max < now_check)
		{
			max = now_check;
		}
		_delay_ms(50);
		
		VoltADC = readADC3()/310.3;
		printf("ADC3: ");
		now_check = wypisz(VoltADC);
		if(max < now_check)
		{
			max = now_check;
		}
		_delay_ms(50);
		
		printf("Najwieksza wartosc: ");
		printf("%d", max);
		
		all += max;
		counter++;
		max = 0;
		
		if(counter == 10)
		{
			
			
			printf("Srednia z pomiarow: ");
			printf("%d", all/10);
			counter = 0;
			all = 0;
			
			 _delay_ms(2050);
		}
		printf("\r\n");
		printf("\r\n");

		 _delay_ms(350);


	 }

	 

 }

