#include <stdio.h>
#ifdef __USE_CMSIS
#include "lpc17xx.h"
#endif
#include <cr_section_macros.h>

void configGPIO();
void configIntGPIO();
void EINT3_IRQHandler();
void delay(uint32_t time);

uint32_t sec1 = 0b010011010; //secuencia cuando interrumpe P2.0
uint32_t sec2 = 0b011100110; //secuencia cuando interrumpe P2.1 

// 010011010   0
// 100110100   1
// 001101001   2
// 011010010   3
// 110100100   4
// 101001001   5
// 010010011   6
// 100100110   7
// 001001101   8
// 010011010   9

int main(void){

	SystemInit();
	configGPIO();
	configIntGPIO();

	while(1){

		LPC_GPIO0->FIOPIN |= (0b11<<0); //una vez terminada la secuencia generada en la interrupcion, las salidas quedan en 1

	}


}

void configGPIO(){
	LPC_PINCON-> PINSEL4 &= ~(0b1111<<0); // P2.0 y P2.1 como GPIO
	LPC_PINCON->PINSEL0 &= ~(0b1111<<); //P0.0 Y P0.1 como GPIO
	LPC_PINCON-> PINMODE4 &= ~(0b1111<<0); // Pull ups para P2.0 y P2.1 
	LPC_PINCON->PINMODE0 &= ~(0b1111<<); //Pull ups P0.0 Y P0.1
	LPC_GPIO2->FIODIR &=  ~(0b11<<0); // P2.0 y P2.1 como entradas
	LPC_GPIO0->FIODIR |= (0b11<<0); //P0.0 y P0.1 como salidas
}

void configIntGPIO(){
	LPC_IO2INT-> IO2IntEnR |= (1<<0); //P2.0 interrumpe por flanco de subida
	LPC_IO2INT-> IO2IntEnF |= (1<<1); //P2.1 interrumpe por flanco de bajada
	NVIC_EnableIRQ(EINT3_IRQn);
}

void EINT3_IRQHandler(){
	if(LPC_IO2->IntStatR & (1<<0)){ //si interrumpe P2.0

		for(uint32_t i = 0; i<9; i++){
			if((sec1<<i) & (1<<0)){ //pregunto si el valor actual de la secuencia es 1 o 0
				LPC_GPIO0->FIOSET |= (1<<0); //pone en 1 P0.0
			}
			else{LPC_GPIO0->FIOCLR |= (1<<0);} //pone en 0 P0.0
			delay(6000000);
		}

	}
	else if(LPC_IO2->IntStatF & (1<<1)){//si interrumpe P2.1

		for(uint32_t i = 0; i<9; i++){
			if((sec2<<i) & (1<<0);){//pregunto si el valor actual de la secuencia es 1 o 0
				LPC_GPIO0->FIOSET |= (1<<1); //pone en 1 P0.1
			}
			else{LPC_GPIO0->FIOCLR |= (1<<1);} //pone en 0 P0.1
			delay(6000000);
		}

	}

	LPC_SC->EXTINT |= (1<<3); //limpio flag de interrupcion de la fuente EINT3 

}

void delay(uint32_t time){for(uint32_t i = 0; i<time; i++);}
