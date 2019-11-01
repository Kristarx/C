/*
 * temperaturaidodatki.c
 *
 * Created: 2018-12-16 13:29:12
 * Author : g580
 */ 

#define F_CPU 1000000ul
#define BAUD 2400
#define MYUBRR  F_CPU/BAUD/16-1		//tryb asynchroniczny normalny
#include <avr/io.h>
#include <stdio.h>
#include "hd44780.h"
#include <util/delay.h>

/*definicja zmiennych programu */

uint8_t licznik_rozkazow = 0, wartosc = 1, klawisz = 1;
uint8_t zwracana_wartosc(void);
uint8_t najnizsza(int, int);
uint8_t najwyzsza(int, int);
uint8_t ktore_ledy(int);
uint8_t temp11 = 200, temp12 = 2000, temp21 = 0, temp22 = 0;
int tem1, tem2;
char str[17];
char buffer[16];
int counter = 0;
uint8_t srodkowy = 0, stan_buzzera = 0;


/* Inicjalizacja portu szeregowego uC ATMEGA16 */

void USART_init(unsigned int myubrr)
{
    /* Ustalenie prêdkoœæ transmisji */
    UBRRH = (unsigned char)(myubrr>>8);
    UBRRL = (unsigned char)myubrr;

    /* W³¹czenie nadajnika */
    UCSRB = (1<<TXEN);
  
    /* Format ramki: 8 bitów danych, 1 bit stopu, brak bitu parzystoœci */
    UCSRC = (1<<URSEL)|(3<<UCSZ0); 
}


/* Wys³anie znaku do portu szeregowego */
static int USART_Transmit(char c, FILE *stream)
{
    while(!(UCSRA & (1<<UDRE)));
    UDR = c;

    return 0;
}


/* Tworzenie strumienia danych o nazwie 'wyjscie' który po³¹czony jest
    z funkcj¹ 'USART_Transmit' */
static FILE wyjscie = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE);



int main(void)
{
	DDRA = 0xFF;			//LED
	DDRB = 0x00;
	PORTB = 0b00000111;		//SWITCH
	DDRC = 0x01;			//BUZZER
	double temp;
	unsigned char ds18b20_pad[9];
	
	USART_init(MYUBRR);		//inicjalizacja

	/* Przekierowuje standardowe wyjœcie do  'wyjscie' */
	stdout = &wyjscie;
	
	lcd_init();
	lcd_home();
	lcd_clrscr();
	
	while(1)
	{		
		/*Pomiar temperatury z u¿yciem czujnika DALLAS DS18B20, tuta ustalane s¹ wartoœci przed przecinkiem
		  jak i po przecinku*/
					
		if(ds18b20_ConvertT())
		{
			_delay_ms(100);
			ds18b20_Read(ds18b20_pad);
			temp = ((ds18b20_pad[1] << 8) + ds18b20_pad[0]) / 16.0 ;
			tem1 = temp;
			tem2 = (temp - tem1)*10;
		}
		
		/*Sprawdzamy czy któryœ z przcisków wcisnêliœmy. Jesli tak to odswie¿amy wartosc ktora mówi nam, która 
		  czêœæ bêdzie wyœwietlana na wyœwietlaczu */
		
		if((!(PINB & 0x01)) || (!(PINB & 0x02)) || (!(PINB & 0x04)))
		{
			klawisz = zwracana_wartosc();
			if(klawisz == 1)
			{
				if(wartosc == 4)
					wartosc = 1;
				else
					wartosc++;
				_delay_ms(200);
			}
				
			if(klawisz == 3)
			{
				if(wartosc == 1)
					wartosc = 4;
				else
					wartosc--;
				_delay_ms(200);
			}
		}
		
		/*Pêtla switch...case mówi¹ca co bêdzie wyœwietlane na wyœwietlaczu hd44780 */
			
		switch(wartosc)
		{
			case 1:
			sprintf(str,"%d.%d", tem1, tem2);
			if(counter == 5)
			{
				//printf("Temperatura wynosi: %s*C\n\r", str);
				printf("%s\r", str);
				counter = 0;
			}
			counter++;
			lcd_home();
			lcd_clrscr();
			lcd_puts("   Termometr    ");
			lcd_goto(0x40);
			lcd_puts("      ");
			lcd_puts(str);
			break;
			
			case 2:
			sprintf(str,"%d.%d", tem1, tem2);
			if(counter == 5)
			{
				//printf("Temperatura wynosi: %s*C\n\r", str);
				printf("%s\r", str);
				counter = 0;
			}
			counter++;
			lcd_clrscr();
			srodkowy = 0;
			if(srodkowy == 0)
			{
				klawisz = zwracana_wartosc();
				
				if(klawisz == 1 || klawisz == 3)
				srodkowy = 1;
				if(klawisz == 2)
				{
					if(stan_buzzera == 1)
					stan_buzzera -= 1;
					else
					stan_buzzera += 1;
					_delay_ms(200);
				}
				
				
				if(stan_buzzera == 1)
				{
					lcd_clrscr();
					lcd_home();
					lcd_puts("BUZZER");
					lcd_goto(0x40);
					lcd_puts("ON");
				}
				
				else
				{
					lcd_clrscr();
					lcd_home();
					lcd_puts("BUZZER");
					lcd_goto(0x40);
					lcd_puts("OFF");
				}
	
			}
			break;
			
			case 3:
			najwyzsza(tem1, tem2);
			sprintf(str,"%d.%d", temp21, temp22);
			if(counter == 5)
			{
				//printf("Temperatura wynosi: %s*C\n\r", str);
				printf("%s\r", str);
				counter = 0;
			}
			counter++;
			lcd_clrscr();
			lcd_home();
			lcd_puts("Najwyzsza temp:");
			lcd_goto(0x40);
			lcd_puts("        ");
			lcd_puts(str);
			break;
			
			case 4:
			najnizsza(tem1, tem2);
			sprintf(str,"%d.%d", temp11, temp12);
			if(counter == 5)
			{
				//printf("Temperatura wynosi: %s*C\n\r", str);
				printf("%s\r", str);
				counter = 0;
			}
			counter++;
			lcd_clrscr();
			lcd_home();
			lcd_puts("Najnizsza temp:");
			lcd_goto(0x40);
			lcd_puts("        ");
			lcd_puts(str);
			break;
		}
		
		/*Sprawdzenie czy buzzer powinien byæ w³¹czony */
		
		if((stan_buzzera == 1) && (tem1 >= 29 || tem1 <=11))
			PORTC |= _BV(0);	
		else
			PORTC &= ~_BV(0);
		
		/*Tutaj sprawdzane jest, jakie LED-y powinny byæ w³¹czone w zale¿oœci od tego jaka jest obecnie temperatura, istnieje specjalna do tego funkcja ktore_ledy */
		
		int LEDY;
		LEDY = ktore_ledy(temp);
		PORTA = LEDY;		
    }
}

