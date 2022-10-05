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

	LPC_PINCON->PINSEL4 &= ~(0b1111<<0); //P2.0 y P2.1  funcionando como gpio
	LPC_GPIO2->FIODIR |= (1<<1) & (~(1<<0)) ; // P2.1- output y P2.0- input ->boton
	LPC_PINCON->PINMODE4 &= ~(0b11<<0);	//pull up P2.0


	while(1){
		if((LPC_GPIO2-> FIOPIN) & (1<<0)){
			LPC_GPIO2-> FIOSET |= 1<<1;
		}
		else{LPC_GPIO2->FIOCLR |= 1<<1;}
	}
}

void delay(uint32_t count){
	for (uint32_t i = 0; i<count; i++){};
}
