/*
===============================================================================
 Name        : blink_led.c
 Author      : $(franco)
 Version     :
 Copyright   : $(copyright)
 Description : prender y apagar LED por el pin 22
===============================================================================
*/
#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif

#include <cr_section_macros.h>

#define LED_ROJO = 22;

void delay(uint32_t count); // prototipo de funcion de retardo
long long convert(int);

int main(void) {
	SystemInit();		//configuracion del pll y marcar la frec de clock del micro
	uint32_t cpuclock = SystemCoreClock;
	LPC_PINCON->PINSEL1 &= ~(0b11<<12); //el pin 22 se configura como gpio con los bits 12 y 13 de PINSEL1  en 0 11000000000000 - 00111111111111, el menos sig queda como bit 12 y tiene 12 ceros atras
	//LPC_PINCON->PINSEL1 &= ~(0x03<<12); // hace lo mismo que la linea superior
	LPC_GPIO0->FIODIR |= 1<<22; //pin 22 como salida

	while(1){
			//LPC_GPIO0->FIODIR ^= 1<<22; //pin 22 toggleado entre salida y entrada, cuando es entrada se apaga el LED
			//delay(6000000);
			//otra forma
			//  LPC_GPIO->FIOCLR |= 1<<22
			//delay(6000000);
			//  LPC_GPIO->FIOSET |= 1<<22
			//otra forma
			LPC_GPIO0-> FIOPIN ^= 1<<22;
			delay(6000000);

	}
}

void delay(uint32_t count){
	for (uint32_t i = 0; i<count; i++){};
}