/*Ta funkcja zwraca wartosc, w zale¿noœci od tego jaki przycisk i o ile zosta³ on wgl wciœniêty */

uint8_t zwracana_wartosc(void)
{
	uint8_t klaw = 0;
	klaw = PINB;
	if((klaw & 0b00000001) == 0)
	{
		return 1;
	}
	
	if((klaw & 0b00000010) == 0)
	{
		return 2;
	}
	
	if((klaw & 0b00000100) == 0)
	{
		return 3;
	}
	
	return 4;
	
}

/*Funkcja zwracaj¹ca wartoœæ binarn¹ dziêki której mo¿na ustawiæ zapalenie odpowiednich
  diod LED w zale¿noœci od obecnej wartoœci temperatury */

uint8_t ktore_ledy(int tem1)
{
	if((tem1 < 22) && (tem1 > 20))
		return 0b00011000;
		
	else if((tem1 == 19) || (tem1 == 20))
		return 0b00010000;
	
	else if((tem1 == 22) || (tem1 == 23))
		return 0b00001000;
		
	else if(tem1 < 19)
	{
		int zmienna = 0b00010000;
		int dzielenie = 0;
		dzielenie = (19 - tem1)/2;
		if(dzielenie > 3)
			dzielenie = 3;
			
		for(int i = 0; i < dzielenie; i++)
		zmienna |= (1<<(i+5));
		
		return zmienna;
		
	}
	
	else if(tem1 > 23)
	{
		int zmienna = 0b00001000;
		int dzielenie = 0;
		dzielenie = (tem1 - 21)/2;
		if(dzielenie > 3)
		dzielenie = 3;
		
		for(int i = 0; i < dzielenie; i++)
		zmienna |= (1<<(2-i));
	
		return zmienna;
	}
	
	else
	return 0;
}

/*Funkcja zwracaj¹ca najni¿sz¹ temperaturê podczas dzia³ania programu */

uint8_t najnizsza(int tem1, int tem2)
{
	if(temp11 > tem1)
	{
		temp11 = tem1;
		temp12 = tem2;
	}
	
	else if(temp11 == tem1)
	{
		temp11 = tem1;
		if(temp12 > tem2)
			temp12 = tem2;
	}
	
	return temp11, temp12;
}

/*Funkcja zwracaj¹ca najwy¿sz¹ temperaturê podczas dzia³ania programu */

uint8_t najwyzsza(int tem1, int tem2)
{
	if(temp21 < tem1)
	{
		temp21 = tem1;
		temp22 = tem2;
	}
	
	else if(temp21 == tem1)
	{
		temp21 = tem1;
		if(temp22 < tem2)
		temp22 = tem2;
	}
	
	return temp21, temp22;
}