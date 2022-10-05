/*
===============================================================================
 Name        : blink_led.c
 Author      : $(franco)
 Version     :
 Copyright   : $(copyright)
 Description : prender y apagar LED por P2.1 usando interrupciones del Systick
===============================================================================
*/
#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif
#include <cr_section_macros.h>

void configGPIO();
void SysTick_Handler();

uint8_t inte = 0;
uint32_t timer = 0;

int main(void) {
	SystemInit();		//configuracion del pll y marcar la frec de clock del micro
	configGPIO();


	if(SysTick_Config(SystemCoreClock/100)){while(1);} 
	while (1){
		//timer = SysTick->VAL;
		if(inte%2){LPC_GPIO2->FIOSET |= (1<<1);}
		else{LPC_GPIO2->FIOCLR |= (1<<1);}
	}
}

void configGPIO(){
	LPC_PINCON->PINSEL4 &= ~(0b11<<1); //P2.1  funcionando como gpio
	LPC_GPIO2->FIODIR |= (1<<1); // P2.1- output
}

void SysTick_Handler(){
	timer = SysTick->VAL;
	inte++;
	SysTick->CTRL &= SysTick->CTRL; //al leer el registro CTRL
}



