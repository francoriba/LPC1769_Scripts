/*
===============================================================================
 Name        : adc_timer
 Author      : $FerG
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

#include <stdio.h>


void confADC();
void confTimer();
uint16_t ADC0Value=0;
void confGPIO();

int main(void) {
	SystemInit();
	confTimer();
	confGPIO();
	confADC();
	while(1){}
	return 0;

}

void confADC(){
	LPC_SC->PCONP |= (1<<12); 
	LPC_ADC->ADCR |= (1<<21); 
	LPC_SC->PCLKSEL0 |= (3<<24); 
	LPC_ADC->ADCR &=~(255<<8); 
	LPC_ADC->ADCR &=~(1<<16); 
	LPC_ADC->ADCR |= (1<<26); 
	LPC_ADC->ADCR &= ~(3<<24); 
	LPC_ADC->ADCR |= (1<<27); 
	LPC_PINCON->PINSEL4 |=(1<<20); 
	LPC_PINCON->PINMODE1 |=(1<<15);
	LPC_PINCON->PINSEL1 |=(1<<14);
	LPC_PINCON->PINSEL3 |= (3<<26);
	LPC_ADC->ADINTEN = 1; 
	NVIC_EnableIRQ(ADC_IRQn);
	return;
}

void ADC_IRQHandler(){
	ADC0Value=((LPC_ADC->ADDR0)>>4)&0xFFF;
	LPC_GPIO0->FIOPIN = ADC0Value;
	return;
}

void confGPIO(){
	LPC_GPIO0->FIODIR |= (0xFFF<<0);
	LPC_GPIO0->FIOMASK |= ~(0xFFF<<0);
}

void confTimer(){
	LPC_SC->PCONP |= (1<<1); 
	LPC_SC->PCLKSEL0|=(1<<2); 
	LPC_TIM0->EMR|=(3<<6); 
	LPC_TIM0->MR1=4999;
	LPC_TIM0->PR = 0;
	LPC_TIM0->MCR=(1<<4); 
	LPC_TIM0->TCR =3; 
	LPC_TIM0->TCR &= ~(1<<1); 
	return;
}
