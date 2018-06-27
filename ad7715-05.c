#define F_CPU 8000000UL

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>

#define DRDY bit_is_clear(PINB,1)
#define Chip_select PORTB2
#define Reset PORTB6

union {
	uint16_t W;
	uint8_t Byte[2];
}Result;


void ad7715_init();
uint16_t Read_AD7715_register(void);
void Write_to_AD7715_register(uint16_t Byteword);
uint8_t Read_byte_SPI(uint8_t Addr);

void ad7715_init() {
	
	PORTB |= (1<<Chip_select);

	Write_to_AD7715_register(0x10); /*Write to communication register: set gain 1; exit the sleep mode and wait for writing to the settings register*/ 

	Write_to_AD7715_register(0x66); /*Write to settings register: perform self-calibration (MD1 = 0, MD0 = 1); crystal frequency is 2.4576 MHz; result frequency is 50 Hz; unipolar operation mode; buffer is on; FSYNC filter is off*/
	
	while(!DRDY);
}

void Write_to_AD7715_register(uint16_t Byteword) {

	PORTB &= ~(1<<Chip_select);
	
	SPDR = Byteword; 
	while(!(SPSR & (1<<SPIF)));

	PORTB |= 1<<Chip_select;
}

uint16_t Read_AD7715_register(void) {
	while(!DRDY);

	PORTB &= ~(1<<Chip_select);

	Result.Byte[1] = Read_byte_SPI(0xff);
	Result.Byte[0] = Read_byte_SPI(0xff);

	PORTB|=1<<Chip_select;

	return Result.W;
}

uint8_t Read_byte_SPI(char Addr) {
	SPDR = Addr;
	while(!(SPSR & (1<<SPIF)));
	Addr = SPDR;
	return Addr;
}

int main(void) {
	uint16_t ADC_result = 0;
	Result.W = 0;
	
	DDRB=(1<<DDRB7) | (1<<DDRB6) | (1<<DDRB5) | (1<<DDRB4) | (1<<DDRB3) | (1<<DDRB2) | (0<<DDRB1) | (0<<DDRB0);
	PORTB=(0<<PORTB7) | (1<<PORTB6) | (0<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);	
	SPCR=(0<<SPIE) | (1<<SPE) | (0<<DORD) | (1<<MSTR) | (1<<CPOL) | (1<<CPHA) | (1<<SPR1) | (1<<SPR0);

	PORTB&= ~1<<Reset;
	_delay_ms(100);
	PORTB|=(1<<Reset);

	ad7715_init();

   	while(1) {
		Write_to_AD7715_register(0x38); /*Write to communication register: the next operation will be reading 16 bits of result*/
		ADC_result =  Read_AD7715_register();

	}
}

