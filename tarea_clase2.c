/*
===============================================================================
 Name        : tarea_clase2.c
 Author      : $(franco)
 Version     :
 Copyright   : $(copyright)
 Description : Sacar por P1, un numero de 4 bits (4 gpios) detenerse cuando
 	 	 	 	 haga overflow una unidad de tiempo, y rotar un bit circular-
 	 	 	 	 mente el numero
===============================================================================
*/
#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif

#include <cr_section_macros.h>

void delay(unsigned int count); // prototipo de funcion de retardo
long long convert(int);

int main(void) {
	SystemInit();		//configuracion del pll y marcar la frec de clock del micro
	volatile uint8_t byte_value = 0x0F;
	LPC_PINCON->PINSEL2 = 0x00000000;		//configura los pines 0 a 15 del puerto 1 como GPIO,no hace falta config por defecto
	LPC_GPIO1->FIODIR = 1<<20|1<<21|1<<22|1<<23;  //configura los pines 20, 21, 22 y 23 puerto 1 como salidas
	LPC_GPIO1->FIOCLR = 1<<20|1<<21|1<<22|1<<23; //empiezan todos los leds apagados
	LPC_GPIO1->FIOMASK2 = 0x0f;						//aplico mascara para no alterar los bits del nibble bajo del fiopin

	while(1){
		LPC_GPIO1->FIOPIN2 = byte_value;
		delay(1000);
		byte_value+=1;
	}
    return 0 ;
}

void delay(unsigned int count){
	for (unsigned int i = 0; i<count; i++){
		for(unsigned int j; j<5000; j++);
	}
	}
