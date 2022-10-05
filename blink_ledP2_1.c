/*
===============================================================================
 Name        : blink_led.c
 Author      : $(franco)
 Version     :
 Copyright   : $(copyright)
 Description : prender y apagar LED por P2.1
===============================================================================
*/
#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif

#include <cr_section_macros.h>


void delay(uint32_t count); // prototipo de funcion de retardo

int main(void) {
	SystemInit();		//configuracion del pll y marcar la frec de clock del micro
	uint32_t cpuclock = SystemCoreClock;
	LPC_PINCON->PINSEL4 &= ~(0b11<<1); //P2.1  funcionando como gpio
	LPC_GPIO2->FIODIR |= (1<<1); // P2.1- output

	while(1){
			//LPC_GPIO0->FIODIR ^= 1<<22; //pin 22 toggleado entre salida y entrada, cuando es entrada se apaga el LED
			//delay(6000000);
			//otra forma
			//  LPC_GPIO->FIOCLR |= 1<<22
			//delay(6000000);
			//  LPC_GPIO->FIOSET |= 1<<22
			LPC_GPIO2-> FIOPIN ^= 1<<1;
			delay(6000000);

	}
}

void delay(uint32_t count){
	for (uint32_t i = 0; i<count; i++){};
}
