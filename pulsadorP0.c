/*
===============================================================================
 Name        : pulsador.c
 Author      : $(franco)
 Version     :
 Copyright   : $(copyright)
 Description : apagar LED al presionar pulsador (usando pull-ups) por polling
===============================================================================
*/
#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif

#include <cr_section_macros.h>

void delay(uint32_t count); // prototipo de funcion de retardo
long long convert(int);

int main(void) {
	SystemInit();		//configuracion del pll y marcar la frec de clock del micro
	uint32_t cpuclock = SystemCoreClock;
	LPC_PINCON->PINSEL2 &= ~(0b11<<10); //pin 21 funcionando como gpio (el 20 ya viene como gpio si o si)
	LPC_GPIO1->FIODIR |= (1<<21) & (~(1<<20)) ; // P1.21- output P2.20- input
	LPC_PINCON->PINMODE3 &= ~(0b11<<8);	//pull up P1.20

	while(1){
		if((LPC_GPIO1-> FIOPIN) & (1<<20)){
			LPC_GPIO1-> FIOSET |= 1<<21;
			//delay(6000000);
		}
		else{LPC_GPIO1->FIOCLR |= 1<<21;}
	}
}

void delay(uint32_t count){
	for (uint32_t i = 0; i<count; i++){};
}
