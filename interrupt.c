/*
===============================================================================
 Name        : pulsador.c
 Author      : $(franco)
 Version     :
 Copyright   : $(copyright)
 Description : apagar LED al presionar pulsador (usando pull-ups) por interrupcion de GPIO
===============================================================================
*/
#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif

#include <cr_section_macros.h>

void delay(uint32_t count); // prototipo de funcion de retardo
void confGPIO();
void confIntGPIO();
void EINT3_IRQHandler();
uint8_t inte; //uint 8 porque por mas que la voy a incrementar solo interesa el LSB al hacer %2


int main(void) {
	SystemInit();		//configuracion del pll y marcar la frec de clock del micro
	uint32_t cpuclock = SystemCoreClock;
	confGPIO();
	confIntGPIO();

	while(1){
		if((inte%2)){
			LPC_GPIO2-> FIOSET |= 1<<1;
		}
		else{LPC_GPIO2->FIOCLR |= 1<<1;}
	}
}

void confGPIO(){
	LPC_PINCON->PINSEL4 &= ~(0b1111<<0); //P2.0 y P2.1  funcionando como gpio
	LPC_GPIO2->FIODIR |= (1<<1) & (~(1<<0)); // P2.1- output y P2.0- input ->boton
	LPC_PINCON->PINMODE4 &= ~(0b11<<0);	//pull up P2.0
}

void confIntGPIO(){
	LPC_GPIOINT->IO2IntEnF |= (1<<0);  //interrupcion por el P2.0
	LPC_GPIOINT->IO2IntClr |= (1<<0); //limpio flag de interrupcion del P2.0
	NVIC_EnableIRQ(EINT3_IRQn); // macro de cmsis que permite identificar el handeler de interrupcion que se va a habilitar
	return;
}

void EINT3_IRQHandler(){
	if(LPC_GPIOINT->IO2IntStatF & (1<<0)){
		inte++;
	}
	LPC_GPIOINT-> IO2IntClr |= (1<<0);//limpio flag de interrupcion del P2.0
}

void delay(uint32_t count){
	for (uint32_t i = 0; i<count; i++){};
}
