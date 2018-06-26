/*
 * ad7715_05.c
 *
 * Created: 10.10.2015 3:59:31
 *  Author: User
 */ 
#define F_CPU 8000000UL

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include "LCD.h"

#define DRDY bit_is_clear(PINB,1)
#define Chip_select PORTB2
#define Reset PORTB6
#define VREF 2400 // 2.5 volt
#define VREF_STEP 0.00003662109375 // 2500/2^16



union Data
{
	uint16_t W;
	uint8_t Byte[2];
}Result;


void ad7715_init();
uint16_t Read_AD7715_register(void);
void Write_to_AD7715_register(uint16_t Byteword);
uint8_t Read_byte_SPI(char Addr);

void ad7715_init()
{
	PORTB |= (1<<Chip_select);

	Write_to_AD7715_register(0x10); /* Пишем в коммуникационный регистр: установить коэф. усиления 1; выйти из спящего режима и ожидать записи в регистр настроек*/ 

	Write_to_AD7715_register(0x66); /*Пишем в регистр настроек: выполнить самокалибровку (MD1=0,MD0=1); частота кварца 2.4576 МГц; частота выдачи результата 50 Гц; днополярный режим работы; буфер включен; FSYNC фильтр выключен*/
	
	while(!DRDY);
}

void Write_to_AD7715_register(uint16_t Byteword)
{

	PORTB &= ~(1<<Chip_select);
	
	SPDR = Byteword; 
	while(!(SPSR & (1<<SPIF)));

	PORTB |= 1<<Chip_select;
}

uint16_t Read_AD7715_register(void)
{
	while(!DRDY);

	PORTB &= ~(1<<Chip_select);

	Result.Byte[1] = Read_byte_SPI(0xff);
	Result.Byte[0] = Read_byte_SPI(0xff);

	PORTB|=1<<Chip_select;

	return Result.W;
}

uint8_t Read_byte_SPI(char Addr)
{
	SPDR = Addr;
	while(!(SPSR & (1<<SPIF)));
	Addr = SPDR;
	return Addr;
}

int main(void)
{
uint16_t ADC_result = 0;
uint8_t LCD_buffer[32];

DDRB=(1<<DDRB7) | (1<<DDRB6) | (1<<DDRB5) | (1<<DDRB4) | (1<<DDRB3) | (1<<DDRB2) | (0<<DDRB1) | (0<<DDRB0);

PORTB=(0<<PORTB7) | (1<<PORTB6) | (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);	

DDRD=(0<<DDRD7) | (0<<DDRD6) | (0<<DDRD5) | (0<<DDRD4) | (0<<DDRD3) | (0<<DDRD2) | (0<<DDRD1) | (0<<DDRD0);

PORTD=(0<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);	

SPCR=(0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (1<<CPOL) | (1<<CPHA) | (1<<SPR1) | (1<<SPR0);

LCDinit();
LCDclear();

PORTB&= ~1<<Reset;
LCDstring("Reset all...",0,0);
_delay_ms(500);
PORTB|=(1<<Reset);


LCDstring("AD7715 ADC test",0,0);
_delay_ms(1000);
LCDclear();

Result.W = 0;

ad7715_init();

    while(1)
    {
	Write_to_AD7715_register(0x38); /*Пишем в коммуникационный регистр, что следующая операция будет считывание 16 бит из регистра Data*/
	ADC_result =  Read_AD7715_register();

	sprintf(LCD_buffer,"adc = %u",ADC_result);
	LCDstring(LCD_buffer,0,0);
	
	sprintf(LCD_buffer,"V = %.5f V", ADC_result * VREF_STEP);
	LCDstring(LCD_buffer,0,1);
	
	_delay_ms(250); 
	LCDclear();  
    }
}

