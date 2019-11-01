/*
 * main.c
 *
 * Created: 2019-06-29 16:12:25
 * Author : g580
 */ 

#define F_CPU 1000000ul	
#include <stdio.h>			
#include <avr/io.h>			
#include <util/delay.h>		
#include <stdlib.h>

#define BAUD 2400
#define MYUBRR  F_CPU/BAUD/16-1		//tryb asynchroniczny normalny

#define ACK 1
#define NOACK 0

#define ds3231_adr 0b11010000		//adresy bazowe dla DS3231 RTC
#define ds3231_REG_TIME             0x00
#define ds3231_REG_ALARM_1          0x07
#define ds3231_REG_ALARM_2          0x0B
#define ds3231_REG_CONTROL          0x0E
#define ds3231_REG_STATUS           0x0F
#define ds3231_REG_TEMPERATURE		0x11

void twi_clock_setting(uint16_t clk_KHz); //funkcja do ustawienia czêstotliwoœci sygna³u zegarowego (SCL)
void twistart(void);				//funkcja START
void twistop(void);					//funkcja STOP
void twiwrite(char data);			//funkcja DATA
char twiread(char ack);				//funkcja DATA

void twiwrite_buf( uint8_t SLA, uint8_t adr, uint8_t len, uint8_t *buf );	//funkcja do zapisu buforu
void twiread_buf(uint8_t SLA, uint8_t adr, uint8_t len, uint8_t *buf);		//funkcja do czytania buforu

uint8_t dec2bcd(uint8_t d);			//DEC to BCD
uint8_t bcd2dec(uint8_t b);			//BCD to DEC

void twiread_ds3231(void);				//funkcja do czytania z ds3231 RTC
void twiwrite_ds3231(uint8_t register_adr, uint8_t data);			//funkcja do zapisywania w ds3231 RTC
void write_control_register(uint8_t control);		//funkcja do ustawiania rejestru kontrolnego (address=0x0E)

char buffer[16];					//bufor dla wyœwietlacza LCD

uint8_t wr_ds3231_buf[19], rd_ds3231_buf[19];
uint8_t seconds=0, minutes=0, hours=0, years=0;
uint8_t temp0=0, temp1=0, temp2=0;


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

int check = 0;

int main(void)
{
	USART_init(MYUBRR);		//inicjalizacja

	/* Przekierowuje standardowe wyjœcie do  'wyjscie' */
	stdout = &wyjscie;
	
	_delay_ms(5000);	
	
	DDRC =0;							
	PORTC|= (1 << 1)|(1 << 0);			
	twi_clock_setting(100);				//inicjalizacja sygna³u SCL dla I2C, 100kHz
	_delay_ms(150);						
	twiwrite_ds3231(ds3231_REG_TIME, 0x00);		
	write_control_register(0b00000000);	//wyjœciowa czêstotliwoœæ 1Hz - pin INT/SQW
	
	while(1)
	{
		_delay_ms(200);						
		twiread_buf(ds3231_adr,ds3231_REG_TIME,7,rd_ds3231_buf);
		seconds = bcd2dec(rd_ds3231_buf[0]);
		minutes = bcd2dec(rd_ds3231_buf[1]);
		hours   = bcd2dec(rd_ds3231_buf[2]);

		if(check != seconds)
		{
			sprintf(buffer,"%d:%d:%d  ",hours, minutes, seconds);
			printf("HH:MM:SS: ");
			printf(buffer);
			printf("\r");
			check = seconds;
		}
		
	}

}

void twi_clock_setting(uint16_t clk_KHz) 
//ustawienie czêstotliwoœci sygna³u zegarowego - SCL -> min. 64kHz max 500kHz
{
	uint8_t clk_div;
	clk_div = ((F_CPU/1000l)/clk_KHz);
	if(clk_div >= 16)
	clk_div = (clk_div-16)/2;
	TWBR = clk_div;
}

void twistart(void)
// start transmisji
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));	//czekaj na koniec transmisji
}

void twistop(void)
//koniec transmisji
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
	while ((TWCR & (1<<TWSTO)));	//czekaj na koniec transmisji
}

void twiwrite(char data)
// write data
{
	TWDR = data;					//za³aduj dane do rejestru
	TWCR = (1<<TWINT) | (1<<TWEN);	//zacznij transmisje
	while (!(TWCR & (1<<TWINT)));	//czekaj na jej koniec
}

char twiread(char ack)
// czytaj dane
{
	TWCR = ack
//uruchom modu³ TWI i potwierdŸ dane po odbiorze
	? ((1 << TWINT) | (1 << TWEN) | (1 << TWEA))
	: ((1 << TWINT) | (1 << TWEN)) ;
	while (!(TWCR & (1<<TWINT)));	//czekaj na koniec
	return TWDR;
}

void twiread_ds3231(void)
// funkcja do czytania danych z ds3231 RTC
{
	twistart();						//START
	twiwrite(ds3231_adr);			//pisz adres ds3231 fdla operacji czytania
	twiwrite(ds3231_REG_TIME);		//adres bazowy rejestrów wewnetrznych
	twistart();						//START
	twiwrite(ds3231_adr|0x01);		//pisz adres + 1 ds3231 dla operacji czytania
	rd_ds3231_buf[0] = twiread(ACK);			//czytaj dane z ACK
	rd_ds3231_buf[1] = twiread(ACK);			//czytaj dane z ACK
	rd_ds3231_buf[2] = twiread(ACK);			//czytaj dane z ACK
	rd_ds3231_buf[3] = twiread(ACK);			//czytaj dane z ACK
	rd_ds3231_buf[4] = twiread(ACK);			//czytaj dane z ACK
	rd_ds3231_buf[5] = twiread(ACK);			//czytaj dane z ACK
	rd_ds3231_buf[6] = twiread(NOACK);			//czytaj dane z ACK
	twistop();						//STOP
}

void twiwrite_ds3231(uint8_t register_adr, uint8_t data)
// funkcja do pisania danych do ds3231 RTC
{
	twistart();					
	twiwrite(ds3231_adr);		
	twiwrite(register_adr);		
	twiwrite(data);
	twistop();					
}


void write_control_register(uint8_t control)		
//ustawienie rejestru kontrolnego(adres=0x0E)
{
 twistart();					
 twiwrite(ds3231_adr);			
 twiwrite(ds3231_REG_CONTROL);	
 twiwrite(control);
 twistop();						
}

void twiwrite_buf( uint8_t SLA, uint8_t adr, uint8_t len, uint8_t *buf ) 
/* pisz bufor do slave'a 
	SLA  - adres slave'a 
	adr  - adres bazowy
	len  - iloœæ bajtów do odczytania
	*buf - nazwa bufora
*/
{
	twistart();
	twiwrite(SLA);
	twiwrite(adr);
	while (len--) twiwrite(*buf++);
	twistop();
}

void twiread_buf(uint8_t SLA, uint8_t adr, uint8_t len, uint8_t *buf) 
{
	twistart();
	twiwrite(SLA);
	twiwrite(adr);
	twistart();
	twiwrite(SLA + 1);
	while (len--) *buf++ = twiread( len ? ACK : NOACK );
	twistop();
	
	/* czytaj bufor ze slave'a 
	SLA  - adres slave'a 
	adr  - adres bazowy
	len  - iloœæ bajtów do odczytania
	*buf - nazwa bufora
*/
}

uint8_t dec2bcd(uint8_t d)
{
	return ((d/10 * 16) + (d % 10));
}

uint8_t bcd2dec(uint8_t b)
{
	return ((b/16 * 10) + (b % 16));
}


