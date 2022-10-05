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

void configGPIO();
void configSystick();



int main(void) {
	SystemInit();		//configuracion del pll y marcar la frec de clock del micro
	uint32_t cpuclock = SystemCoreClock;
	configGPIO();
	configSystick();

	while(1){
		if(SysTick->VAL == 0){
			LPC_GPIO2->FIOPIN ^= 1<<1;
			//SysTick->CTRL & (1<<16)
			//SysTick->VAL == 0
		}
	}
}

void configGPIO(){
	LPC_PINCON->PINSEL4 &= ~(0b11<<1); //P2.1  funcionando como gpio
	LPC_GPIO2->FIODIR |= (1<<1); // P2.1- output
}
void configSystick(){
	SysTick->CTRL |= (5<<0); //Systick activado, sin int, clock interno
	SysTick->LOAD |= 0xF423F; //999999 -> 10 mseg si se trabaja a 100MHz
}

