/*
===============================================================================
 Name        : pulsador.c
 Author      : $(franco)
 Version     :
 Copyright   : $(copyright)
 Description : apagar LED al presionar pulsador (usando pull-ups) por interrupción externa
===============================================================================
*/
#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif

#include <cr_section_macros.h>

void delay(uint32_t count); // prototipo de funcion de retardo
void confGPIO();
void confIntExt();
void EINT3_IRQHandler();
uint8_t inte; //uint 8 porque por mas que la voy a incrementar solo interesa el LSB al hacer %2


int main(void) {
	SystemInit();		//configuracion del pll y marcar la frec de clock del micro
	uint32_t cpuclock = SystemCoreClock;
	confGPIO();
	confIntExt();

	while(1){
		if((inte%2)){
			LPC_GPIO2-> FIOSET |= 1<<1;
		}
		else{LPC_GPIO2->FIOCLR |= 1<<1;}
	}
}

void confGPIO(){
	LPC_PINCON->PINSEL4 &= ~(0b11<<1); //P2.1  funcionando como gpio
	LPC_GPIO2->FIODIR |= (1<<1) & (~(1<<13)); // P2.1- output y P2.13- input ->boton
	LPC_PINCON->PINMODE4 &= ~(0b11<<13);	//pull up P2.13
}

void confIntExt(){
	LPC_PINCON->PINSEL4 |= (1<<26); //P2.13  funcionando como EXTINT3
	LPC_SC->EXTMODE |= 8; //EINT3 es interrupcion externa por flanco (1) y no por nivel (0)
	LPC_SC->EXTPOLAR &= ~(1<<8); //EINT3 es interrupcion externa por flanco de bajada (0)
	LPC_SC->EXTINT |= 8; //limpio bandera EXTINT3 (1)
	NVIC_EnableIRQ(EINT3_IRQn);
}

void EINT3_IRQHandler(){
		inte++;
		LPC_SC->EXTINT |= 8; // limpia flag de itnerrupcion externa
}

void delay(uint32_t count){
	for (uint32_t i = 0; i<count; i++){};
}
